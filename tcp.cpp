//
// Created by Theodore Ahlfeld on 10/30/15.
//
#include <iostream>
#include <cstddef>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <math.h>
#include <vector>
#include <queue>
#include <sys/time.h>
#include "ftp.h"
#include "tcp.h"


static uint32_t sequence;
static int finflag;
static timeval SRTT;

/* Basic Typeless Macros */
#define min(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a)    ((a) >= 0  ? (a) : -1*(a))

/* Protocols */
bool check_timeout(struct timeval *time);
ssize_t sendack(const int sock, const uint16_t src,
                const uint16_t dst, const uint32_t ack);

/* Creates a UDP Addr */
addrinfo *create_udp_addr(char *hostname, char *port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_ADDRCONFIG;
    struct addrinfo *addr = nullptr;
    if (getaddrinfo(hostname, port, &hints, &addr)) {
        return nullptr;
    }
    return addr;
}

int create_udp_socket(addrinfo *addr) {
    int sock = socket(addr->ai_family, (addr->ai_socktype),
                      (addr->ai_protocol));
    fcntl(sock, F_SETFL, O_NONBLOCK);
    if (sock < -1) {
        die_with_err("socket() failed");
    }
    return sock;
}

void bind_udp(int sock, struct addrinfo *addr) {
    if (bind(sock, addr->ai_addr, addr->ai_addrlen) == -1) {
        die_with_err("bind() failed");
    }
}

int udp_init_listen(char *port) {
    struct addrinfo *addr = create_udp_addr(nullptr, port);
    int sock = create_udp_socket(addr);
    bind_udp(sock, addr);
    freeaddrinfo(addr);
    return sock;
}

struct PacketCompare {
    bool operator()(Packet *lhs, Packet *rhs) const {
        return lhs->get_seq() > rhs->get_seq();
    }
};

/*
 * Receives TCP data until socket breaks or File is completely received
 * int recvsock -> The listening UDP socket
 * int tcpsock  -> The TCP ACK port
 * FILE *fout   -> The file to write data too
 * FILE *log    -> The log file to write received packet data
 */
size_t recv_tcp(int recvsock, int tcpsock, FILE *fout, FILE *log) {
    /* No more data to read */
    if (finflag) {
        finflag = 0;
        return 0;
    }
    struct timeval tv;
    ssize_t len = 0;
    uint16_t src, dst;
    struct sockaddr_storage src_addr;
    socklen_t src_len = sizeof(src_addr);
    /* Packet and ACK bookkeeping */
    uint32_t expected_seq = 0;
    uint32_t totalsent = 0;
    uint32_t lastpkt_len = 0;
    /* Packet Priority Queue for packet buffering */
    std::priority_queue<Packet *, std::vector<Packet *>, PacketCompare> pq;
    do {
        if (finflag && pq.empty()) {
            return totalsent;
        }
        uint8_t packet_data[MSS + HEADLEN];
        if ((len = recvfrom(recvsock, packet_data, MSS + HEADLEN, NOFLAG,
                            (struct sockaddr *) &src_addr, &src_len)) <= 0) {
            if (errno == EAGAIN) {
                continue;
            } else {
                perror("recvfrom() failed");
                return 0;
            }
        }
        Packet *pkt = new Packet(packet_data, (size_t) len);
        src = pkt->get_srcport();
        dst = pkt->get_dstport();
        gettimeofday(&tv, NULL);
        char *logtime = ctime(&(tv.tv_sec));
        logtime[strlen(logtime) - 1] = '\0';
        /* Checksum doesn't match */
        if (!(pkt->check_checksum((size_t) len))) {
            if(log != nullptr) {
                fprintf(log, "FAILURE %s %u %u %u %u %d\n", logtime, src, dst,
                        pkt->get_seq(), pkt->get_ack(), pkt->get_flags());
            }
            /* Update Timeout */
            check_timeout(&tv);
            /* Send old ack for fast retransmit */
            if (sendack(tcpsock, dst, src, expected_seq) != HEADLEN) {
                if (!finflag) {
                    die_with_err("send failed");
                }
            }
            delete pkt;
            continue;
        }
        if(log != nullptr) {
            fprintf(log, "SUCCESS %s %u %u %u %u %d\n", logtime, src, dst,
                    pkt->get_seq(), pkt->get_ack(), pkt->get_flags());
        }
        if (!finflag) {
            finflag = (pkt->get_flags() & FINFLAG);
            /* Sets the length of the last packet for final packet handeling */
            lastpkt_len = (uint32_t)len-HEADLEN;
        }
        if (expected_seq == pkt->get_seq()) {
            /* Writes data to file output skipping the header */
            fwrite(pkt->get_data()+HEADLEN, sizeof(uint8_t),
                   (size_t)len - HEADLEN, fout);
            expected_seq += len - HEADLEN;
            totalsent += len - HEADLEN;
            delete pkt;
            /* Writes buffered data to file if the sequence number matches */
            while (!pq.empty() && (pq.top())->get_seq() == expected_seq) {
                pkt = pq.top();
                size_t size = (size_t)((pkt->get_flags() & FINFLAG)
                                       ? lastpkt_len : MSS);
                fwrite(pkt->get_data()+HEADLEN, sizeof(uint8_t), size, fout);
                totalsent += size;
                expected_seq += size;
                delete pkt;
                pq.pop();
            }
        } else if(expected_seq < pkt->get_seq()) { // Out of order packet
            pq.push(pkt);
        } else { // Already written, delete
            delete pkt;
        }
        if (sendack(tcpsock, dst, src, expected_seq) != HEADLEN) {
            if (!finflag) {
                die_with_err("send failed");
            }
        }
    } while (!pq.empty() || !finflag);
    return totalsent;
}

ssize_t sendack(const int sock, const uint16_t src,
                const uint16_t dst, const uint32_t ack) {
    Packet pkt(src, dst, ack, ACKFLAG);
    return send(sock, pkt.get_data(), HEADLEN, 0);
}

uint32_t recvack(const int sock) {
    ssize_t len = 0;
    uint8_t buf[HEADLEN];
    if ((len = recv(sock, buf, HEADLEN, 0)) == HEADLEN) {
        Packet pkt(buf, (size_t) len);
        return pkt.get_ack();
    }
    return 0;
}

/*
 * Sends data to the designated UDP socket
 * int sock             -> The UDP socket to send data over
 * void *buf            -> Data to send
 * size_t buflen        -> Size of data to send
 * struct addrinfo *addr-> The addrinfor for the listening UDP addr
 * uint16_t src_port    -> The port that data is sent from
 * uint16_t dst_port    -> The port to send data to
 * Results *res         -> The result structure to record data to.
 * int tcpsock          -> The tcp ack to send data to
 * size_t window_size   -> The desired window size, must be larger than 0
 * FILE *log            -> The log file to write sending data to
 */
ssize_t send_tcp(int sock, void *buf, size_t buflen, struct addrinfo *addr,
                 uint16_t src_port, uint16_t dst_port, Results *res,
                 int tcpsock, size_t window_size, FILE *log) {
    if(window_size==0) {
        errno = EINVAL;
        return -1;
    }
    struct timeval tv;
    ssize_t ack = 0;
    static ssize_t baseack;
    size_t total_sent = 0;
    size_t len = 0;
    uint32_t counter = 0;
    unsigned int i = 0;
    if (gettimeofday(&tv, NULL) < 0) {
        die_with_err("gettimeofday() failed");
    }
    check_timeout(&tv);
    std::priority_queue<Packet *, std::vector<Packet *>, PacketCompare>
            pkt_que;
    while (total_sent != buflen) {
        if (gettimeofday(&tv, NULL) < 0) {
            die_with_err("gettimeofday() failed");
        }
        /* Send all packets that can fit in the window */
        for(;pkt_que.size() < window_size && i * MSS < buflen; i++) {
            len = min(buflen - (i * MSS), MSS); //MSS or size of remaining data
            Packet *pkt = new
                    Packet(src_port, dst_port, (uint8_t *)buf + (i * MSS), len,
                           sequence, ((i+1) * MSS>buflen ? FINFLAG
                                                         : (uint8_t) NOFLAG) |
                           (i == 0 ? SYNFLAG : (uint8_t) NOFLAG));

            if(res != nullptr) {
                res->bytes = sequence += len;
            }
            sendto(sock, pkt->get_data(), len + HEADLEN, NOFLAG, addr->ai_addr,
                   addr->ai_addrlen);

            gettimeofday(&tv, NULL);
            char *logtime = ctime(&(tv.tv_sec));
            logtime[strlen(logtime) - 1] = '\0';
            if(log != nullptr) {
                fprintf(log, "Sending  %s %u %u %u %u %d %u\n", logtime,
                        src_port, dst_port, pkt->get_seq(), pkt->get_ack(),
                        pkt->get_flags(), (uint32_t)
                        (SRTT.tv_sec * 1000000 + SRTT.tv_usec) / 1000);
            }
            if(res != nullptr) {
                res->segments++;
            }
            pkt_que.push(pkt);
        }
        /* Recv Acks til 0 or ACK that has already be processed */
        while ((ack = recvack(tcpsock)) > 0) {
            if(ack > baseack) {
                check_timeout(&tv); // Update RTO, SRTT, and RTTVAR
                total_sent += ack - baseack;
                baseack = ack;
                counter = 0;
                /* Remove windowed information if ACK is greater than seq_num*/
                while (!pkt_que.empty() && pkt_que.top()->get_seq() < ack) {
                    delete pkt_que.top();
                    pkt_que.pop();
                }
            } else { // Old ACK received
                counter++;
                break;
            }
        }
        /* Unexpected Errors */
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != 0) {
            die_with_err("recv() failed");
        }
        /* Resending logic */
        if ((check_timeout(NULL) || (ack <= baseack && ack > 0 && counter > 3))
                                 && !pkt_que.empty()) {
            Packet *p = pkt_que.top();
            char *logtime = ctime(&(tv.tv_sec));
            logtime[strlen(logtime) - 1] = '\0';
            if(log != nullptr) {
                fprintf(log, "Resending  %s %u %u %u %u %d %u\n", logtime,
                        src_port, dst_port, p->get_seq(), p->get_ack(),
                        p->get_flags(), (uint32_t)
                        (SRTT.tv_sec * 1000000 + SRTT.tv_usec) / 1000);
            }
            sendto(sock, p->get_data(),
                   (p->get_flags() & FINFLAG ? len : MSS) + HEADLEN,
                   NOFLAG, addr->ai_addr, addr->ai_addrlen);
            if(res != nullptr) {
                res->seg_retrans++;
            }
        }
    }
    return total_sent;
}

void retrans_timer(timeval *RTO, timeval *RTTVAR, timeval *R) {
    static uint32_t counter;
    const double BETA = (1.0 / 4.0), ALPHA = (1.0 / 8.0);
    if (counter == 0) {
        RTO->tv_sec = 1;
        RTO->tv_usec = 0;
        SRTT.tv_sec = 1;
    } else if (counter == 1) {
        SRTT.tv_sec = R->tv_sec;
        SRTT.tv_usec = R->tv_usec;
        RTTVAR->tv_sec = R->tv_sec / 2;
        RTTVAR->tv_usec = R->tv_usec / 2;
        RTO->tv_sec = SRTT.tv_sec + 4 * RTTVAR->tv_sec;
        RTO->tv_usec = SRTT.tv_usec + 4 * RTTVAR->tv_usec;
        RTO->tv_sec -= RTTVAR->tv_usec / 1000000;
        RTO->tv_usec %= 1000000;
    } else {
        SRTT.tv_sec = (time_t) ((1.0 - ALPHA) * (double)SRTT.tv_sec +
                                ALPHA * (double)R->tv_sec);
        SRTT.tv_usec = (suseconds_t)((1.0-ALPHA) * (double)SRTT.tv_usec +
                                      ALPHA * (double)R->tv_usec);
        RTTVAR->tv_sec = (time_t)(((1.0 - BETA) * (double)RTTVAR->tv_sec) +
                                  (BETA * (double)ABS(SRTT.tv_sec-R->tv_sec)));
        RTTVAR->tv_usec=(suseconds_t)(((1.0 - BETA) * (double)RTTVAR->tv_usec)+
                                       BETA * (double)
                                       ABS(SRTT.tv_usec - R->tv_usec));
        RTTVAR->tv_sec -= RTTVAR->tv_usec/1000000;
        RTTVAR->tv_usec %= 1000000;
        RTO->tv_sec = SRTT.tv_sec + 4 * RTTVAR->tv_sec;
        RTO->tv_usec = SRTT.tv_usec + 4 * RTTVAR->tv_usec;
        RTO->tv_sec -= RTTVAR->tv_usec / 1000000;
        RTO->tv_usec %= 1000000;
    }
    counter++;
}

bool check_timeout(struct timeval *time) {
    static timeval tv;
    static timeval RTO;
    static timeval RTTVAR;
    static timeval R;
    timeval now;
    gettimeofday(&now, NULL);
    if (RTO.tv_sec < 1) {
        RTO.tv_sec = 1;
    } else if (RTO.tv_sec > 60) {
        RTO.tv_sec = 60;
    }
    if (time == nullptr) {
        if (now.tv_sec >= tv.tv_sec + RTO.tv_sec) {
            RTO.tv_sec = 2 * RTO.tv_sec;
            RTO.tv_usec = 2 * RTO.tv_usec;
            return true;
        }
    }
    if (time) {
        R.tv_usec = now.tv_usec - tv.tv_usec;
        R.tv_sec = now.tv_sec - tv.tv_sec;
        if(R.tv_usec < 0 && R.tv_sec > 0) {
            R.tv_sec--;
            R.tv_usec *= -1;
        }
        retrans_timer(&RTO, &RTTVAR, &R);
        tv.tv_sec = time->tv_sec;
        tv.tv_usec = time->tv_usec;
    }
    return false;
}

void die_with_err(std::string msg) {
    std::perror(msg.c_str());
    exit(1);
}

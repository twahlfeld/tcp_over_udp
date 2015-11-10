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


size_t window_size;
static uint16_t sequence;
static int finflag;

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define ABS(a)    ((a) >= 0  ? (a) : -1*(a))

bool check_timeout(struct timeval *time);
ssize_t sendack(const int sock, const uint16_t src, const uint16_t dst, const uint32_t ack);

addrinfo *create_udp_addr(char *hostname, char *port)
{
    struct addrinfo hints;
    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
    hints.ai_flags=AI_ADDRCONFIG;
    struct addrinfo *addr = nullptr;
    if(getaddrinfo(hostname, port, &hints, &addr)) {
        return nullptr;
    }
    return addr;
}

int create_udp_socket(addrinfo *addr)
{
    int sock = socket(addr->ai_family, (addr->ai_socktype),
                      (addr->ai_protocol));
    fcntl(sock, F_SETFL, O_NONBLOCK);
    if(sock < -1) {
        die_with_err("socket() failed");
    }
    return sock;
}

void bind_udp(int sock, struct addrinfo *addr)
{
    if(bind(sock, addr->ai_addr, addr->ai_addrlen) == -1) {
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

struct PacketCompare
{
    bool operator()(Packet *lhs, Packet *rhs) const
    {
        return lhs->get_seq() < rhs->get_seq();
    }
};


size_t recv_tcp(int recvsock, int tcpsock, void *buf, size_t buflen,
                Results *res, char *src, char *dst, FILE *log)
{
    uint8_t *bufp = (uint8_t *)buf;
    struct timeval tv;
    ssize_t len = 0;
    uint16_t src_port, dst_port;
    struct sockaddr_storage src_addr;
    socklen_t src_len = sizeof(src_addr);
    uint32_t expected_seq = 0;
    uint32_t lastack = 0, lastpkt;
    std::priority_queue<Packet *, std::vector<Packet *>, PacketCompare > pq;
    do {
        if(finflag && !pq.empty()) {
            return max((size_t)len, 0);
        }
        uint8_t packet_data[MSS+HEADLEN];
        if((len = recvfrom(recvsock, packet_data, MSS+HEADLEN, NOFLAG,
                       (struct sockaddr *)&src_addr, &src_len)) <= 0) {
            if(errno == EAGAIN) {
                continue;
            } else {
                perror("recvfrom() failed");
                return max((size_t) len, 0);
            }
        }
        Packet *pkt = new Packet(packet_data, (size_t)len);
        src_port = pkt->get_srcport();
        dst_port = pkt->get_dstport();
        gettimeofday(&tv, NULL);
        char *logtime = ctime(&(tv.tv_sec));
        logtime[strlen(logtime)-1] = '\0';
        if(!(pkt->check_checksum((size_t)len))) {
            fprintf(log, "FAILURE %s %s %s %d %d %d\n", logtime, src, dst,
                    pkt->get_seq(), pkt->get_ack(), pkt->get_flags());
            delete pkt;
            continue;
        }
        fprintf(log, "SUCCESS %s %s %s %d %d %d\n", logtime, src, dst,
                pkt->get_seq(), pkt->get_ack(), pkt->get_flags());
        printf("expected_seq:%u vs seq:%u\n", expected_seq, pkt->get_seq());
        if(expected_seq == pkt->get_seq()) {
			memcpy(((uint8_t *)buf)+pkt->get_seq(), pkt->get_data()+HEADLEN,
                   ((size_t)len)-HEADLEN);
            //printf("%s\n", (char *)buf+expected_seq);
            if(!finflag) {
                finflag = (pkt->get_flags() & FINFLAG);
            }
            res->bytes = expected_seq += len-HEADLEN;
            delete pkt;
            while(!pq.empty()&&(pq.top())->get_seq()==expected_seq) {
                pkt = pq.top();
                if(!finflag) {
                    finflag = (pkt->get_flags() & FINFLAG);
                }
                res->bytes += len-HEADLEN;
                memcpy(bufp, pkt->get_data()+HEADLEN, MSS);
                bufp += MSS;
                expected_seq += MSS;
                pq.pop();
            }
        } else {
            pq.push(pkt);
        }
        if(sendack(tcpsock, dst_port, src_port, expected_seq) != HEADLEN) {
            if(!finflag) {
                die_with_err("send failed");
            }
        } else {
            lastack = expected_seq;
        }
    } while(!pq.empty() || !finflag || expected_seq != buflen);
    printf("res->byte=%u\n", res->bytes);
    return res->bytes;
}

ssize_t sendack(const int sock, const uint16_t src,
                const uint16_t dst, const uint32_t ack)
{
    printf("sending ack:%u\n", ack);
    Packet pkt(src, dst, ack, ACKFLAG);
    uint8_t *buf = pkt.get_data();
    for(long i = 0; i < HEADLEN; i++){
        printf("%02x%s", buf[i], ((i+1)%4 ? " " : "\n"));
    }
    return send(sock, pkt.get_data(), HEADLEN, 0);
}

uint32_t recvack(const int sock)
{
    ssize_t len = 0;
    uint8_t buf[HEADLEN];
    if((len = recv(sock, buf, HEADLEN, 0)) == HEADLEN) {
        for (long i = 0; i < len; i++) {
            printf("%02x%s", buf[i], ((i + 1) % 4 ? " " : "\n"));
        }
        Packet pkt(buf, (size_t) len);
        printf("ACK:%u\n", pkt.get_ack());
        return pkt.get_ack();
    }
    return 0;
}

ssize_t send_tcp(int sock, uint8_t *buf, size_t buflen,
                 struct addrinfo *addr, uint16_t src_port,
                 uint16_t dst_port, Results *res, int tcpsock)
{
    struct timeval tv;
    ssize_t ack;
    size_t total_sent = 0;
    size_t len = 0;
    ssize_t lastack = 0;
    int i = 0;
    std::priority_queue<Packet *, std::vector<Packet *>, PacketCompare>
            pkt_que;
    while(lastack != buflen) {
        if(gettimeofday(&tv, NULL) < 0) {
            die_with_err("gettimeofday() failed");
        }
        while(pkt_que.size() < window_size && i*MSS < buflen) {
            len = min(buflen - (i*MSS), MSS);
            Packet *pkt = new
                    Packet(src_port, dst_port, buf + (i * MSS), len,
                           sequence,
                           ((i+1)*MSS>buflen ? FINFLAG : (uint8_t)NOFLAG) |
                           (i == 0 ? SYNFLAG : (uint8_t) NOFLAG));
            res->bytes = sequence += len;
            total_sent += (sendto(sock, pkt->get_data(), len+HEADLEN,
                   NOFLAG, addr->ai_addr, addr->ai_addrlen))-HEADLEN;
            pkt_que.push(pkt);
            i++;
        }
        while((ack = recvack(tcpsock)) > 0) {
            //total_sent += ack - sequence;
            //if(ack <= sequence + MSS) {
                check_timeout(&tv);
                lastack = ack;
            //}
            //res->bytes = sequence += total_sent;
            while(!pkt_que.empty() && (pkt_que.top())->get_seq() <= ack) {
                pkt_que.pop();
            }
        }
        if(errno != EAGAIN && errno != EWOULDBLOCK) {
            die_with_err("recv() failed");
        }
        if(check_timeout(nullptr) && !pkt_que.empty()) {
            Packet *p = pkt_que.top();
            sendto(sock, p->get_data(),
                   (p->get_flags() & FINFLAG ? len : MSS) + HEADLEN,
                   NOFLAG, addr->ai_addr, addr->ai_addrlen);
        }
    }
    perror("GOTHERE");
    return total_sent;
}

void retrans_timer(timeval *RTO, timeval *SRTT, timeval *RTTVAR, timeval *R) {
    static uint32_t counter;
    const float BETA =(1/4), ALPHA = (1/8);
    if(counter == 0) {
        RTO->tv_sec = 1;
        RTO->tv_usec = 0;
    }
    if(counter == 1) {
        SRTT->tv_sec = R->tv_sec;
        SRTT->tv_usec = R->tv_usec;
        RTTVAR->tv_sec = R->tv_sec/2;
        RTTVAR->tv_usec = R->tv_usec/2;
        RTO->tv_sec = SRTT->tv_sec + 4*RTTVAR->tv_sec;
        RTO->tv_usec = SRTT->tv_usec + 4*RTTVAR->tv_usec;
    } else {
        RTTVAR->tv_sec = (time_t)(((1-BETA)*RTTVAR->tv_sec) +
                (BETA*ABS(SRTT->tv_sec-R->tv_sec)));
        RTTVAR->tv_usec = (suseconds_t)(((1-BETA)*RTTVAR->tv_usec) +
                (BETA*ABS(SRTT->tv_usec-R->tv_usec)));
        SRTT->tv_sec = (time_t)((1-ALPHA)*SRTT->tv_sec + ALPHA*R->tv_sec);
        SRTT->tv_usec = (suseconds_t)((1-ALPHA)*SRTT->tv_usec + ALPHA*R->tv_usec);
        RTO->tv_sec = SRTT->tv_sec + 4*RTTVAR->tv_sec;
        RTO->tv_usec = SRTT->tv_usec + 4*RTTVAR->tv_usec;
    }
    if(RTO->tv_sec < 1) {
        RTO->tv_sec = 1;
        RTO->tv_usec = 0;
    }
    counter++;
}

bool check_timeout(struct timeval *time) {
    static timeval tv;
    static timeval RTO;
    static timeval SRTT;
    static timeval RTTVAR;
    static timeval R;
    timeval now;
    gettimeofday(&now, NULL);
    if(RTO.tv_sec > 60) {
        RTO.tv_sec = 60;
    }
    if(time == nullptr) {
        if(now.tv_sec >= tv.tv_sec+RTO.tv_sec && now.tv_usec >= tv.tv_usec + RTO.tv_usec) {
            RTO.tv_sec = 2*RTO.tv_sec;
            RTO.tv_usec = 2*RTO.tv_usec;
            return true;
        }
    }
    R.tv_usec = now.tv_usec - tv.tv_usec;
    R.tv_sec = now.tv_sec - tv.tv_sec;
    if(time) {
        retrans_timer(&RTO, &SRTT, &RTTVAR, &R);
        memcpy(&tv, time, sizeof(timeval));
    }
    return false;
}

void die_with_err(std::string msg)
{
    std::perror(msg.c_str());
    exit(1);
}

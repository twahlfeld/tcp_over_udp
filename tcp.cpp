//
// Created by Theodore Ahlfeld on 10/30/15.
//
#include <iostream>
#include <fcntl.h>
#include <math.h>
#include <vector>
#include <queue>
#include "ftp.h"
#include "tcp.h"

size_t window_size;
static uint16_t sequence;

#define min(a, b) ((a) < (b) ? (a) : (b))

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

int create_udp_socket(addrinfo *addr) {

    int sock = socket(addr->ai_family, (addr->ai_socktype),
                      (addr->ai_protocol));
    //fcntl(sock, O_NONBLOCK);
    if(sock < -1) {
        die_with_err("socket() failed");
    }
    return sock;
}

void bind_udp(int sock, struct addrinfo *addr) {
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
bool Compare(Packet *lhs, Packet *rhs)
{
    return lhs->get_seq() > rhs->get_seq();
}

ssize_t recv_tcp(int sock, uint8_t *buf, size_t buflen, Results *res) {
    ssize_t len;
    struct sockaddr_storage src_addr;
    socklen_t src_len = sizeof(src_addr);
    std::priority_queue< <Packet>, std::vector<Packet>, Compare> pq;
    do {
        len = recvfrom(sock, buf, buflen, NOFLAG,
                       (struct sockaddr *) &src_addr, &src_len);
        fwrite(buf, 1, len, stdout);
        if (len <= 0) {
            return len;
        }
    } while(len);
    return len;
}

ssize_t send_tcp(int sock, uint8_t *buf, size_t buflen, struct addrinfo *addr,
                 uint16_t src_port, uint16_t dst_port, Results *res) {
    Packet *window = new Packet[window_size];
    ssize_t size;
    int i, max = (int)ceil(MSS/buflen);
    for(i = 0; i < max; i++) {
        size_t len = min(buflen - (i * MSS), MSS);
        window[i].init(src_port, dst_port, buf + (i * MSS), len,
                           sequence + (uint16_t) (i * MSS), NOFLAG);
        sendto(sock, window[i].get_data(), len+HEADLEN,
               NOFLAG, addr->ai_addr, addr->ai_addrlen);
    }
    sequence += buflen-(buflen%MSS);
    return buflen;
}

void die_with_err(std::string msg)
{
    std::perror(msg.c_str());
    exit(1);
}
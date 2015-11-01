//
// Created by Theodore Ahlfeld on 10/30/15.
//
#include <iostream>
#include <fcntl.h>
#include "tcp.h"

#define NOFLAG 0
static size_t window_size;

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
    fcntl(sock, O_NONBLOCK);
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

ssize_t recv_tcp(int sock, char *buf, size_t buflen) {
    ssize_t len;
    struct sockaddr_storage src_addr;
    socklen_t src_len = sizeof(src_addr);
    //Packet
    do {
        len = recvfrom(sock, buf, buflen, NOFLAG,
                       (struct sockaddr *) &src_addr, &src_len);

        if (len <= 0) {
            return len;
        }
    } while(len);
    return len;
}

ssize_t send_tcp(int sock, char *buf, size_t buflen, struct addrinfo *addr) {
    return sendto(sock, buf, buflen, NOFLAG, addr->ai_addr, addr->ai_addrlen);
}

void die_with_err(std::string msg)
{
    std::perror(msg.c_str());
    exit(1);
}
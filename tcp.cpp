//
// Created by Theodore Ahlfeld on 10/30/15.
//
#include <iostream>
#include <fcntl.h>
#include "tcp.h"

#define NOFLAG 0
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

ssize_t send_tcp(int sock, char *buf, size_t buflen, struct addrinfo *addr,
                 uint16_t src_port, uint16_t dst_port) {

    Packet *window = new Packet[window_size];
    size_t seg_size = MSS-HEADLEN;
    char *buf_base = buf;
    int base = 0, i = 0;
    while(i < (buflen-(buflen%seg_size))) {///base != (buflen-(buflen%seg_size))) {
        window[i] = Packet(src_port, dst_port, buf + (i * seg_size), min(buflen - (i * seg_size), seg_size),
                           sequence + (uint16_t) (i * seg_size), 0);
        //sendto(sock, , sizeof(Packet), NOFLAG, addr->ai_addr, addr->ai_addrlen);
        printf("%s\n", window[i].get_data());
        fwrite(window[i++].get_data(), MSS, sizeof(char), stdout);
        std::cout << std::endl;
    }
    sequence += buflen-(buflen%seg_size);
}

void die_with_err(std::string msg)
{
    std::perror(msg.c_str());
    exit(1);
}
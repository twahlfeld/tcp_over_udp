//
// Created by Theodore Ahlfeld on 10/30/15.
//
#include <iostream>
#include "tcp.h"

addrinfo *create_udp_sock(char *hostname, char *port)
{
    struct addrinfo hints;
    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
    hints.ai_flags=AI_ADDRCONFIG;
    struct addrinfo *addr = NULL;
    if(getaddrinfo(hostname, port, &hints, &addr)) {
        return NULL;
    }
    return addr;
}

int create_udp_socket(addrinfo *addr) {
    int sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if(sock < -1) {
        die_with_err("socket() failed");
    }
}

void die_with_err(std::string msg)
{
    perror(msg);
    exit(1);
}
//
// Created by Theodore Ahlfeld on 10/30/15.
//

#ifndef __TCP_H
#define __TCP_H
#include <sys/socket.h>
#include <netdb.h>
#include "packet.h"

void die_with_err(std::string msg);
addrinfo *create_udp_addr(char *hostname, char *port);
int create_udp_socket(addrinfo *addr);
int udp_init_listen(char *port);
ssize_t recv_tcp(int sock, char *buf, size_t buflen);
int udp_init(char *hostname, char *port);
ssize_t send_tcp(int sock, char *buf, size_t buflen, struct addrinfo *addr,
                 uint16_t src_port, uint16_t dst_port);

#endif

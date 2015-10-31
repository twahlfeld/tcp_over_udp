//
// Created by Theodore Ahlfeld on 10/30/15.
//

#ifndef __TCP_H
#define __TCP_H
#include <sys/socket.h>
#include <netdb.h>
addrinfo *create_udp_sock(char *hostname, char *port);
int create_udp_socket(addrinfo *addr);
void die_with_err(std::string msg);
#endif

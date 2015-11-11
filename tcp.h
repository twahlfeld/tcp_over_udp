//
// Created by Theodore Ahlfeld on 10/30/15.
//

#ifndef __TCP_H
#define __TCP_H
#include <sys/socket.h>
#include <netdb.h>
#include "packet.h"
#include "ftp.h"

#define NOFLAG 0

void die_with_err(std::string msg);
addrinfo *create_udp_addr(char *hostname, char *port);
int create_udp_socket(addrinfo *addr);
int udp_init_listen(char *port);
size_t recv_tcp(int recvsock, int tcpsock, FILE *fout, FILE *log);
int udp_init(char *hostname, char *port);
ssize_t send_tcp(int sock, uint8_t *buf, size_t buflen, struct addrinfo *addr, uint16_t src_port, uint16_t dst_port,
                 Results *res, int tcpsock, size_t window_size, FILE *log);

#endif

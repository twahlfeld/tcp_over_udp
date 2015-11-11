//
// Created by Theodore Ahlfeld on 11/1/15.
//

#ifndef __FTP_OVER_UDP_H
#define __FTP_OVER_UDP_H

#include <cstdio>
#include <stdint.h>

typedef struct Results {
    uint8_t status;
    uint64_t bytes;
    uint32_t segments;
    uint32_t seg_retrans;
} Results;

Results sendfile(FILE *fp, int sock, uint16_t src_port, uint16_t dst_port, struct addrinfo *addr, int tcpsock,
                 size_t window_size, FILE *log);
int recvfile(FILE *fp, int recvsock, int tcpsock, FILE *log);

#endif //TCP_OVER_UDP_FTP_H

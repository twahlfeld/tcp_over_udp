//
// Created by Theodore Ahlfeld on 11/1/15.
//

#include <iostream>
#include "tcp.h"
#include "ftp.h"

extern size_t window_size;

Results sendfile(FILE *fp, int sock, uint16_t src_port,
                 uint16_t  dst_port, struct addrinfo *addr)
{
    Results res;
    memset(&res, 0, sizeof(Results));
    size_t len;
    ssize_t send_data = 0;
    uint8_t *buf = new uint8_t[MSS*window_size];
    while((len = fread(buf, sizeof(char), MSS*window_size, fp)) > 0) {
        send_tcp(sock, buf, len, addr, src_port, dst_port, &res);
    }
    delete[] buf;
    return res;
}

Results recvfile(FILE *fp, int sock, uint16_t src_port, uint16_t dst_port)
{
    Results res;
    memset(&res, 0, sizeof(Results));
    size_t len;
    ssize_t recv_len;
    uint8_t *buf = new uint8_t[MSS*10];
    while((recv_len = recv_tcp(sock, buf, MSS * 10, &res)) > 0) {
        len = fwrite(buf, sizeof(uint8_t), (size_t)recv_len, fp);
        res.bytes += len;
    }
    return res;
}

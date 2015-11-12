//
// Created by Theodore Ahlfeld on 11/1/15.
//

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/fcntl.h>
#include "tcp.h"
#include "ftp.h"


Results sendfile(FILE *fp, int sock, uint16_t src_port, uint16_t dst_port,
                 struct addrinfo *addr, int tcpsock,
                 size_t window_size, FILE *log)
{
    Results res;
    memset(&res, 0, sizeof(Results));
    size_t len;
    uint8_t *buf = new uint8_t[MSS*window_size];
    int acksock;
    struct sockaddr_in clnt_addr;
    memset(&clnt_addr, 0, sizeof(clnt_addr));
    socklen_t addrlen = sizeof(struct sockaddr_in);
    while((acksock = accept(tcpsock, (sockaddr *)&clnt_addr, &addrlen)) < 0 &&
          errno == EWOULDBLOCK);
    if(acksock < 0) {
        die_with_err("accept() failed");
    }
    fcntl(acksock, F_SETFL, O_NONBLOCK);

    while((len = fread(buf, sizeof(char), MSS*window_size, fp)) > 0) {
        send_tcp(sock, buf, len, addr, src_port, dst_port, &res, acksock,
                 window_size, log);
    }
    if(errno == 0 || errno == EAGAIN || errno == EWOULDBLOCK) {
        res.status = 0;
    } else {
        res.status = 1;
    }
    close(acksock);
    delete[] buf;
    return res;
}

int recvfile(FILE *fp, int recvsock, int tcpsock, FILE *log)
{
    uint8_t *buf = new uint8_t[MSS*10]();
    while(recv_tcp(recvsock, tcpsock, fp, log) > 0);
    delete[] buf;
    if(errno == EAGAIN || errno == EWOULDBLOCK) {
        return 0;
    }
    return 1;
}

//
// Created by Theodore Ahlfeld on 11/1/15.
//

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/fcntl.h>
#include "tcp.h"
#include "ftp.h"

extern size_t window_size;

Results sendfile(FILE *fp, int sock, uint16_t src_port, uint16_t dst_port,
                 struct addrinfo *addr, int tcpsock)
{

    //printf("sock:%d=>Blocking:%d\n", sock, (fcntl(sock, F_GETFL) & O_NONBLOCK)!=0);
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

    printf("acksock:%d=>Blocking:%d\n", acksock, (fcntl(acksock, F_GETFL) & O_NONBLOCK)!=0);
    while((len = fread(buf, sizeof(char), MSS*window_size, fp)) > 0) {
        send_tcp(sock, buf, len, addr, src_port, dst_port, &res, acksock);
    }
    close(acksock);
    delete[] buf;
    return res;
}

Results recvfile(FILE *fp, int recvsock, int tcpsock, char *src, char *dst, FILE *log)
{
    Results res;
    memset(&res, 0, sizeof(Results));
    size_t len;
    ssize_t recv_len;
    uint8_t *buf = new uint8_t[MSS*10]();
    while((recv_len = recv_tcp(recvsock, tcpsock, buf, MSS * 10, &res, src, dst, log)) > 0) {
        perror("PERROR recv_tcp");
        printf("recv_len=%ld\n", recv_len);
        len = fwrite(buf, sizeof(uint8_t), (size_t)recv_len, fp);
        res.bytes += len;
    }
    delete[] buf;
    return res;
}

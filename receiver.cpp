//
// Created by Theodore Ahlfeld on 10/30/15.
//

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include "tcp.h"
#include "receiver.h"

int create_tcpsock(char *host, char *port)
{
    struct sockaddr_in serv_addr;
    struct hostent *he;
    int sock;
    int enabled = 1;
    if((he = gethostbyname(host)) == NULL) {
        die_with_err("gethostbyname() failed");
    }
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        die_with_err("socket() failed");
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(
            inet_ntoa(*(struct in_addr *)he->h_addr));
    serv_addr.sin_port        = htons(atoi(port));
    int res;
    while((res = connect(sock, (struct sockaddr*)&serv_addr,
                         sizeof(serv_addr))) < 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(int));
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &enabled, sizeof(int));
    if(res < 0) {
        die_with_err("connect() failed");
    }
    errno = 0;
    return sock;
}

int main(int argc, char *argv[])
{
    if(argc != 6) {
        die_with_err("Incorrect number of arguments\n"
                     "receiver <filename> <listening_port> <sender_IP> "
                     "<sender_port> <log_filename>");
    }
    FILE *fp = fopen(argv[1], "wb");
    if(fp == nullptr) {
        die_with_err("Failed to creater file");
    }
    FILE *log = (strcmp("stdout", argv[5]) ? fopen(argv[5], "w") : stdout);
    if(log == nullptr) {
        die_with_err("Failed to creater file");
    }
    int recv_sock = udp_init_listen(argv[2]);
    int send_sock = create_tcpsock(argv[3], argv[4]);

    if(recvfile(fp, recv_sock, send_sock, log)) {
        printf("Delivery was unsuccessful\n");
    } else {
        printf("Delivery completed successfully\n");
    }
    fclose(fp);
    if(log != stdout) fclose(log);
    close(recv_sock);
    close(send_sock);
    return 0;
}

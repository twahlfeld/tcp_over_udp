//
// Created by Theodore Ahlfeld on 10/30/15.
//
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/fcntl.h>
#include "ftp.h"
#include "tcp.h"

extern size_t window_size;

struct sockaddr_in init_socket_addr(const unsigned long addr,
                                    const uint16_t port)
{
    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = htonl(addr);
    sock_addr.sin_port = htons(port);
    return sock_addr;
}

int create_tcp_listener(char *port)
{
    struct sockaddr_in serv_addr;
    serv_addr = init_socket_addr(INADDR_ANY, (uint16_t)atoi(port));
    int enabled = 1;
    int sock;
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        die_with_err("socket() failed");
    }
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(int));
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &enabled, sizeof(int));
    fcntl(sock, F_SETFL, O_NONBLOCK);
    if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        die_with_err("bind() failed");
    }
    if (listen(sock, 1) < 0) {
        die_with_err("listen() failed");
    }
    return sock;
}

int main(int argc, char *argv[])
{
    if(argc != 7) {
        if(argc > 7 || argc < 6) {
        die_with_err("Incorrect number of arguments\n"
                     "sender <filename> <remote_IP> <remote_port> "
                     "<ack_port_num> <log_filename> <window_size(optional)>");
		}
    }
    uint16_t src_port, dst_port;
    unsigned long a_to_num = strtoul(argv[3], NULL, 10);
    if(a_to_num > 0xFFFF || a_to_num == 0) {
        die_with_err("Invalid Destination Port Number");
    }
    dst_port = (uint16_t)a_to_num;
    a_to_num = strtoul(argv[4], NULL, 10);
    if(a_to_num > 0xFFFF || a_to_num == 0) {
        die_with_err("Invalid ACK Port Number");
    }
    src_port = (uint16_t)a_to_num;
    window_size = (size_t)(argc == 7 ? atoi(argv[6]) : 1);
    if(window_size == 0) {
        die_with_err("Invalid Window Size");
    }
    FILE *fp = fopen(argv[1], "rb");
    struct addrinfo *addr = create_udp_addr(argv[2], argv[3]);
    int send_sock = create_udp_socket(addr);
    int recv_sock = create_tcp_listener(argv[4]);

    Results res = sendfile(fp, send_sock, dst_port, src_port, addr, recv_sock);
    fclose(fp);
    close(recv_sock);
    close(send_sock);
    printf("btyes:%llu\n", res.bytes);
    return 0;
}

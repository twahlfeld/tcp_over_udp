//
// Created by Theodore Ahlfeld on 10/30/15.
//
#include <iostream>
#include "packet.h"
#include "tcp.h"

#define MAXBUFF 4096

int main(int argc, char *argv[])
{
    if(argc > 7 && argc < 6) {
        die_with_err("Incorrect number of arguments\n"
                     "sender <filename> <remote_IP> <remote_port> "
                     "<ack_port_num> <log_filename> <window_size(optional)>");
    }
    size_t len;
    extern window_size = (argc == 7 ? atoi(argv[6]) : 1);
    if(window_size == 0) {
        die_with_err("Invalid Window Size");
    }
    FILE *fp = fopen(argv[1], "r");
    struct addrinfo *addr = create_udp_addr(argv[2], argv[3]);
    int send_sock = create_udp_socket(addr);
    int recv_sock = udp_init_listen(argv[4]);

    char buf[MAXBUFF];

    while((len = fread(buf, sizeof(char), MAXBUFF, fp)) > 0) {
        ssize_t packlen;
        packlen = send_tcp(send_sock, buf, len, addr);
    }
    return 0;
}
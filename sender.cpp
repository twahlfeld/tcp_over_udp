//
// Created by Theodore Ahlfeld on 10/30/15.
//
#include <iostream>
#include "tcp.h"

int main(int argc, char *argv[])
{
    if(argc != 7) {
        die_with_err("Incorrect number of arguments\n"
                     "sender <filename> <remote_IP> <remote_port> "
                     "<ack_port_num> <log_filename> <window_size>");
    }
    size_t len;
    size_t window_size = atoi(argv[6]);
    if(window_size == 0) {
        die_with_err("Invalid Window Size");
    }
    char *buf = new (std::nothrow) char[window_size];
    if(buf == nullptr) {
        die_with_err("new() char[] failed");
    }
    FILE *fp = fopen(argv[1], "r");
    //int recv_sock = udp_init_listen(argv[4]);
    struct addrinfo *addr = create_udp_addr(argv[2], argv[3]);
    int send_sock = create_udp_socket(addr);
    while((len = fread(buf, sizeof(char), window_size, fp)) > 0) {
        ssize_t packlen;
        packlen = send_tcp(send_sock, buf, len, addr);
    }
    delete[] buf;
    return 0;
}

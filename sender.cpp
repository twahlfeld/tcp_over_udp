//
// Created by Theodore Ahlfeld on 10/30/15.
//
#include <iostream>
#include "ftp.h"
#include "tcp.h"

#define MAXBUFF 4096
extern size_t window_size;

int main(int argc, char *argv[])
{
<<<<<<< HEAD
    if(argc != 7) {
=======
    if(argc > 7 || argc < 6) {
>>>>>>> 44d8298679737ac8ec198c028000db7fe5f1a6ed
        die_with_err("Incorrect number of arguments\n"
                     "sender <filename> <remote_IP> <remote_port> "
                     "<ack_port_num> <log_filename> <window_size(optional)>");
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
    //int recv_sock = udp_init_listen(argv[4]);

    Results res = sendfile(fp, send_sock, dst_port, src_port, addr);
    printf("btyes:%lld\n", res.bytes);
    return 0;
}

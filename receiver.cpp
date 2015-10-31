//
// Created by Theodore Ahlfeld on 10/30/15.
//

#include <iostream>
#include "tcp.h"
#include "receiver.h"

#define MAXBUFF 4096

int main(int argc, char *argv[])
{
    if(argc != 6) {
        die_with_err("Incorrect number of arguments\n"
                     "receiver <filename> <listening_port> <sender_IP> "
                     "<sender_port> <log_filename>");
    }
    //FILE *fp = fopen(argv[1], "w");
    int recv_sock = udp_init_listen(argv[2]);
    std::perror("AFTER UDP_INIT_LISTEN");



    char buf[MAXBUFF];
    ssize_t len = recv_tcp(recv_sock, buf, sizeof(buf));
    if(len < 0) {
        die_with_err("recvfrom() failed");
    } else if (len < 0) {
        printf("Peer close their half of the socket");
    } else {
        printf("%s\n", buf);
    }


    /*struct addrinfo *send_addr = create_udp_addr(argv[3], argv[4]);
    int send_sock = create_udp_socket(send_addr);*/


    return 0;
}

//
// Created by Theodore Ahlfeld on 10/30/15.
//

#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include "tcp.h"
#include "receiver.h"

using namespace std;

int main(int argc, char *argv[])
{
    if(argc != 5) {
        die_with_err("Incorrect number of arguments\n"
                     "receiver <filename> <listening_port> <sender_IP> "
                     "<sender_port> <log_filename>");
    }
    FILE *fp = fopen(argv[1], "w");
    uint16_t receiver_port, sender_port;
    struct addrinfo *send_addr = create_udp_sock(argv[3], argv[4]);
    struct addrinfo *recv_addr = create_udp_sock(NULL, argv[0]);
    int recv_sock = create_udp_socket(recv_addr);
    int send_sock = create_udp_socket(send_addr);
    if(bind(recv_sock, recv_addr->ai_addr, recv_addr->ai_addrlen) == -1) {
        die_with_err("bind() failed");
    }
}

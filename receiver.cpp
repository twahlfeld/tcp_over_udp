//
// Created by Theodore Ahlfeld on 10/30/15.
//

#include <iostream>
#include <cstdlib>
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
    FILE *fp = fopen(argv[1], "wb");
	if(fp == nullptr) {
		die_with_err("Failed to creater file");
	}
    int recv_sock = udp_init_listen(argv[2]);


    /*uint8_t buf[MAXBUFF];
    ssize_t len = recv_tcp(recv_sock, buf, sizeof(buf), nullptr);*/
	Results res = recvfile(fp, recv_sock, atoi(argv[2]), atoi(argv[4]));
    /*if(len < 0) {
        die_with_err("recvfrom() failed");
    } else if (len == 0) {
        printf("Peer close their half of the socket");
    } else {*/
 //       fwrite(buf, sizeof(char), res.bytes, fp);
    //}

    //struct addrinfo *send_addr = create_udp_addr(argv[3], argv[4]);
    //int send_sock = create_udp_socket(send_addr);

    //fclose(fp);
    return 0;
}

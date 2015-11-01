//
// Created by Theodore Ahlfeld on 10/31/15.
//

#ifndef TCP_PACKET_H
#define TCP_PACKET_H

#define HEADLEN 20
const int MSS = 576;

class Packet {

private:
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq_num;
    uint32_t ack_num;
    unsigned len:4;
    unsigned flags:6;
    uint16_t recv_window;
    uint16_t checksum;
    uint16_t urgent;
    char data[MSS-20];
public:
    void init_header();
    Packet();
    Packet(char *buf);
    ~Packet();
};

#endif //TCP_PACKET_H

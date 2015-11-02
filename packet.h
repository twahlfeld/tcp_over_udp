//
// Created by Theodore Ahlfeld on 10/31/15.
//

#ifndef TCP_PACKET_H
#define TCP_PACKET_H

#define HEADLEN 20
const int MSS = 576;
const uint8_t FINFLAG = 0x1;
const uint8_t SYNFLAG = 0x2;
const uint8_t ACKFLAG = 0x10;

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
    char data[MSS];
public:
    void init_header();
    void set_checksum();
    //void send_packet(struct addrinfo *addr);
    char *get_data() {
        return this->data;
    }
    Packet(uint16_t src, uint16_t dst, char *buf, size_t len, uint16_t seq_num, uint8_t flags);
    Packet() {

    }
    Packet(char *buf);

};

#endif //TCP_PACKET_H

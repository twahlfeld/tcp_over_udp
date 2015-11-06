//
// Created by Theodore Ahlfeld on 10/31/15.
//

#ifndef TCP_PACKET_H
#define TCP_PACKET_H

#include <array>

const int HEADLEN = 20;
const int MSS = 556;
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
    uint8_t *data;
public:
    void init_header();
    void set_checksum();
    int get_flags() { return this->flags; }
    int check_flags(unsigned int flag) { return flag&(this->flags); }
    const uint32_t get_seq() { return this->seq_num; }
    void init(uint16_t src, uint16_t dst, uint8_t *buf, size_t len, uint16_t seq_num, uint8_t flags);
    uint8_t *get_data() {
        return this->data;
    }
    Packet(uint16_t src, uint16_t dst, uint8_t *buf, size_t len, uint16_t seq_num, uint8_t flags)
    {
        init(src, dst, buf, len, seq_num, flags);
    }
    Packet(uint8_t *data, size_t len);
    Packet() {
        ;
    }
    ~Packet();
};

#endif //TCP_PACKET_H

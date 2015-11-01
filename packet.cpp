//
// Created by Theodore Ahlfeld on 10/31/15.
//

#include "packet.h"

void convert_word(char *data, uint16_t x)
{
    char *c = data;
    *c = (x>>8) & 0xFF;
    *(++c) = x & 0xFF;
}

void convert_dword(char *data, uint32_t x)
{
    int i;
    for(i = 0; i < 4; i++) {
        data[i] = (x >> (32 + (i+1)*8)) * 0xFF;
    }
}

void Header::init_header()
{
    convert_word(data[0], this->src_port);
    convert_word(data[2], this->dst_port);
    convert_dword(data[4], this->seq_num);
    convert_dword(data[8], this->ack_num);
    data[12] = ((this->len)<<4) & 0xF0;
    data[13] = (this->flags) & 3F;
    convert_word(data[14], this->recv_window);
    convert_word(data[16], this->checksum);
    convert_word(data[18], this->urgent);
}

uint16_t char_to_word(char *c)
{
    return ((*c)<<8 | (*(c+1)));
}

uint32_t char_to_dword(char *c)
{
    return ((char_to_word(c)<<16) | (char_to_word(c+2)));
}

Packet(int src, int dst) {
    this->src_port = src;
    this->dst_port = dst;
}

Packet(char *buf) {
    this->src_port = char_to_word(buf);
    this->dst_port = char_to_word(buf+2);
    this->seq_num = char_to_dword(buf+4);
    this->ack_num = char_to_dword(buf+8);
    this->len = buf[12]>>4;
    this->flags = buf[13];
    this->recv_window = char_to_word(buf[14]);
    this->checksum = char_to_word(buf[16]);
    this->urgent = char_to_word(buf[18]);
    this->data = std::string(buf[20]);
    
}

~Packet() {
    delete this->data;
}

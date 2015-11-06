//
// Created by Theodore Ahlfeld on 10/31/15.
//

#include <iostream>
#include <cstdint>
#include <cstring>
#include "packet.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

void convert_word(void *data, int16_t x)
{
    uint8_t *c = (uint8_t *)data;
    *c = (uint8_t)((x>>8) & 0xFF);
    *(++c) = (uint8_t)(x & 0xFF);
}

void convert_dword(uint8_t *data, uint32_t x)
{
    int i;
    for(i = 0; i < 4; i++) {
        data[i] = (uint8_t)((x >> (32 + (i+1)*8)) * 0xFF);
    }
}

void Packet::init_header()
{
    convert_word(data, this->src_port);
    convert_word(data+2, this->dst_port);
    convert_dword(data+4, this->seq_num);
    convert_dword(data+8, this->ack_num);
    data[12] = (uint8_t)(((this->len)<<4) & 0xF0);
    data[13] = (uint8_t)((this->flags) & 0x3F);
    convert_word(data+14, this->recv_window);
    convert_word(data+16, this->checksum);
    convert_word(data+18, this->urgent);
}

void Packet::set_checksum()
{
    int i;
    uint32_t x;
    this->checksum = 0;
    void *kludge = this;
    char *data = (char *)kludge;
    for(i = 0; i < MSS/2; i++) {
        if((uint16_t *)data == &((this->checksum))) {
            continue;
        }
        x = (uint32_t)(*(uint8_t *)(data+i))<<8;
        x |= (uint32_t)(*(uint8_t *)(data+i+1));
        x += this->checksum;
        if(x & 0x10000) {
            x++;
        }
        this->checksum = (uint16_t)x;
    }
}

uint16_t char_to_word(uint8_t *c)
{
    uint16_t word = uint16_t((*c)<<8);
    word = word | (uint16_t)(*(c+1));
    return word;
}

uint32_t char_to_dword(uint8_t *c)
{
    uint32_t dword = char_to_word(c)<<16;
    dword = dword | (char_to_word(c+2));
    return dword;
}

void Packet::init(uint16_t src, uint16_t dst, uint8_t *buf, size_t len,
                  uint16_t seq_num, uint8_t flags)
{
    this->src_port = src;
    this->dst_port = dst;
    this->seq_num = seq_num;
    this->flags = (unsigned)(flags & 0x3F);
    this->len = 0x5;
    this->urgent = 0;
    this->data = new uint8_t[len+HEADLEN];
    memcpy(data+HEADLEN, buf, len);
    set_checksum();
    init_header();
}

Packet::Packet(uint8_t *buf, size_t len)
{
    this->src_port = char_to_word(buf);
    this->dst_port = char_to_word(buf+2);
    this->seq_num = char_to_dword(buf+4);
    this->ack_num = char_to_dword(buf+8);
    this->len = buf[12]>>4;
    this->flags = buf[13];
    this->recv_window = char_to_word(buf+14);
    this->checksum = char_to_word(buf+16);
    this->urgent = char_to_word(buf+18);
    this->data = new uint8_t[len-HEADLEN];
    memcpy(this->data+HEADLEN, buf, len);
}

Packet::~Packet()
{
    if(data) {
        delete[] data;
    }
}

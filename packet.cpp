//
// Created by Theodore Ahlfeld on 10/31/15.
//

#include <iostream>
#include <cstdlib>
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

    for(int i = 0; i < 4; i++) {
        data[i] = (uint8_t)((x >> (32 - (i+1)*8)) & 0xFF);
    }
}

void Packet::init_header()
{
    convert_word(data, this->src_port);
    //printf("src:%d\n", src_port);
    convert_word(data+2, this->dst_port);
    //printf("dst:%d\n", dst_port);
    convert_dword(data+4, this->seq_num);
    //printf("seq:%d\n", seq_num);
    convert_dword(data+8, this->ack_num);
    //printf("ack:%d\n", ack_num);
    data[12] = (uint8_t)(((this->len)<<4) & 0xF0);
    //printf("len:%d\n", len);
    data[13] = (uint8_t)((this->flags) & 0x3F);
    //printf("flg:%d\n", flags);
    convert_word(data+14, this->recv_window);
    //printf("win:%d\n", recv_window);
    convert_word(data+16, 0);;
    //printf("chk:%d\n", 0);
    convert_word(data+18, this->urgent);;
    //printf("urgent:%d\n\n", urgent);
}

uint16_t Packet::calc_checksum(size_t len)
{
    uint32_t x = 0;
    uint16_t sum = 0;
    uint8_t *data = this->data;

    for(unsigned long i = 0; i < len; i++) {
        if(i == 16) {
            i+=2;
        }
        x = (uint32_t)(data[i++])<<8;
        x |= (uint32_t)((i<len ? data[i] : 0));
        x += sum;
        if(x & 0x10000) {
            x &= 0xFFFF;
            x++;
        }
        sum = (uint16_t)x;
    }
    return sum;
}

bool Packet::check_checksum(size_t len)
{
    return this->checksum == this->calc_checksum(len);
}

void Packet::init(uint16_t src, uint16_t dst, uint8_t *buf, size_t len,
                  uint32_t seq_num, uint32_t ack_num, uint8_t flags)
{
    this->src_port = src;
    this->dst_port = dst;
    this->seq_num = seq_num;
    this->ack_num = ack_num;
    this->flags = (unsigned)(flags & 0x3F);
    this->len = 0x5;
    this->urgent = 0;
    this->recv_window = 0;
    this->data = new uint8_t[len+HEADLEN]();
    memcpy(data+HEADLEN, buf, len);
    init_header();
    this->checksum = this->calc_checksum(len+HEADLEN);
    convert_word(data+16, this->checksum);
}


uint16_t char_to_word(uint8_t *c)
{
    return (*c)<<8 | *(c+1);
}

uint32_t char_to_dword(uint8_t *c)
{
    uint32_t dword = char_to_word(c)<<16;
    dword = dword | (char_to_word(c+2));
    return dword;
}

Packet::Packet(uint8_t *buf, size_t len)
{
    this->data = new uint8_t[len]();
    memcpy(this->data, buf, len);
    this->checksum = char_to_word(buf+16);
    //printf("chk:%d\n", checksum);
    this->src_port = char_to_word(buf);
    //printf("src:%d\n", src_port);
    this->dst_port = char_to_word(buf+2);
    //printf("dst:%d\n", dst_port);
    this->seq_num = char_to_dword(buf+4);
    //printf("seq:%d\n", seq_num);
    this->ack_num = char_to_dword(buf+8);
    //printf("ack:%d\n", ack_num);
    this->len = buf[12]>>4;
    //printf("len:%d\n", this->len);
    this->flags = buf[13];
    //printf("flg:%d\n", flags);
    this->recv_window = char_to_word(buf+14);
    //printf("win:%d\n", recv_window);
    this->urgent = char_to_word(buf+18);
    //printf("urg:%d\n\n", urgent);
}

Packet::Packet()
{
    memset(this, 0, sizeof(Packet));
}

Packet::~Packet()
{
    if (this->data != nullptr) {
        delete[] this->data;
    }
}
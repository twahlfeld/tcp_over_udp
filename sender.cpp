//
// Created by Theodore Ahlfeld on 10/30/15.
//
#include <iostream>
#include <cstdlib>
#include <sys/socket.h>
#include <netdb.h>
#include "sender.h"
using namespace std;

void die_with_err(string msg)
{
    cout << msg << endl;
    exit(1);
}

int main(int argc, char *argv[])
{
    if(argc != 6) {
        die_with_err("Incorrect number of arguments\n"
                     "sender <filename> <remote_IP> <remote_port> "
                     "<ack_port_num> <log_filename> <window_size>");
    }

}
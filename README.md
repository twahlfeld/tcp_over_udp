TCP OVER UDP
Author: Theodore Ahlfeld (twa2108)

Installation:
    $ make

Running:
    ./receiver with <filename> <listening_port> <sender_IP> <sender_port> <log_filename>.
    The receiver listens on <listening_port> for incoming data. Acks are sent from the sender
    to receiver of an actual TCP port defined as <sender_port>.
    Packet results are logged to <log_filename>, which is a specified logfile, can be stdout
    in the format :<Result> <timestamp> <source> <destination> <sequence#> <ack#> <flags>
    hen all packets have been received correctly, the receiver completes the output file and 
    the log file.

    ./sender <filename> <remote_IP> <remote_port> <ack_port_num> <log_filename> <window_size>.
    Sender reads <filename> and sends packets to the receiver.
    Sender receives acks on <ack_port_num> and logs results to <log_filename> in the format: 
    <timestamp> <source> <destination> <sequence#> <ack#> <flags> <SRTT in miliseconds>.
    Sender will buffer up to <window_size> packets. <window_size> is optional and default is 1.

tcp.cpp contains some basic functionallity.

    TCP header is implemented as 20 bytes organized in the manner presented 
    in the textbook. There is no options, and urgent is never used. The three flags
    of bitwise or with FIN(0x01), SYN(0x02), and ACK(0x10),

    Sender sends packets from file til the window is full. Then it waits for acks. 
    If there is an ACK larger than last received it removes the buffered packets 
    from the window. If it receives a smaller or equal to ACK, a counter gets incremented.
    After it checks if 3 of the same ACKs have been repeated, timeout, and window has stuff.
    If the above conditions are met the sender resends the base packet.
    Sender repeats these two functions until all packets have been sent,
    when it sends fin and enters a loop where it only receives acks until a fin
    is received. Throughout a timer is being set and running based on the
    calcuated timeout interval. When the timer expires the sender sends the
    oldest packet that has not been acked.

    Receiver enters a loop until both the receiver buffer is empty and finflag is set.
    Upon receipt the receiver checks the buffered packets and sends acks accordingly. 
    The receiver stays until this loop until fin is received and the last packet matches the seq of
    the fin. Then receiver sends fin and completes.

    LOSS RECOVERY MECHANISM: If the receiver receives a packet with an
    incorrect checksum it sends the last correct ack and discards the packet.
    If the receiver receives a packet out of order it is buffered 
    When the receiver receives a correct and in-orderpacket it writes to file and
    checks the buffer and writes any packets that are now in order.  Receiver sends 
    cumulative acks after any correct receipt. The sender buffers every packet that it sends, 
    and clears packets from the buffer when they have been ACK’d. The sender also maintains a
    timer (using socket timeout) that triggers a resend of the oldest packet if
    expired.  Everything seems to work correctly.

    Notes about my interpretation of the assignment. The receiver window size is
    not specified so the it matches the window by the sender. I interpreted the source and
    destination fields in the logs to refer to the port number. 

    RTT time for each packet is calculated based on the time of receipt of
    cumulative acks, therefore packets that had been previously buffered will
    have longer RTT than the packet with the highest seq# in the ack.

    RTT Timeout Interval(RTO) is calculated with as specified in the RFC 6298
    Checksum is calculated performing binary addition 16 bits at a time
    exactly as specified in the textbook and lecture.

    Sample inputs to run program.  First run make to compile.  Must run in the
    order below.

Sample Runs:

$ ./newudpl -p12001:12003 -ilocalhost/**** -olocalhost/12000 -d0.001 -B100 -vv

$ ./receiver asdf.asdf 12000 127.0.0.1 6691 recv.log
Delivery completed successfully
$ head recv.log
SUCCESS Thu Nov 12 18:55:19 2015 12001 6691 0 0 2
FAILURE Thu Nov 12 18:55:19 2015 12257 6691 556 0 0
FAILURE Thu Nov 12 18:55:19 2015 12257 6691 1112 0 0
FAILURE Thu Nov 12 18:55:19 2015 12257 6691 556 0 0
SUCCESS Thu Nov 12 18:55:19 2015 12001 6691 556 0 0
FAILURE Thu Nov 12 18:55:19 2015 12257 6691 556 0 0
FAILURE Thu Nov 12 18:55:19 2015 12257 6691 1112 0 0
SUCCESS Thu Nov 12 18:55:19 2015 12001 6691 1112 0 0
FAILURE Thu Nov 12 18:55:19 2015 12257 6691 1668 0 2
SUCCESS Thu Nov 12 18:55:19 2015 12001 6691 2224 0 0

$ ./sender receiver 127.0.0.1 12001 6691 send.log 3
Delivery Completed successfully
Total bytes send = 187209
Segments send = 337
Segments retransmitted = 844
$ head send.log
Sending  Thu Nov 12 18:55:19 2015 12001 6691 0 0 2 1000
Sending  Thu Nov 12 18:55:19 2015 12001 6691 556 0 0 1000
Sending  Thu Nov 12 18:55:19 2015 12001 6691 1112 0 0 1000
Resending  Thu Nov 12 18:55:19 2015 12001 6691 556 0 0 15
Resending  Thu Nov 12 18:55:19 2015 12001 6691 556 0 0 15
Resending  Thu Nov 12 18:55:19 2015 12001 6691 556 0 0 15
Resending  Thu Nov 12 18:55:19 2015 12001 6691 1112 0 0 15
Resending  Thu Nov 12 18:55:19 2015 12001 6691 1112 0 0 15
Sending  Thu Nov 12 18:55:19 2015 12001 6691 1668 0 2 15
Sending  Thu Nov 12 18:55:19 2015 12001 6691 2224 0 0 15

$ diff receiver asdf.asdf 
$


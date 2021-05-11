#!/usr/bin/python3
#
# @file SendUDP.py
#
# Test program to send a UDP packet to an address/port
#
# usage:
#    sendUDP.py [ip] [port]
#
# if IP/port is not provided 127.0.0.1 and 8080 will be used.
#
# @author Will Merges
#
import sys, socket

def main(args):
    ip = args[1]
    port = int(args[2])
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    sock.setsockopt(socket.IPPROTO_UDP, 1, 1)

    sock.bind(('', 8081))

    while(1):
        # send
        msg = sys.stdin.readline()[:-1]
        sock.sendto(msg.encode('utf-8'), (ip, port))

        # receive
        print("receiving...")
        msg = sock.recv(1024)
        print(msg)

if len(sys.argv) == 3:
    main(sys.argv)
else:
    print("using default ip: 127.0.0.1 port: 8080")
    main(["","127.0.0.1", "8080"])

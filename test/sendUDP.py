#!/usr/bin/python3

import sys, socket

def main(args):
    ip = args[1]
    port = int(args[2])
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

    msg = "testing"

    sock.sendto(msg.encode('utf-8'), (ip, port))

if len(sys.argv) == 3:
    main(sys.argv)
else:
    print("using default ip: 127.0.0.1 port: 8080")
    main(["","127.0.0.1", "8080"])

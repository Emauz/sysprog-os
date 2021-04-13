#!/usr/bin/python3

import sys, socket

def main(args):
    ip = args[1]
    port = int(args[2])
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_IP)
    msg = "test"

    sock.sendto(msg.encode('utf-8'), (ip, port))

main(sys.argv)

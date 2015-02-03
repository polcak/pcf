#!/usr/bin/env python3

import argparse
import http.client as httplib
import random
import sys
import time

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("host",
            help = "Address of the remote host running the web server to be contacted")
    parser.add_argument("-p", "--port", help="Port of the web server",
                    type=int, default=80)
    parser.add_argument("-s", "--sleep",
            help="The mean time in minutes to sleep between connections",
            type=float, default=10.0)
    parser.add_argument("-u", "--url",
            help="Relative URL to be requested on the server",
            default="/pcf/timestamp46.html")
    args = parser.parse_args()
    return args

if __name__ == "__main__":
    args = parse_args()
    addr = args.host
    port = args.port
    lambdaexp = 1.0 / args.sleep
    url = args.url
    print(addr, port, lambdaexp, url)
    while True:
        http = httplib.HTTPConnection(addr, port)
        http.request("POST",
            url,
            body=str(time.time()))
        sleeptime = random.expovariate(lambdaexp) * 60
        time.sleep(sleeptime)

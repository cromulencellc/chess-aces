#!/usr/bin/env python3

import sys
import socket
import random
import time
import string
import base64
import os
import datetime
import string
import dns.query
import dns.message

def get_txt_bad(HOST, PORT):
    print('[TEST] get_txt_bad')

    dest, txt = random.choice([('darpaaachess.com', '86400 IN TXT "This is the text for the darpa chess domain"'), ('daraapachess.org', '86400 IN TXT "This is the text for the darpa chess org domain"'), ('a.daraapachess.org', '86400 IN TXT "Basic txt record for a"')])

    q = dns.message.make_query(dest, 'TXT')

    result = dns.query.udp(q, HOST)

    if ( len(result.question) != 0):
        print('[FAIL] The request shouldn\'t return any results')
        return 1

    print('[SUCCESS] get_txt_bad')

    return 0

def get_txt(HOST, PORT):
    print('[TEST] get_txt')

    dest, txt = random.choice([('darpachess.com', '86400 IN TXT "This is the text for the darpa chess domain"'), ('darpachess.org', '86400 IN TXT "This is the text for the darpa chess org domain"'), ('a.darpachess.org', '86400 IN TXT "Basic txt record for a"')])

    q = dns.message.make_query(dest, 'TXT')

    result = dns.query.udp(q, HOST)

    good = False

    ## Check the question
    for x in result.question:
         if x.to_text() == dest + '. IN TXT':
            good = True
            break

    if good == False:
        print('[FAIL] Failed to find question')
        return 1

    good = False

    ## Check the answer
    for x in result.answer:
        if x.to_text() == dest + '. ' + txt:
            good = True
            break

    if good == False:
        print('[FAIL] Failed to find answer')
        return 1

    print('[SUCCESS] get_txt')

    return 0

def get_A(HOST, PORT):
    print('[TEST] get_A')

    dest, txt = random.choice([('darpachess.com', '86400 IN A 10.0.2.15'), ('darpachess.org', '86400 IN A 10.0.2.15'), ('a.darpachess.org', '86400 IN A 10.0.2.15')])

    q = dns.message.make_query(dest, 'A')

    result = dns.query.udp(q, HOST)

    good = False

    ## Check the question
    for x in result.question:
         if x.to_text() == dest + '. IN A':
            good = True
            break

    if good == False:
        print('[FAIL] Failed to find question')
        return 1

    good = False

    ## Check the answer
    for x in result.answer:
        if x.to_text() == dest + '. ' + txt:
            good = True
            break

    if good == False:
        print('[FAIL] Failed to find answer')
        return 1

    print('[SUCCESS] get_A')

    return 0

def get_CNAME(HOST, PORT):
    print('[TEST] get_A')

    dest, txt = random.choice([('darpachess.com', '86400 IN CNAME www.darpachess.com.'), ('darpachess.org', '86400 IN CNAME www.darpachess.org.'), ('a.darpachess.org', '86400 IN CNAME b.darpachess.org.')])

    q = dns.message.make_query(dest, 'CNAME')

    result = dns.query.udp(q, HOST)

    good = False

    ## Check the question
    for x in result.question:
         if x.to_text() == dest + '. IN CNAME':
            good = True
            break

    if good == False:
        print('[FAIL] Failed to find question')
        return 1

    good = False

    ## Check the answer
    for x in result.answer:
        if x.to_text() == dest + '. ' + txt:
            good = True
            break

    if good == False:
        print('[FAIL] Failed to find answer')
        return 1

    print('[SUCCESS] get_A')

    return 0

def get_A_bad(HOST, PORT):
    print('[TEST] get_A_bad')

    dest, txt = random.choice([('darpachessasdf.com', '86400 IN A 10.0.2.15'), ('darpachess.orgadf', '86400 IN A 10.0.2.15'), ('aasdf.darpachess.orgasdf', '86400 IN A 10.0.2.15')])

    q = dns.message.make_query(dest, 'A')

    result = dns.query.udp(q, HOST)

    if len(result.question) != 0:
        print('[FAIL] Should not have received a good response')
        return 1

    print('[SUCCESS] get_A_bad')

    return 0

def get_AAAA(HOST, PORT):
    print('[TEST] get_AAAA')

    dest, txt = random.choice([('darpachess.com', '86400 IN AAAA ::'), ('darpachess.org', '86400 IN AAAA ::'), ('a.darpachess.org', '86400 IN AAAA ::')])

    q = dns.message.make_query(dest, 'AAAA')

    result = dns.query.udp(q, HOST)

    good = False

    ## Check the question
    for x in result.question:
         if x.to_text() == dest + '. IN AAAA':
            good = True
            break

    if good == False:
        print('[FAIL] Failed to find question')
        return 1

    good = False

    ## Check the answer
    for x in result.answer:
        if x.to_text() == dest + '. ' + txt:
            good = True
            break

    if good == False:
        print('[FAIL] Failed to find answer')
        return 1

    print('[SUCCESS] get_AAAA')

    return 0

def get_MX(HOST, PORT):
    print('[TEST] get_MX')

    dest, txt = random.choice([('darpachess.com', '86400 IN MX 10 mail1.darpachess.com.'), ('darpachess.org', '86400 IN MX 10 mail1.darpachess.org.'), ('a.darpachess.org', '86400 IN MX 10 mail1.darpachess.org.')])

    q = dns.message.make_query(dest, 'MX')

    result = dns.query.udp(q, HOST)
    
    good = False

    ## Check the question
    for x in result.question:
         if x.to_text() == dest + '. IN MX':
            good = True
            break

    if good == False:
        print('[FAIL] Failed to find question')
        return 1

    good = False

    ## Check the answer
    for x in result.answer:
        if x.to_text() == dest + '. ' + txt:
            good = True
            break

    if good == False:
        print('[FAIL] Failed to find answer')
        return 1

    print('[SUCCESS] get_MX')

    return 0

def get_NS(HOST, PORT):
    print('[TEST] get_MX')

    dest, txt = random.choice([('darpachess.com', '86400 IN NS ns1.darpachess.com.'), ('darpachess.org', '86400 IN NS ns1.darpachess.org.'), ('a.darpachess.org', '86400 IN NS ns1.darpachess.org.')])

    q = dns.message.make_query(dest, 'NS')

    result = dns.query.udp(q, HOST)
    
    good = False

    ## Check the question
    for x in result.question:
         if x.to_text() == dest + '. IN NS':
            good = True
            break

    if good == False:
        print('[FAIL] Failed to find question')
        return 1

    good = False

    ## Check the answer
    for x in result.answer:
        if x.to_text() == dest + '. ' + txt:
            good = True
            break

    if good == False:
        print('[FAIL] Failed to find answer')
        return 1

    print('[SUCCESS] get_MX')

    return 0

def get_ANY(HOST, PORT):
    print('[TEST] get_ANY')

    dest, txt = random.choice([('darpachess.com', '86400 IN NS ns1.darpachess.com.'), ('darpachess.org', '86400 IN NS ns1.darpachess.org.'), ('a.darpachess.org', '86400 IN NS ns1.darpachess.org.')])

    q = dns.message.make_query(dest, 'ANY')

    result = dns.query.udp(q, HOST)
    
    good = False

    ## Check the question
    for x in result.question:
         if x.to_text() == dest + '. IN ANY':
            good = True
            break

    if good == False:
        print('[FAIL] Failed to find question')
        return 1

    good = False

    ## Check the answer
    for x in result.answer:
        if x.to_text() == dest + '. ' + txt:
            good = True
            break

    if good == False:
        print('[FAIL] Failed to find answer')
        return 1

    print('[SUCCESS] get_ANY')

    return 0

def main():
    if len(sys.argv) == 3:
        HOST = sys.argv[1]
        PORT = int(sys.argv[2])
    else:
        if 'HOST' in os.environ and 'PORT' in os.environ:
            HOST = os.environ['HOST']
            PORT = int(os.environ['PORT'])
        else:
            print('[ERROR] HOST and PORT must be specified via arguments or variables.')
            sys.exit(1)

    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])
    else:
        SEED = random.randint(0, 2**64 - 1)

    print('[INFO] SEED: {0}'.format(SEED))

    random.seed( SEED )

    dns_ip = socket.gethostbyname(HOST)

    print(f'[INFO] DNS server: {dns_ip}')

    for _ in range(15):

        tests = [get_txt, get_txt_bad, get_A, get_A_bad, get_CNAME, get_AAAA, get_MX, get_NS, get_ANY]
        action = random.choice(tests)
        if action(dns_ip, PORT):
            print('[FAIL] Poller failed')
            sys.exit()
    
    print('[SUCCESS] Poller completed successfully')

if __name__ == '__main__':
    main()

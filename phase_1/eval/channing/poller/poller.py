#!/usr/bin/env python3

import requests
from requests.auth import HTTPBasicAuth
import sys
import os
import random


import logging

def randomstring():

    return ''.join(random.choice('abcdefghijklmnopqrstuvwxyz1234567890') for _ in range(random.randint(5, 200)))


def chunked_data_gen():

    if random.randint(0, 10) == 0:
        return

    yield randomstring().encode()
    yield randomstring().encode()
    yield randomstring().encode()

HOST = os.getenv('HOST', '127.0.0.1')
PORT = os.getenv('PORT', '8080')
SEED = int(os.getenv('SEED', random.randint(0, 2**64 - 1)))
LENGTH = int(os.getenv('LENGTH', random.randint(50, 100)))


print('SEED=' + str(SEED))
print('LENGTH=' + str(LENGTH))

BASE_URI = 'http://' + HOST + ':' + PORT + '/'
URI1 = BASE_URI + 'index.html'
URI2 = BASE_URI
URI3 = BASE_URI + 'index2.html'
URI4 = BASE_URI + 'secure/index.html'
URI5 = BASE_URI + 'stats'
URI6 = BASE_URI + 'useragents'

if SEED is not None:
    random.seed(SEED)

USER_AGENT_LIST = []

logging.getLogger().setLevel(10)
logging.getLogger("urllib3.connectionpool").setLevel(50)

def simple_get():
    logging.info("Doing the simple GET test")

    randuseragent = randomstring()
    USER_AGENT_LIST.append(randuseragent)

    headers = {'User-Agent': randuseragent }

    response1 = requests.get(URI1, headers=headers)

    if response1.status_code != 200:

        logging.fatal("bad response for request1")
        sys.exit(-1)


    logging.info("Doing the GET test with a default filepath")

    randuseragent = randomstring()
    USER_AGENT_LIST.append(randuseragent)

    headers = {'User-Agent': randuseragent }

    response2 = requests.get(URI2, headers=headers)

    if response2.status_code != 200:
        logging.fatal("bad response for request2")

        sys.exit(-1)


    if response1.content != response2.content:
        logging.fatal("Response1 and Response2 should have the same content")
        sys.exit(-1)

def notfound():
    randuseragent = randomstring()
    USER_AGENT_LIST.append(randuseragent)

    headers = {'User-Agent': randuseragent }

    logging.info("Doing the 404 test")

    response3 = requests.get(URI3, headers=headers)

    if response3.status_code != 404:
        logging.fatal("Response3 should be a 404 error")
        sys.exit(-1)

def auth_reqd():
    randuseragent = randomstring()
    USER_AGENT_LIST.append(randuseragent)

    headers = {'User-Agent': randuseragent }

    logging.info("Doing the Auth required test")

    response4 = requests.get(URI4, headers=headers)

    if response4.status_code != 401:
        logging.fatal("Response4 should have been a 401 error code")
        sys.exit(-1)

    if response4.headers['WWW-Authenticate'] != 'Basic realm="Secure Realm':
        logging.fatal("Response4 auth headers were incorrect")
        logging.fatal(response4.headers)
        sys.exit(-1)

def basic_auth():

    randuseragent = randomstring()
    USER_AGENT_LIST.append(randuseragent)
    auth = HTTPBasicAuth('swood', 'labrat')
    headers = {'User-Agent': randuseragent }

    logging.info("Doing the test with Basic Auth credentials")

    response5 = requests.get(URI4, headers=headers, auth=auth)

    if response5.status_code != 200:

        logging.fatal("Response5 should have been 200")
        sys.exit(-1)


def bad_auth():
    auth = HTTPBasicAuth('swood', randomstring())
    headers = {'User-Agent': 'poller/1.0.1'}

    logging.info("Doing the Auth test with bad credentials")

    response6 = requests.get(URI4, headers=headers, auth=auth)

    if response6.status_code != 401:

        logging.fatal("Response6 should have been 401")
        logging.fatal("Status_code = {}".format(response6.status_code))
        sys.exit(-1)

def stats():
    headers = {'User-Agent': 'poller/1.0.1'}
    logging.info("Requesting the stats page")

    response7 = requests.get(URI5, headers=headers)

    if response7.status_code != 200:

        logging.fatal("Response7 should have been 200")
        sys.exit(-1)

def useragents():
    headers = {'User-Agent': 'poller/1.0.1'}
    logging.info("Requesting the useragents page")

    response8 = requests.get(URI6, headers=headers)

    if response8.status_code != 200:

        logging.fatal("Response8 should have been 200")
        sys.exit(-1)

    for uastring in USER_AGENT_LIST:

        if uastring.encode() not in response8.content:

            logging.fatal("Did not find expected useragent string in list")
            sys.exit(-1)

def post_urlencoded():

    num_pairs = random.randint(1, 10)
    pairs = []


    for _ in range(num_pairs):

        pairs.append( (randomstring(), randomstring()))


    data = '='.join(pairs[0])
    expected = '<tr><td>{}</td><td>{}</td></tr>'.format(pairs[0][0], pairs[0][1])

    for item in pairs[1:]:
        data += '&' + '='.join(item)
        expected += '<tr><td>{}</td><td>{}</td></tr>'.format(item[0], item[1])

    logging.info("Doing the POST test with x-www-form-urlencoded data")


    headers = {'Content-Type': 'application/x-www-form-urlencoded'}

    response9 = requests.post(BASE_URI, headers=headers, data=data.encode())

    if expected.encode() not in response9.content:
        logging.fatal("did not get the expected response for x-www-form-urlencoded post")

        logging.fatal(response9.content)
        sys.exit(-1)

def post_binary():

    randdata = bytearray(random.getrandbits(8) for _ in range(random.randint(10, 1000)))

    expected = ''

    for i in range(len(randdata)):

        expected += '{:02x} '.format(randdata[i])

        if (i+1) % 16 == 0:

            expected += '</br>'

    headers = {'Content-Type': 'application/octet-stream'}

    logging.info("Doing the POST test with binary data")

    response9 = requests.post(BASE_URI + randomstring(), headers=headers, data=randdata)


    if expected.encode() not in response9.content:
        logging.fatal("Did not get expected response from post with binary data")
        logging.fatal(expected)
        logging.fatal(response9.content)
        sys.exit(-1)


def post_random():
    content_type = randomstring()

    headers = {'Content-Type': content_type}

    logging.info("Doing the POST test with random Content-Type field")

    response9 = requests.post(BASE_URI + randomstring(), headers=headers, data=b'alpha=beta')


    if 'You submitted {} data'.format(content_type) not in response9.content.decode():

        logging.fatal("Did not get the expected content-type processing")
        logging.fatal(response9.content)

        sys.exit(-1)

def post_chunky():
    content_type = randomstring()

    headers = {'Content-Type': content_type}
    logging.info("Doing the POST test with chunked transfer encoding")

    response9 = requests.post(BASE_URI+ randomstring(), headers=headers, data=chunked_data_gen())


    if 'You submitted {} data'.format(content_type) not in response9.content.decode():

        logging.fatal("Did not get the expected content-type processing")
        logging.fatal(response9.content)

        sys.exit(-1)


ACTIVITIES = [simple_get,
              #filepath_get,
              notfound,
              auth_reqd,
              basic_auth,
              bad_auth,
              stats,
              useragents,
              post_urlencoded,
              post_binary,
              post_random,
              post_chunky]

for _i in range(LENGTH):
    acti = random.choice(ACTIVITIES)
    logging.info("running activity {0}".format(acti.__name__))
    acti()

logging.info("success :)")

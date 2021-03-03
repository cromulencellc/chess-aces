import os
import signal
import socket
import sys
import time
import threading


def str2bytes(s):
    if type(s) == str:
        return bytes(s, "utf8")
    else:
        return s

def bytes2str(b):
    if type(b) == bytes:
        return str(b, "utf8")
    else:
        return b


class Comms:
    def __init__(self, host, port):
        self._host = host
        self._port = port
    
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.connect((host, port))
        self._sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        self._debug = False
        self._data = bytes()

        if "TIMEOUT" in os.environ and os.environ["TIMEOUT"] != 0:
            signal.alarm(int(os.environ["TIMEOUT"]))

    
    def close(self):
        self._sock.close()
    
    def set_debug(self, debug):
        self._debug = debug
    
    def sendall(self, data):
        if self._debug:
            print("Sending {}".format(data))
        self._sock.sendall(str2bytes(data))
    
    def recv(self, length):
        data_recevied = self._sock.recv(length)
        if len(data_recevied) == 0:
            print("Disconnected")
            sys.exit(0)
        self._data = self._data + data_received
        if self._debug:
            print("Received {}".format(data))
        data = self._data[:length]
        self._data = self._data[:length]
        return data
    
    def recv_exact(self, length):
        while len(self._data) < length:
            data_received = self._sock.recv(length - len(self._data))
            if len(data_received) == 0:
                break
            self._data += data_received
        if self._debug:
            print("Received {}".format(data))
        data = self._data[:length]
        self._data = self._data[length:]
        return data
    
    def recv_until(self, substr):
        while self._data.find(str2bytes(substr)) == -1:
            data_received = self._sock.recv(1)
            if len(data_received) == 0:
                print("Disconnected")
                sys.exit(0)
            self._data += data_received
        if self._debug:
            print("Received {}".format(data))
        data = self._data[:self._data.find(str2bytes(substr)) + len(substr)]
        self._data = self._data[self._data.find(str2bytes(substr)) + len(substr):]
        return data
    
    def recvline(self):
        return self.recv_until("\n")
    
    def recvall(self):
        while True:
            received = self._sock.recv(1024)
            if len(received) == 0:
                break
            self._data += received
        if self._debug:
            print("Received {}".format(data))
        data = self._data
        self._data = bytes()
        return data
    
    def info(self, msg):
        print("[.] {}".format(msg))
    
    def error(self, msg):
        print("[-] {}".format(msg))
    
    def success(self, msg):
        print("[!] {}".format(msg))

    def interactive(self):

        def recv_thread():
            while True:
                data = self._sock.recv(1024)
                if len(data) == 0:
                    break
                sys.stdout.buffer.write(data)
                sys.stdout.buffer.flush()
            print("recv_thread exit")

        t = threading.Thread(target=recv_thread)
        t.start()

        try:
            while True:
                stdin = sys.stdin.buffer.read(1)
                self.sendall(stdin)
        except KeyboardInterrupt:
            t.join()
        
        return

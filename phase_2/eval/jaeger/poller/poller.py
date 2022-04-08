import base64
import jaeger_lib
import os
import random
import struct
import sys
import time

HOST = os.environ["HOST"]
PORT = int(os.environ["PORT"])
SEED = random.randint(0, 10000)
LENGTH = random.randint(1, 10)

if "SEED" in os.environ:
    SEED = int(os.environ["SEED"])
if "LENGTH" in os.environ:
    LENGTH = int(os.environ["LENGTH"])

print("SEED={}".format(SEED))
print("LENGTH={}".format(LENGTH))
print("HOST={}".format(HOST))
print("PORT={}".format(PORT))

random.seed(SEED)

ACCESS_TOKEN_CREATE_REQUEST = 0
ACCESS_TOKEN_CREATE_RESPONSE = 1
ACCESS_TOKEN_VERIFICATION_REQUEST = 2
ACCESS_TOKEN_VERIFICATION_RESPONSE = 3

ACCESS_TYPE_FILESYSTEM = 0x01
ACCESS_TYPE_DEBUG = 0x02
ACCESS_TYPE_COMMUNICATIONS = 0x10
ACCESS_TYPE_MAINTENANCE = 0x20
ACCESS_TYPE_VALID = 0x80

ACD_ADDRESS = 1
MCD_ADDRESS = 2
RCD_ADDRESS = 3
FSD_ADDRESS = 4

RCD_KEY = b"\x0f\xa2\xd0\x4f\x57\x31\x0b\x8b\xdd\x70\x4e\xc8\xcc\xb8\x6c\xb2"

RCD_PERMISSIONS = ACCESS_TYPE_COMMUNICATIONS

def test_get_token(serial, serial_proto, key, requested_permissions):
    serial.relay_privileged_bus()
    request = struct.pack("<BB",
                          ACCESS_TOKEN_CREATE_REQUEST,
                          requested_permissions) + \
              key
    serial_proto.send_stream(ACD_ADDRESS, request)
    response = serial_proto.receive()
    return (response.data[1] & 0x80) == 0x80


def test_verify_valid_token(serial, serial_proto, key, permissions):
    serial.relay_privileged_bus()
    # get a token
    request = struct.pack("<BB", ACCESS_TOKEN_CREATE_REQUEST, permissions) + key
    serial_proto.send_stream(ACD_ADDRESS, request)

    response = serial_proto.receive()
    if not ((response.data[1] & 0x80) == 0x80):
        return False
    token = response.data[2:10]

    # verify it
    request = struct.pack("<B", ACCESS_TOKEN_VERIFICATION_REQUEST) + token
    serial_proto.send_stream(ACD_ADDRESS, request)
    response = serial_proto.receive()
    return (response.data[1] & 0x80) == 0x80


def test_verify_invalid_token(serial, serial_proto, token, permissions):
    serial.relay_privileged_bus()
    request = struct.pack("<BB", ACCESS_TOKEN_VERIFICATION_REQUEST, permissions) + \
              token
    serial_proto.send_stream(ACD_ADDRESS, request)
    response = serial_proto.receive()
    return (response.data[1] & 0x80) == 0x80


def test_get_file(serial_proto):
    serial.relay_privileged_bus()
    request = struct.pack("<BB", ACCESS_TOKEN_CREATE_REQUEST + ACCESS_TYPE_FILESYSTEM) + \
              RCD_KEY
    serial_proto.send_stream(ACD_ADDRESS, request)

    response = serial_proto.receive()
    if not ((response.data[1] & 0x80) == 0x80):
        return False
    
    token = response.data[2:10]
    serial.relay_rf_comms_bus()
    file_contents = jaeger_lib.fsp.get_file_contents(serial_proto, \
                                                     FCD_ADDRESS, \
                                                     token, \
                                                     b"/TOKEN\x00")
    return file_contents.success


print("[*] Connecting")
serial = jaeger_lib.Serial(HOST, PORT)
print("[+] Connected")
serial_proto = jaeger_lib.SerialProto(serial, RCD_ADDRESS)

for i in range(LENGTH):
    choice = random.randint(0, 2)
    # Test getting a valid token
    if choice == 0:
        if random.randint(0, 1) == 0:
            requested_permissions = random.randint(0, 255)
        else:
            requested_permissions = RCD_PERMISSIONS
        valid = test_get_token(serial, serial_proto, RCD_KEY, requested_permissions)
        if requested_permissions & 0x7f == RCD_PERMISSIONS:
            if valid:
                print("[pass] permissions were correctly valid")
            else:
                print("[fail] permissions were incorrectly invalid")
                sys.exit(-1)
        else:
            if valid:
                print("[fail] permissions were incorrectly valid")
                sys.exit(-1)
            else:
                print("[pass] permissions were correctly invalid")
    # Test token verification for valid token
    elif choice == 1:
        if test_verify_valid_token(serial, serial_proto, RCD_KEY, RCD_PERMISSIONS):
            print("[pass] valid token verified")
        else:
            print("[fail] valid token was not verified")
            sys.exit(-1)
    # Test token verification for invalid token
    elif choice == 2:
        fh = open("/dev/urandom", "rb")
        token = fh.read(8)
        fh.close()

        # Any permission will work here, so we just choose one
        valid = test_verify_invalid_token(serial, serial_proto, token, ACCESS_TYPE_FILESYSTEM)
        if valid:
            print("[fail] invalid token was verified")
            sys.exit(-1)
        else:
            print("[pass] invalid token was not verified")
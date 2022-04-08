from .frame import *
from .prot import *

from . import fsp
import socket

ACD_ADDRESS = 1
MCD_ADDRESS = 2
RCD_ADDRESS = 3
FSD_ADDRESS = 4

RCD_DISABLE_BUS_RELAY       = 0xF0
RCD_RELAY_PRIVILEGED_BUS    = 0xF1
RCD_RELAY_RC_COMMS_BUS      = 0xF3
RCD_SEND_BYTE_STREAM_TO_BUS = 0xA0


class ByteStream:
    '''
        Represents an out-going byte stream for the RF Comms device
    '''
    def __init__(self, bytes):
        self._bytes = bytes
    
    def encode(self):
        return struct.pack(">B", len(self._bytes)) + self._bytes

class Serial:
    '''
        Interaction with the Serial TCP device connected to the RF Comms device.
    '''
    def __init__(self, host, port):
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.connect((host, port))
    
    def __del__(self):
        try:
            self._sock.close()
        except:
            pass

    def disable_bus_relay(self):
        '''
            Disable the bus relay
        '''
        self._sock.sendall(bytes([RCD_DISABLE_BUS_RELAY]))
    
    def relay_privileged_bus(self):
        '''
            Enable the relay for the privileged bus
        '''
        self._sock.sendall(bytes([RCD_RELAY_PRIVILEGED_BUS]))
    
    def relay_rf_comms_bus(self):
        '''
            Enable the relay for the RF Comms bus
        '''
        self._sock.sendall(bytes([RCD_RELAY_RC_COMMS_BUS]))
    
    def send_byte_stream(self, bytes):
        '''
            Send the following bytes to the currently selected relay bus
        '''
        byte_stream = ByteStream(bytes)
        self._sock.sendall(
            struct.pack(">B", RCD_SEND_BYTE_STREAM_TO_BUS) +
            byte_stream.encode()
        )
    
    def receive_frame(self):
        '''
            Receive bytes from the currently relayed bus until a complete frame
            has been received.
        '''
        data = bytes()
        while True:
            incoming = self._sock.recv(1)
            if len(incoming) == 0:
                return None
            data += incoming
            if len(data) >= 2 and (data[1] & 0x7) + 2 == len(data):
                break
        return Frame(data)
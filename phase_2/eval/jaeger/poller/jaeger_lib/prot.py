import struct

from enum import Enum

from . import *

BROADCAST_ADDRESS = 15


class BusMessageType(Enum):
    STREAM = 1
    BROADCAST = 2
    DATA = 3
    ABORT = 4


class BusMessage:
    def __init__(self, typ, peer_address, data):
        self._typ = typ
        self._peer_address = peer_address
        self._data = data
    
    @property
    def typ(self):
        return self._typ
    
    @property
    def peer_address(self):
        return self._peer_address
    
    @property
    def data(self):
        return self._data
    
    def __str__(self):
        data = ""
        if self.data != None:
            data = "".join(map(lambda x: "{:02x}".format(x), self.data))
        return "<BusMessage: typ={}, from={}, data={}>".format(
            self.typ,
            self.peer_address,
            self.data.hex()
        )

    def __repr__(self):
        return str(self)

class PeerState(Enum):
    '''
        The possible protocol states we can have for a peer
    '''
    READY = 1
    RECEIVING_STREAM = 2
    AWAITING_ACK = 3
    SENDING_STREAM_COMPLETE = 4



class ProtoPeer:
    '''
        Represents a peer, and our state with that peer, on the bus
    '''
    def __init__(self, peer_address):
        self._peer_address = peer_address
        self.reset()

    @property    
    def state(self):
        return self._state
    
    @property
    def address(self):
        return self._peer_address
    
    @property
    def stream_data(self):
        return self._stream_data

    def reset(self):
        self._state = PeerState.READY
        self._stream_data = bytes()
    
    def set_state(self, state):
        self._state = state
    
    def set_stream_data(self, data):
        self._stream_data = data
    
    def append_stream_data(self, data):
        self._stream_data += data
    
    def get_remaining_stream_data(self, len):
        '''
            Retrieve up to len bytes from the current stream data, and advance
            the stream by that many bytes.

            Useful for when we are sending a stream, and need to get the next
            batch of bytes to send
        '''
        data = self._stream_data[:len]
        self._stream_data = self._stream_data[len:]
        return data



class SerialProto:
    '''
        Implements the bus protocol over a serial device.
    '''
    def __init__(self, serial, address):
        self._serial = serial
        self._address = address
        self._peers = {}
    

    @property
    def address(self):
        return self._address

    @property
    def serial(self):
        return self._serial

    def send_frame(self, frame):
        self._serial.send_byte_stream(frame.bytes)

    def send_stream(self, peer_address, data):
        peer = self.get_peer(peer_address)
        if peer.state != PeerState.READY:
            print("Failed to start stream because peer state is {}".format(peer.state))
            return False
        
        else:
            frame = FrameBuilder()\
                .from_(self.address)\
                .to(peer_address)\
                .set_stream()\
                .set_stream_initiate()\
                .set_data(struct.pack("<L", len(data)))\
                .frame()
            self.send_frame(frame)
            peer.set_state(PeerState.AWAITING_ACK)
            peer.set_stream_data(data)
            return True
    

    def send_abort(self, peer_address):
        '''
            Send an abort frame the peer at the given address
        '''
        frame = FrameBuilder()\
            .from_(self.address)\
            .to(peer_address)\
            .set_abort()\
            .frame()
        self.send_frame(frame)


    def get_peer(self, peer_address):
        if peer_address == BROADCAST_ADDRESS:
            return None

        if peer_address not in self._peers:
            self._peers[peer_address] = ProtoPeer(peer_address)
        
        return self._peers[peer_address]
    

    def process_frame_for_peer(self, frame):
        peer = self.get_peer(frame.from_)

        # If we receive an abort flag, reset this peer's state
        if frame.prot_type & PROT_FLAGS_TYPE_ABORT == PROT_FLAGS_TYPE_ABORT:
            peer.set_state(PeerState.READY)
            return BusMessage(BusMessageType.ABORT, peer.address, None)
        
        # Process streams
        elif frame.prot_type & PROT_FLAGS_TYPE_STREAM == PROT_FLAGS_TYPE_STREAM:

            # Ending streams
            if frame.prot_type & PROT_FLAGS_STREAM_END == PROT_FLAGS_STREAM_END:
                # Is this peer acknowledging our previous end of stream message?
                if peer.state == PeerState.SENDING_STREAM_COMPLETE and \
                   frame.prot_type & PROT_FLAGS_STREAM_ACK:
                    peer.reset()
                # Did this peer just tell us to end a stream, but we are not
                # receiving a stream from this peer?
                elif peer.state != PeerState.RECEIVING_STREAM:
                    self.send_abort(frame.frame_)
                    peer.reset()
                # This peer is sending us a stream, and just ended the stream
                else:
                    peer.append_stream_data(frame.data)
                    frame = FrameBuilder()\
                        .from_(self.address)\
                        .to(frame.from_)\
                        .set_stream_ack()\
                        .set_stream_end()\
                        .frame()
                    self.send_frame(frame)
                    stream_data = peer.stream_data
                    peer.reset()
                    return BusMessage(BusMessageType.STREAM, peer.address, stream_data)

            # Received an ack for stream data we are sending
            # Acks with PROT_FLAGS_STREAM_END are handled above
            elif frame.prot_type & PROT_FLAGS_STREAM_ACK == PROT_FLAGS_STREAM_ACK:
                data_to_send = peer.get_remaining_stream_data(6)
                # If this is the last batch of data to send
                if len(data_to_send) < 6:
                    frame = FrameBuilder()\
                        .from_(self.address)\
                        .to(frame.from_)\
                        .set_stream()\
                        .set_stream_end()\
                        .set_data(data_to_send)\
                        .frame()
                    self.send_frame(frame)
                    peer.set_state(PeerState.SENDING_STREAM_COMPLETE)
                else:
                    frame = FrameBuilder()\
                        .from_(self.address)\
                        .to(frame.from_)\
                        .set_stream()\
                        .set_data(data_to_send)\
                        .frame()
                    self.send_frame(frame)
                    peer.set_state(PeerState.AWAITING_ACK)

            # Stream initialization
            elif frame.prot_type & PROT_FLAGS_STREAM_INITIATE == PROT_FLAGS_STREAM_INITIATE:
                if peer.state != PeerState.READY:
                    self.send_abort(frame.frame())
                    peer.reset()
                else:
                    frame = FrameBuilder()\
                        .from_(self.address)\
                        .to(frame.from_)\
                        .set_stream()\
                        .set_stream_ack()\
                        .set_stream_initiate()\
                        .frame()
                    self.send_frame(frame)
                    peer.reset()
                    peer.set_state(PeerState.RECEIVING_STREAM)

            # Receiving normal stream data
            else:
                peer.append_stream_data(frame.data)
                frame = FrameBuilder()\
                    .from_(self.address)\
                    .to(frame.from_)\
                    .set_stream()\
                    .set_stream_ack()\
                    .frame()
                self.send_frame(frame)
        
        # Broadcast frames
        elif frame.prot_type & PROT_FLAGS_TYPE_BROADCAST:
            return BusMessage(BusMessageType.BROADCAST, peer.address, frame.data)
        
        elif frame.prot_type & PROT_FLAGS_TYPE_DATA:
            return BusMessage(BusMessageType.DATA, peer.address, frame.data)
        
        return None

    def process_next_frame(self):
        frame = self._serial.receive_frame()

        if frame == None:
            raise "Connection terminated"
        
        # This frame is destined for us
        if frame.to == self.address:
            return self.process_frame_for_peer(frame)
        else:
            return None
    
    def receive(self):
        '''
            Continue processing frames on the bus until we receive data
        '''
        while True:
            bus_message = self.process_next_frame()
            if bus_message != None:
                return bus_message
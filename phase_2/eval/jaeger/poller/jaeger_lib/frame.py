from . import *

PROT_FLAGS_TYPE_ABORT      = 0b11100000
PROT_FLAGS_TYPE_BROADCAST  = 0b10000000
PROT_FLAGS_TYPE_DATA       = 0b00100000
PROT_FLAGS_TYPE_STREAM     = 0b01000000
PROT_FLAGS_STREAM_ACK      = 0b01100000
PROT_FLAGS_STREAM_END      = 0b00010000
PROT_FLAGS_STREAM_INITIATE = 0b00001000

FLAGS = [
    PROT_FLAGS_TYPE_ABORT,
    PROT_FLAGS_TYPE_BROADCAST,
    PROT_FLAGS_TYPE_DATA,
    PROT_FLAGS_TYPE_STREAM,
    PROT_FLAGS_STREAM_ACK,
    PROT_FLAGS_STREAM_END,
    PROT_FLAGS_STREAM_INITIATE
]

FLAG_NAMES = {
    PROT_FLAGS_TYPE_ABORT:      "ABORT",
    PROT_FLAGS_TYPE_BROADCAST:  "BROADCAST",
    PROT_FLAGS_TYPE_DATA:       "DATA",
    PROT_FLAGS_TYPE_STREAM:     "STREAM",
    PROT_FLAGS_STREAM_ACK:      "STREAM-ACK",
    PROT_FLAGS_STREAM_END:      "STREAM-END",
    PROT_FLAGS_STREAM_INITIATE: "STREAM-INITIATE"
}

def parse_flags(flags):
    result = []
    for flag in FLAGS:
        if flags & flag == flag:
            result.append(flag)

    # Special Cases
    if PROT_FLAGS_TYPE_BROADCAST in result and \
       PROT_FLAGS_TYPE_DATA in result and \
       PROT_FLAGS_TYPE_STREAM in result:
        result.remove(PROT_FLAGS_TYPE_BROADCAST)
        result.remove(PROT_FLAGS_TYPE_DATA)
        result.remove(PROT_FLAGS_TYPE_STREAM)

    if PROT_FLAGS_TYPE_DATA in result and PROT_FLAGS_TYPE_STREAM in result:
        result.remove(PROT_FLAGS_TYPE_DATA)
        result.remove(PROT_FLAGS_TYPE_STREAM)

    return result

def get_flag_names(flags):
    flags = parse_flags(flags)
    return list(map(lambda f: FLAG_NAMES[f], flags))

FLAG_MASK = 0xF8
FRAME_SIZE_MASK = 0x07


class Frame:
    '''
        A frame as understood by the devices in this system.
    '''
    def __init__(self, data=bytes()):
        self._bytes = data

    @property
    def from_(self):
        '''
            Retrieves the address of this frame's sender
        '''
        return (self._bytes[0] >> 4) & 0x0f
    
    @property
    def to(self):
        '''
            Retrieves the address of this frame's recipient.
        '''
        return self._bytes[0] & 0x0f
    
    @property
    def flags(self):
        '''
            Retrieves the flags byte from this frame.
        '''
        return self._bytes[1]
    
    @property
    def prot_type(self):
        '''
            Retrieves the protocol, "Types," from the flags byte.
        '''
        return self._bytes[1] & 0xf8
    
    @property
    def data_len(self):
        '''
            Retrieves the length of the data section for this frame.
        '''
        return self._bytes[1] & 0x07
    
    @property
    def data(self):
        '''
            Returns the data associated with this frame
        '''
        if len(self._bytes) > 2:
            return self._bytes[2:]
        else:
            return bytes([])
    
    @property
    def bytes(self):
        '''
            Returns the raw bytes which make up this frame
        '''
        return self._bytes

    def __str__(self):
        data = "".join(map(lambda b: "{:02x}".format(b), self.data))

        flags = ", ".join(get_flag_names(self.flags))

        return "<Frame: from=0x{:x}, to=0x{:x}, flags=({:02x}, {}), data={}>".format(
            self.from_, self.to, self.flags, flags, data)
        
    def __repr__(self):
        return str(self)


class FrameBuilder:
    '''
        A builder for frames
    '''
    def __init__(self):
        self._from = None
        self._to = None
        self._flags = 0
        self._data = bytes()
    
    def from_(self, address):
        '''
            Set the sender for this frame
        '''
        self._from = address
        return self
    
    def to(self, address):
        '''
            Set the recipient for this frame
        '''
        self._to = address
        return self
    
    def set_abort(self):
        '''
            Set the abort flag in this frame
        '''
        self._flags |= PROT_FLAGS_TYPE_ABORT
        return self
    
    def set_broadcast(self):
        '''
            Set the broadcast flag in this frame
        '''
        self._flags |= PROT_FLAGS_TYPE_BROADCAST
        return self
    
    def set_stream(self):
        '''
            Set the stream flag in this frame
        '''
        self._flags |= PROT_FLAGS_TYPE_STREAM
        return self
    
    def set_stream_ack(self):
        '''
            Set the stream ack flag in this frame
        '''
        self._flags |= PROT_FLAGS_STREAM_ACK
        return self
    
    def set_stream_end(self):
        '''
            Set the stream end flag in this frame
        '''
        self._flags |= PROT_FLAGS_STREAM_END
        return self
    
    def set_stream_initiate(self):
        '''
            Set the stream initialize flag in this frame
        '''
        self._flags |= PROT_FLAGS_STREAM_INITIATE
        return self
    
    def set_data(self, data):
        '''
            Set the data flag in this frame
        '''
        self._data = data
        return self
    
    def frame(self):
        '''
            Get this frame as a Frame object
        '''
        data = bytes([
            (self._from << 4) | (self._to),
            self._flags | len(self._data),
        ])
        data += self._data
        return Frame(data=data)
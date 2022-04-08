from enum import IntEnum
import struct


FSD_FILENAME_SIZE = 0x80
FSD_CONTENTS_SIZE = 0x40


class FsdProtType(IntEnum):
    FILE_EXISTS_REQUEST = 1
    FILE_SIZE_REQUEST = 2
    FILE_CONTENTS_REQUEST = 3
    FILE_EXISTS_RESPONSE = 4
    FILE_SIZE_RESPONSE = 5
    FILE_CONTENTS_RESPONSE = 6


class FileExistsRequest:
    def __init__(self, token, filename):
        self._token = token
        self._filename = filename
    
    @property
    def token(self):
        return self._token
    
    @property
    def filename(self):
        return self._filename
    
    def encode(self):
        return struct.pack("<B", FsdProtType.FILE_EXISTS_REQUEST) + \
            self._token + \
            self._filename[:FSD_FILENAME_SIZE]
    

class FileSizeRequest:
    def __init__(self, token, filename):
        self._token = token
        self._filename = filename
    
    @property
    def token(self):
        return self._token

    @property
    def filename(self):
        return self._filename
    
    def encode(self):
        return struct.pack("<B", FsdProtType.FILE_SIZE_REQUEST) + \
            bytes(self._token) + \
            bytes(self._filename[:FSD_FILENAME_SIZE])


class FileContentsRequest:
    def __init__(self, token, offset, size, filename):
        self._token = token
        self._offset = offset
        self._size = size
        self._filename = filename
    
    @property
    def token(self):
        return self._token
    
    @property
    def offset(self):
        return self._offset
    
    @property
    def size(self):
        return self._size
    
    @property
    def filename(self):
        return self._filename
    
    def encode(self):
        return struct.pack("<B", FsdProtType.FILE_CONTENTS_REQUEST) + \
            bytes(self._token[:8]) + \
            struct.pack("<LL", self._offset, self._size) + \
            self._filename[:FSD_FILENAME_SIZE]


class FileExistsResponse:
    def __init__(self, success):
        self._success = success
    
    @property
    def success(self):
        return self._success


class FileSizeResponse:
    def __init__(self, success, size):
        self._success = success
        self._size = size
    
    @property
    def success(self):
        return self._success
    
    @property
    def size(self):
        return self._size


class FileContentsResponse:
    def __init__(self, success, offset, size, contents):
        self._success = success
        self._offset = offset
        self._size = size
        self._contents = contents
    
    @property
    def success(self):
        return self._success
    
    @property
    def offset(self):
        return self._offset
    
    @property
    def size(self):
        return self._size
    
    @property
    def contents(self):
        return self._contents


def check_if_file_exists(serial_proto, fsd_address, token, filename):
    file_exists_request = FileExistsRequest(token, filename)
    serial_proto.send_stream(fsd_address, file_exists_request.encode())
    bus_message = serial_proto.receive()
    if bus_message.data[0] != FsdProtType.FILE_EXISTS_RESPONSE:
        raise "Got invalid messages back from bus"
    
    typ, success = struct.unpack_from("<BB", bus_message.data)

    return FileExistsResponse(success == 1)

def get_file_size(serial_proto, fsd_address, token, filename):
    file_contents_request = FileSizeRequest(token, filename)
    serial_proto.send_stream(fsd_address, file_contents_request.encode())
    bus_message = serial_proto.receive()
    typ, success, size = struct.unpack_from("<BBL", bus_message.data)
    if typ != FsdProtType.FILE_SIZE_RESPONSE:
        raise "Got invalid messages back from bus"
    
    return FileSizeResponse(success == 1, size)

def get_file_contents(serial_proto, fsd_address, token, filename):
    file_contents_request = FileContentsRequest(token, 0, 64, filename)
    serial_proto.send_stream(fsd_address, file_contents_request.encode())
    bus_message = serial_proto.receive()
    typ, success, offset, size = struct.unpack_from("<BBLL", bus_message.data)
    if typ != FsdProtType.FILE_CONTENTS_RESPONSE:
        raise "Got invalid messages back from bus"
    
    return FileContentsResponse(success == 1, offset, size, bus_message.data[10:10+size])
        
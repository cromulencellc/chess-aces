## Bus Protocol

Devices communicate with each other via the Bus Protocol. This protocol allows for up to 15 devices to communicate with either single-frame broadcast messages, single-frame data messages, or multi-frame streams.

## Frame Format

Each frame consists of a two-byte header, followed by up to 6 data bytes.

### Frame Header

The Frame Header consists of two bytes, the address byte and the flags byte. A breakout of these bytes, and the meaning of their bits is provided below.

```
Bit:   0123 4567 89AB CDEF
       ---- ---- ---- ----
Key:   FFFF TTTT BSDE IXXX

FFFF: 4 bits to represent the frame sender. This is the "From" field.
TTTT: 4 bits to represent the frame recipent. This is the "To" field.
B:    Broadcast Bit
S:    Stream Bit
D:    Data Bit
E:    Stream End Bit
I:    Stream Initiate Bit
XXX:  Size of frame data in bytes.
```

We can combine the values of several bits to represent different types of messages. An exhaustive list of message types, and the flag bits they must set, follows:

* Abort - Sent to a peer on protocol error, or when stream requests can not be satisfied.
* Broadcast - A frame sent to all peers on the bus.
* Stream - A frame which contains streaming data.
* Stream Ack - Acknowledgement of receipt of a Stream frame.
* Stream End - The last frame sent in a stream. Signals stream termination to the peer.
* Stream End Ack - An acknowledgement of a stream's end frame.
* Stream Initiate - Requests the initiation of a stream.
* Stream Initiate Ack - Acknowledges and accepts the inititiation of a stream.

```
                Bit: 0123 4567
                Key: BSDE IXXX
              ABORT: 1110 0000
          BROADCAST: 1000 0...
               DATA: 0010 0...
             STREAM: 0100 0...
         STREAM-ACK: 0110 0000
         STREAM-END: 0101 0...
     STREAM-END-ACK: 0111 0...
    STREAM_INITIATE: 0100 1100
STREAM-INITIATE-ACK: 0110 1...
```

## Broadcasts

Broadcast messages are sent to address `0x0f` with the `BROADCAST` bit set, and are special frames that may be processed by all peers which support the processing of broadcast frames.

Broadcast frames are not acknowledged.

## Data Frames

Data frames may be used when the amount of data to transmit is less than or equal to six bytes, and avoid the requirement to establish a stream.

Data frames are not acknowledged.

## Streams

The protocol supports the streaming of up to `2^32-1` bytes, though peers while normally support streams of much smaller lengths.

We refer to the peer who initiates the stream as the `CLIENT`, and the peer who accepts and receives the stream as the `SERVER`. When the `STREAM` bit is set, the `DATA` bit is interpreted as an acknowledgement bit. The `STREAM-ACK` bits refer to both the `STREAM` and the `DATA` bits being set together.

### Stream Initialization

Streams are initiated with a 6-byte frame. The address byte is filled in with `FROM` field set to the `CLIENT` address, and the `TO` field set to the `SERVER` address. The flags bit has the `STREAM` and `STREAM-INITIATE` bits set, with the data size field set to `4`. The intended length of the data to be streamed is then send in the following 4-bytes as a little-endian 32-bit value.

If the `SERVER` is willing to accept the stream, the client responds with the `STREAM-ACK`, and `STREAM-INITIATE` bits set.

If the `SERVER` is unwilling to accept the stream, the client responds with the `ABORT` bits set.

### Streaming Data

Upon successful initialization of a stream, the `CLIENT` will send its data in consecutive 6-byte chunks until it has completed sending all of its data. If the `CLIENT` attempts to send more data than it has requested, the `SERVER` will respond with an `ABORT` frame, terminating the stream.

Each frame containing stream data will have the `STREAM` bit set, with the data size bits set to the number of data bytes available in the stream.

Each frame is acknowledged by the `SERVER` by setting the `STREAM-ACK` bits.

### Ending the Stream

When the `CLIENT` is sending its last frame, it sets the `STREAM` and `STREAM-END` bits. Upon receiving a frame with the `STREAM` and `STREAM-END` bits set, the `SERVER` sends a response frame with the `STREAM-ACK` and `STREAM-END` bits set, successfully terminating the stream.

## RF Communications Protocol

The RF Communications Protocol (RCP) allows devices external to the JF-DOFSS to communicate with the RCD via an RF comms link. For the purposes of this demonstration, the RF comms link has been replaced with a TCP/IP connection.

The RCP allows for the relaying of messages to and from busses connected to the RF Comms device, as well as injecting arbitrary bitstreams onto the currently relayed bus.

There are four RCP commands, given below:

```
DISABLE_COMMS_RELAY  = 0xF0
RELAY_PRIVILEGED_BUS = 0xF1
RELAY_RF_COMMS_BUS   = 0xF3
RELAY_MESSAGE_TO_BUS = 0xA0
```

These commands allow the RCD to be put in one of three states:

* Not relaying messages.
* Relaying messages from the Privileged Cus.
* Relaying messages from the RF Comms Bus.

If the RCD is relaying messages from a particular bus, the `RELAY_MESSAGE_TO_BUS` command allows the `RCD` to send messages to that bus.

The RCD will always be initially placed in the state where it is not relaying bus messages.

### Disable Comms Relay

The disable comms relay command is a 1-byte command sent to the RCD. Sending the byte `0xF0` to the RCD disables all message relaying.

### Relay Privileged Bus

The relay privileged bus command is a 1-byte command send to the RCD. Sending the byte `0xF1` to the RCD will cause the RCD to begin relaying all messages sent over the privileged bus, as well as allow users to inject messages into the privileged bus via the `RELAY_MESSAGE_TO_BUS` command.

### Relay RF Comms

The relay RF comms bus command is a 1-byte command send to the RCD. Sending the byte `0xF3` to the RCD will cause the RCD to begin relaying all messages sent over the privileged bus, as well as allow users to inject messages into the privileged bus via the `RELAY_MESSAGE_TO_BUS` command.

### Relay Messages to Bus

The relay messages bus command is formatted as follows:

```
+----------------+---------------+-------------------------+
| 1-Byte Command | 1-Byte Length | Variable Length Message |
+----------------+---------------+-------------------------+
```

The 1-byte command will always be `0xA0`. The length field designates the number of bytes in the message field that should be written to the currently relayed bus. When processing this messages, the RCD will copy the `length` bytes from the `message` field verbatim onto the currently relayed bus.

*THIS COMMAND IS ONLY INTENDED TO BE USED IN EMERGENCY SITUATIONS*

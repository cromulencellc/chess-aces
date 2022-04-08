#!/usr/bin/env python3

# Test https://github.com/eclipse/mosquitto/issues/2163
# Does the broker cope with a malformed CONNACK sent to it after a valid CONNECT?

from mosq_test_helper import *

def do_test(proto_ver):
    rc = 1
    keepalive = 10
    connect_packet = mosq_test.gen_connect("connect-connack-2163", keepalive=keepalive, proto_ver=proto_ver)
    connack_packet = mosq_test.gen_connack(rc=0, proto_ver=proto_ver)
    connack_malformed = struct.pack("BBBBB", 0x02, 0x00, 0x01, 0xE0, 0x00)
    connack_malformed = struct.pack("BBBB", 0x29, 0x02, 0x00, 0x01)
    pingreq_packet = mosq_test.gen_pingreq()

    port = mosq_test.get_port()
    broker = mosq_test.start_broker(filename=os.path.basename(__file__), port=port)

    try:
        sock = mosq_test.do_client_connect(connect_packet, connack_packet, port=port)
        sock.send(connack_malformed)
        try:
            mosq_test.do_send_receive(sock, pingreq_packet, b"", "pingreq")
        except ConnectionResetError:
            pass
        sock.close()

        # Does the broker still exist?
        sock = mosq_test.do_client_connect(connect_packet, connack_packet, port=port)
        mosq_test.do_ping(sock)
        sock.close()

        rc = 0
    except mosq_test.TestError:
        pass
    finally:
        broker.terminate()
        broker.wait()
        (stdo, stde) = broker.communicate()
        if rc:
            print(stde.decode('utf-8'))
            print("proto_ver=%d" % (proto_ver))
            exit(rc)


do_test(proto_ver=3)
do_test(proto_ver=4)
do_test(proto_ver=5)
exit(0)

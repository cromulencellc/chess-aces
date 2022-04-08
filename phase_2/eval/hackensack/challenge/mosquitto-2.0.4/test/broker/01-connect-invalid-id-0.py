#!/usr/bin/env python3

# Test whether a CONNECT with a zero length client id results in the correct CONNACK packet.
# MQTT V3.1 only - zero length is invalid.
from mosq_test_helper import *

def do_test(proto_ver):
    rc = 1
    keepalive = 10
    connect_packet = mosq_test.gen_connect("", keepalive=keepalive, proto_ver=proto_ver)
    if proto_ver == 3:
        connack_packet = mosq_test.gen_connack(rc=2, proto_ver=proto_ver)
    elif proto_ver == 4:
        connack_packet = mosq_test.gen_connack(rc=0, proto_ver=proto_ver, properties=None)

    port = mosq_test.get_port()
    broker = mosq_test.start_broker(filename=os.path.basename(__file__), port=port)

    try:
        sock = mosq_test.do_client_connect(connect_packet, connack_packet, port=port)
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
exit(0)

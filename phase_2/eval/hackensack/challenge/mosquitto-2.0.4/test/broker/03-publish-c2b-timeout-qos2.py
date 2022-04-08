#!/usr/bin/env python3

# Test whether a PUBLISH to a topic with QoS 2 results in the correct packet
# flow. This test introduces delays into the flow in order to force the broker
# to send duplicate PUBREC and PUBCOMP messages.

from mosq_test_helper import *


def do_test(port):
    rc = 1
    keepalive = 600
    connect_packet = mosq_test.gen_connect("pub-qos2-timeout-test", keepalive=keepalive, proto_ver=proto_ver)
    connack_packet = mosq_test.gen_connack(rc=0, proto_ver=proto_ver)

    mid = 1926
    publish_packet = mosq_test.gen_publish("pub/qos2/test", qos=2, mid=mid, payload="timeout-message", proto_ver=proto_ver)
    pubrec_packet = mosq_test.gen_pubrec(mid, proto_ver=proto_ver)
    pubrel_packet = mosq_test.gen_pubrel(mid, proto_ver=proto_ver)
    pubcomp_packet = mosq_test.gen_pubcomp(mid, proto_ver=proto_ver)

    port = mosq_test.get_port()
    broker = mosq_test.start_broker(filename=os.path.basename(__file__), port=port)

    try:
        sock = mosq_test.do_client_connect(connect_packet, connack_packet)
        mosq_test.do_send_receive(sock, publish_packet, pubrec_packet, "pubrec")

        # Timeout is 8 seconds which means the broker should repeat the PUBREC.

        mosq_test.expect_packet(sock, "pubrec", pubrec_packet)
        mosq_test.do_send_receive(sock, pubrel_packet, pubcomp_packet, "pubcomp")

        rc = 0

        sock.close()
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


do_test(proto_ver=4)
do_test(proto_ver=5)
exit(0)


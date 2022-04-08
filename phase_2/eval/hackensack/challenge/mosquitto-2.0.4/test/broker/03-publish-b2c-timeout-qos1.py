#!/usr/bin/env python3

# Test whether a SUBSCRIBE to a topic with QoS 2 results in the correct SUBACK packet.

from mosq_test_helper import *


def helper(port):
    connect_packet = mosq_test.gen_connect("test-helper", keepalive=60)
    connack_packet = mosq_test.gen_connack(rc=0)

    mid = 128
    publish_packet = mosq_test.gen_publish("qos1/timeout/test", qos=1, mid=mid, payload="timeout-message")
    puback_packet = mosq_test.gen_puback(mid)

    sock = mosq_test.do_client_connect(connect_packet, connack_packet, connack_error="helper connack")
    mosq_test.do_send_receive(sock, publish_packet, puback_packet, "helper puback")
    sock.close()


def do_test(proto_ver):
    rc = 1
    mid = 3265
    keepalive = 60
    connect_packet = mosq_test.gen_connect("pub-qos1-timeout-test", keepalive=keepalive, proto_ver=proto_ver)
    connack_packet = mosq_test.gen_connack(rc=0, proto_ver=proto_ver)

    subscribe_packet = mosq_test.gen_subscribe(mid, "qos1/timeout/test", 1, proto_ver=proto_ver)
    suback_packet = mosq_test.gen_suback(mid, 1, proto_ver=proto_ver)

    mid = 1
    publish_packet = mosq_test.gen_publish("qos1/timeout/test", qos=1, mid=mid, payload="timeout-message", proto_ver=proto_ver)
    publish_dup_packet = mosq_test.gen_publish("qos1/timeout/test", qos=1, mid=mid, payload="timeout-message", dup=True, proto_ver=proto_ver)
    puback_packet = mosq_test.gen_puback(mid, proto_ver=proto_ver)

    port = mosq_test.get_port()
    broker = mosq_test.start_broker(filename=os.path.basename(__file__), port=port)

    try:
        sock = mosq_test.do_client_connect(connect_packet, connack_packet)
        mosq_test.do_send_receive(sock, subscribe_packet, suback_packet, "suback")

        helper(port)
        # Should have now received a publish command

        mosq_test.expect_packet(sock, "publish", publish_packet)
        # Wait for longer than 5 seconds to get republish with dup set
        # This is covered by the 8 second timeout

        mosq_test.expect_packet(sock, "dup publish", publish_dup_packet)
        sock.send(puback_packet)
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


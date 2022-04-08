#!/usr/bin/env python3

# Does an interrupted QoS 1 flow from broker to client get handled correctly?

from mosq_test_helper import *

def helper(port):
    connect_packet = mosq_test.gen_connect("test-helper", keepalive=60)
    connack_packet = mosq_test.gen_connack(rc=0)

    mid = 128
    publish_packet = mosq_test.gen_publish("qos1/disconnect/test", qos=1, mid=mid, payload="disconnect-message")
    puback_packet = mosq_test.gen_puback(mid)
    sock = mosq_test.do_client_connect(connect_packet, connack_packet, connack_error="helper connack", port=port)
    mosq_test.do_send_receive(sock, publish_packet, puback_packet, "helper puback")
    sock.close()


def do_test(proto_ver):
    port = mosq_test.get_port()

    rc = 1
    mid = 3265
    keepalive = 60
    connect_packet = mosq_test.gen_connect("pub-qos1-disco-test", keepalive=keepalive, clean_session=False, proto_ver=proto_ver, session_expiry=60)
    connack1_packet = mosq_test.gen_connack(flags=0, rc=0, proto_ver=proto_ver)
    connack2_packet = mosq_test.gen_connack(flags=1, rc=0, proto_ver=proto_ver)

    subscribe_packet = mosq_test.gen_subscribe(mid, "qos1/disconnect/test", 1, proto_ver=proto_ver)
    suback_packet = mosq_test.gen_suback(mid, 1, proto_ver=proto_ver)

    mid = 1
    publish_packet = mosq_test.gen_publish("qos1/disconnect/test", qos=1, mid=mid, payload="disconnect-message", proto_ver=proto_ver)
    publish_dup_packet = mosq_test.gen_publish("qos1/disconnect/test", qos=1, mid=mid, payload="disconnect-message", dup=True, proto_ver=proto_ver)
    puback_packet = mosq_test.gen_puback(mid, proto_ver=proto_ver)

    mid = 3266
    publish2_packet = mosq_test.gen_publish("qos1/outgoing", qos=1, mid=mid, payload="outgoing-message", proto_ver=proto_ver)
    puback2_packet = mosq_test.gen_puback(mid, proto_ver=proto_ver)

    broker = mosq_test.start_broker(filename=os.path.basename(__file__), port=port)

    try:
        sock = mosq_test.do_client_connect(connect_packet, connack1_packet, port=port)

        mosq_test.do_send_receive(sock, subscribe_packet, suback_packet, "suback")

        helper(port)
        # Should have now received a publish command

        mosq_test.expect_packet(sock, "publish", publish_packet)
        # Send our outgoing message. When we disconnect the broker
        # should get rid of it and assume we're going to retry.
        sock.send(publish2_packet)
        sock.close()

        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(60) # 60 seconds timeout is much longer than 5 seconds message retry.
        sock.connect(("localhost", port))

        mosq_test.do_send_receive(sock, connect_packet, connack2_packet, "connack")

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


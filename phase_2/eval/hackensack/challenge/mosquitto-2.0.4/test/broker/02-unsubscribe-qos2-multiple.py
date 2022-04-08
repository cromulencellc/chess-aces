#!/usr/bin/env python3

# Test whether a UNSUBSCRIBE to multiple topics with QoS 2 results in the
# correct UNSUBACK packet, when one subscription exists and the other does not.

from mosq_test_helper import *

def do_test(proto_ver):
    rc = 1
    keepalive = 60
    connect_packet = mosq_test.gen_connect("unsubscribe-qos2-test", keepalive=keepalive, proto_ver=proto_ver)
    connack_packet = mosq_test.gen_connack(rc=0, proto_ver=proto_ver)

    mid = 1
    subscribe_packet = mosq_test.gen_subscribe(mid, "qos2/two", 2, proto_ver=proto_ver)
    suback_packet = mosq_test.gen_suback(mid, 2, proto_ver=proto_ver)

    mid = 3
    unsubscribe_packet = mosq_test.gen_unsubscribe_multiple(mid, ["qos2/one", "qos2/two"], proto_ver=proto_ver)
    if proto_ver == 5:
        unsuback_packet = mosq_test.gen_unsuback(mid, proto_ver=proto_ver, reason_code=[17, 0])
    else:
        unsuback_packet = mosq_test.gen_unsuback(mid, proto_ver=proto_ver)

    port = mosq_test.get_port()
    broker = mosq_test.start_broker(filename=os.path.basename(__file__), port=port)

    try:
        sock = mosq_test.do_client_connect(connect_packet, connack_packet, port=port)
        mosq_test.do_send_receive(sock, subscribe_packet, suback_packet, "suback")
        mosq_test.do_send_receive(sock, unsubscribe_packet, unsuback_packet, "unsuback")

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

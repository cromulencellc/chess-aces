#!/usr/bin/env python3

# Test whether a UNSUBSCRIBE with no topic results in a disconnect. MQTT-3.10.3-2

from mosq_test_helper import *

def gen_unsubscribe_invalid_no_topic(mid):
    pack_format = "!BBH"
    return struct.pack(pack_format, 162, 2, mid)

def do_test(proto_ver):
    rc = 1
    mid = 3
    keepalive = 60
    connect_packet = mosq_test.gen_connect("unsubscribe-invalid-no-topic-test", keepalive=keepalive, proto_ver=proto_ver)
    connack_packet = mosq_test.gen_connack(rc=0, proto_ver=proto_ver)

    unsubscribe_packet = gen_unsubscribe_invalid_no_topic(mid)

    port = mosq_test.get_port()
    broker = mosq_test.start_broker(filename=os.path.basename(__file__), port=port)

    try:
        sock = mosq_test.do_client_connect(connect_packet, connack_packet, port=port)
        if proto_ver == 4:
            mosq_test.do_send_receive(sock, unsubscribe_packet, b"", "disconnect")
        else:
            disconnect_packet = mosq_test.gen_disconnect(proto_ver=5, reason_code=mqtt5_rc.MQTT_RC_MALFORMED_PACKET)
            mosq_test.do_send_receive(sock, unsubscribe_packet, disconnect_packet, "disconnect")

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

#!/usr/bin/env python3

# Test whether a SUBSCRIBE to a topic with an invalid UTF-8 topic fails

from mosq_test_helper import *

def do_test(proto_ver):
    rc = 1
    mid = 53
    keepalive = 60
    connect_packet = mosq_test.gen_connect("subscribe-invalid-utf8", keepalive=keepalive, proto_ver=proto_ver)
    connack_packet = mosq_test.gen_connack(rc=0, proto_ver=proto_ver)

    subscribe_packet = mosq_test.gen_subscribe(mid, "invalid/utf8", 0, proto_ver=proto_ver)
    b = list(struct.unpack("B"*len(subscribe_packet), subscribe_packet))
    b[13] = 0 # Topic should never have a 0x0000
    subscribe_packet = struct.pack("B"*len(b), *b)

    suback_packet = mosq_test.gen_suback(mid, 0, proto_ver=proto_ver)

    port = mosq_test.get_port()
    broker = mosq_test.start_broker(filename=os.path.basename(__file__), port=port)

    try:
        time.sleep(0.5)

        sock = mosq_test.do_client_connect(connect_packet, connack_packet, port=port)
        if proto_ver == 4:
            mosq_test.do_send_receive(sock, subscribe_packet, b"", "suback")
        else:
            disconnect_packet = mosq_test.gen_disconnect(proto_ver=5, reason_code = mqtt5_rc.MQTT_RC_MALFORMED_PACKET)
            mosq_test.do_send_receive(sock, subscribe_packet, disconnect_packet, "suback")

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

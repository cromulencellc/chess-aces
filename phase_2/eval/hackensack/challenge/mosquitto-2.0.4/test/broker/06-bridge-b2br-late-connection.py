#!/usr/bin/env python3

# Does a bridge queue up messages correctly if the remote broker starts up late?

from mosq_test_helper import *

def write_config(filename, port1, port2, protocol_version):
    with open(filename, 'w') as f:
        f.write("port %d\n" % (port2))
        f.write("allow_anonymous true\n")
        f.write("\n")
        f.write("connection bridge_sample\n")
        f.write("address 127.0.0.1:%d\n" % (port1))
        f.write("topic bridge/# out 1\n")
        f.write("notifications false\n")
        f.write("bridge_attempt_unsubscribe false\n")
        f.write("bridge_protocol_version %s\n" % (protocol_version))

def do_test(proto_ver):
    if proto_ver == 4:
        bridge_protocol = "mqttv311"
        proto_ver_connect = 128+4
    else:
        bridge_protocol = "mqttv50"
        proto_ver_connect = 5

    (port1, port2) = mosq_test.get_port(2)
    conf_file = os.path.basename(__file__).replace('.py', '.conf')
    write_config(conf_file, port1, port2, bridge_protocol)

    rc = 1
    keepalive = 60
    client_id = socket.gethostname()+".bridge_sample"
    connect_packet = mosq_test.gen_connect(client_id, keepalive=keepalive, clean_session=False, proto_ver=proto_ver_connect)
    connack_packet = mosq_test.gen_connack(rc=0, proto_ver=proto_ver)

    c_connect_packet = mosq_test.gen_connect("client", keepalive=keepalive, proto_ver=proto_ver)
    c_connack_packet = mosq_test.gen_connack(rc=0, proto_ver=proto_ver)

    mid = 1
    publish_packet = mosq_test.gen_publish("bridge/test", qos=1, mid=mid, payload="message", proto_ver=proto_ver)
    puback_packet = mosq_test.gen_puback(mid, proto_ver=proto_ver)

    ssock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    ssock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    ssock.settimeout(40)
    ssock.bind(('', port1))
    ssock.listen(5)

    broker = mosq_test.start_broker(filename=os.path.basename(__file__), port=port2, use_conf=True)

    try:
        (bridge, address) = ssock.accept()
        bridge.settimeout(20)

        client = mosq_test.do_client_connect(c_connect_packet, c_connack_packet, timeout=20, port=port2)
        mosq_test.do_send_receive(client, publish_packet, puback_packet, "puback")
        client.close()
        # We've now sent a message to the broker that should be delivered to us via the bridge

        mosq_test.expect_packet(bridge, "connect", connect_packet)
        bridge.send(connack_packet)

        mosq_test.expect_packet(bridge, "publish", publish_packet)
        rc = 0

        bridge.close()
    except mosq_test.TestError:
        pass
    finally:
        os.remove(conf_file)
        try:
            bridge.close()
        except NameError:
            pass

        broker.terminate()
        broker.wait()
        (stdo, stde) = broker.communicate()
        ssock.close()
        if rc:
            print(stde.decode('utf-8'))
            exit(rc)

do_test(proto_ver=4)
do_test(proto_ver=5)

exit(0)


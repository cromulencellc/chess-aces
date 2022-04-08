#!/usr/bin/env python3

from mosq_test_helper import *

port = mosq_test.get_lib_port()


resp_topic = "response/topic"
pub_topic = "request/topic"

rc = 1
keepalive = 60
connect1_packet = mosq_test.gen_connect("request-test", keepalive=keepalive, proto_ver=5)
connect2_packet = mosq_test.gen_connect("response-test", keepalive=keepalive, proto_ver=5)
connack_packet = mosq_test.gen_connack(rc=0, proto_ver=5)

mid = 1
subscribe1_packet = mosq_test.gen_subscribe(mid, resp_topic, 0, proto_ver=5)
subscribe2_packet = mosq_test.gen_subscribe(mid, pub_topic, 0, proto_ver=5)
suback_packet = mosq_test.gen_suback(mid, 0, proto_ver=5)


props = mqtt5_props.gen_string_prop(mqtt5_props.PROP_RESPONSE_TOPIC, resp_topic)
publish1_packet = mosq_test.gen_publish(pub_topic, qos=0, payload="action", proto_ver=5, properties=props)

publish2_packet = mosq_test.gen_publish(resp_topic, qos=0, payload="a response", proto_ver=5)


sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.settimeout(10)
sock.bind(('', port))
sock.listen(5)

env = dict(os.environ)
env['LD_LIBRARY_PATH'] = '../../lib:../../lib/cpp'
try:
    pp = env['PYTHONPATH']
except KeyError:
    pp = ''
env['PYTHONPATH'] = '../../lib/python:'+pp
client1 = mosq_test.start_client(filename="03-request-response-1.log", cmd=["c/03-request-response-1.test"], env=env, port=port)

try:
    (conn1, address) = sock.accept()
    conn1.settimeout(10)

    client2 = mosq_test.start_client(filename="03-request-response-2.log", cmd=["c/03-request-response-2.test"], env=env, port=port)
    (conn2, address) = sock.accept()
    conn2.settimeout(10)

    mosq_test.do_receive_send(conn1, connect1_packet, connack_packet, "connect1")
    mosq_test.do_receive_send(conn2, connect2_packet, connack_packet, "connect2")

    mosq_test.do_receive_send(conn1, subscribe1_packet, suback_packet, "subscribe1")
    mosq_test.do_receive_send(conn2, subscribe2_packet, suback_packet, "subscribe2")

    mosq_test.expect_packet(conn1, "publish1", publish1_packet)
    conn2.send(publish1_packet)

    mosq_test.expect_packet(conn2, "publish2", publish2_packet)
    rc = 0

    conn1.close()
    conn2.close()
except mosq_test.TestError:
    pass
finally:
    client1.terminate()
    client1.wait()
    client2.terminate()
    client2.wait()
    if rc:
        (stdo, stde) = client1.communicate()
        print(stde)
        (stdo, stde) = client2.communicate()
        print(stde)
    sock.close()

exit(rc)

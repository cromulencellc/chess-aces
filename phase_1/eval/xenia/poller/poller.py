import os
import random
import re
import socket
import sys
import time
import threading

from changes import Changes, ChangeType
from comms import Comms
from graph import Graph

def fatal(msg):
    print(msg)
    sys.exit(-1)


for var in ["HOST", "PORT"]:
    if var not in os.environ:
        fatal("Missing {} environment variable".format(var))

HOST = os.environ["HOST"]
PORT = int(os.environ["PORT"])
SEED = random.randint(0, 1000000000)
LENGTH = random.randint(1, 100)
if "SEED" in os.environ:
    SEED = int(os.environ["SEED"])
if "LENGTH" in os.environ:
    LENGTH = int(os.environ["LENGTH"])
random.seed(SEED)


def escape_encode_string(s):
    if type(s) == str:
        s = bytes(s, 'utf8')
    result = b""
    for c in s:
        if c >= ord('A') and c <= ord('Z'):
            result += bytes([c])
        elif c >= ord('a') and c <= ord('z'):
            result += bytes([c])
        elif c >= ord('0') and c <= ord('9'):
            result += bytes([c])
        else:
            result += bytes("\\x{:02x}".format(c), 'utf8')
    return result


class Client(Comms):
    def __init__(self, host, port):
        self._comms = Comms(host, port)
        self._comms.recv_until(">")
        self._comms.sendall("2\n")
        time.sleep(0.05)
    
    def exit(self):
        self._comms.sendall("exit;")
    
    def add(self, node_id: int):
        self._comms.sendall("add({});".format(node_id))
    
    def remove(self, node_id: int):
        self._comms.sendall("remove({});".format(node_id))
    
    def get_successors(self, node_id):
        self._comms.sendall("get_successors(get({}));".format(node_id))
    
    def get_predecessors(self, node_id):
        self._comms.sendall("get_predecessors(get({}));".format(node_id))
    
    def set_property(self, node_id: int, key: str, value: str):
        self._comms.sendall(
            b"set_property(get(" + \
            bytes(str(node_id), 'utf8') + \
            b"), \"" + \
            escape_encode_string(key) + \
            b"\", \"" + \
            escape_encode_string(value) + \
            b"\");"
        )
    
    def remove_property(self, node_id: int, key: str):
        self._comms.sendall(
            b"remove_property(get(" + \
            bytes(str(node_id), 'utf8') + \
            b"), \"" + \
            escape_encode_string(key) + 
            b"\");"
        )
    
    def add_successors(self, node_id: int, successor_id: int):
        self._comms.sendall("add_successors(get({}), get({}));".format(
            node_id, successor_id
        ))
    
    def add_predecessors(self, node_id: int, predecessor_id: int):
        self._comms.sendall("add_predecessors(get({}), get({}));".format(
            node_id, predecessor_id
        ))

    def remove_successor(self, predecessor_id: int, successor_id: int):
        self._comms.sendall(
            "set_successors(\
                get({}), \
                where get_successors(get({})) not in get({})\
            );".format(predecessor_id, predecessor_id, successor_id))

    def remove_predecessor(self, predecessor_id: int, successor_id: int):
        self._comms.sendall(
            "set_predecessors(\
                get({}), \
                where get_predecessors(get({})) not in get({})\
            );".format(successor_id, successor_id, predecessor_id))
    
    def get_vertex(self, node_id: int):
        self._comms.sendall("get({});".format(node_id))

    def remove_vertex(self, node_id: int):
        self._comms.sendall("remove({});".format(node_id))

    def recv_vertex(self):
        '''
            Reads from the socket until it finds a response which includes a
            vertex.
            Returns the node_id and properties as
                node_id, [(key, value), ...]
        '''
        while True:
            line = self._comms.recvline()
            print("[recv_vertex] {}".format(line))
            match = re.match(r"(0x[0-9a-f]+).*?(\[.*?=.*?\])*\n", str(line, "utf8"))
            if match == None:
                continue
            node_id, props = match.groups()
            print(node_id, node_id[2:], int(node_id[2:], 16))
            node_id = int(node_id[2:], 16)
            if props != None:
                props = re.findall(r"(\[(.*?)=(.*?)\])", props)
                props = list(map(lambda p: (p[1], p[2]), props))
            break
        return node_id, props

    def apply_changes(self, changes: Changes):
        for change in changes.changes:
            if change.typ == ChangeType.ADD_VERTEX:
                print("[change_type] {} node_id={}".format(change.typ, change.node_id))
                self.add(change.node_id)
            elif change.typ == ChangeType.REMOVE_VERTEX:
                print("[change_type] {} node_id={}".format(change.typ, change.node_id))
                self.remove(change.node_id)
            elif change.typ == ChangeType.SET_PROPERTY:
                print(
                    "[change_type] {} key={}, value={}".format(
                    change.typ,
                    change.property.key,
                    change.property.value
                ))
                self.set_property(
                    change.node_id,
                    change.property.key,
                    change.property.value
                )
            elif change.typ == ChangeType.REMOVE_PROPERTY:
                print(
                    "[change_type] {} key={}".format(
                    change.typ,
                    change.key
                ))
                self.remove_property(change.node_id, change.key)
            elif change.typ == ChangeType.ADD_SUCCESSOR:
                print(
                    "[change_type] {} predecessor={}, successor={}".format(
                    change.typ,
                    change.predecessor_id,
                    change.successor_id
                ))
                self.add_successors(change.predecessor_id, change.successor_id)
            elif change.typ == ChangeType.ADD_PREDECESSOR:
                print(
                    "[change_type] {} predecessor={}, successor={}".format(
                    change.typ,
                    change.predecessor_id,
                    change.successor_id
                ))
                self.add_predecessors(change.successor_id, change.predecessor_id)
            elif change.typ == ChangeType.REMOVE_SUCCESSOR:
                print(
                    "[change_type] {} predecessor={}, successor={}".format(
                    change.typ,
                    change.predecessor_id,
                    change.successor_id
                ))
                self.remove_successor(change.predecessor_id, change.successor_id)
            elif change.typ == ChangeType.REMOVE_PREDECESSOR:
                print(
                    "[change_type] {} predecessor={}, successor={}".format(
                    change.typ,
                    change.predecessor_id,
                    change.successor_id
                ))
                self.remove_predecessor(change.predecessor_id, change.successor_id)
            else:
                print("[change_type] Invalid change type: {}".format(change.typ))
                sys.exit(-1)

    def check_graph_vertices(self, graph):
        for vertex in graph.vertices:
            self.get_vertex(vertex.id)
            vertex_id, props = self.recv_vertex()
            if vertex_id != vertex.id:
                print("[-] check_graph_vertices mismatched vertex ids: {}!={}".format(
                    vertex.id, vertex_id))
                return False
            for key in vertex.properties:
                p = vertex.properties[key]
                if (p.key, p.value) not in props:
                    print("[-] check_graph_vertices {}={} not in returned properties".format(
                        pkey, p.value
                    ))
                    return False
            print("[check] Vertex {} verified".format(vertex.id))
        return True
    
    def check_graph_edges(self, graph):
        for vertex in graph.vertices:
            successors = list(map(lambda e: e.tail, graph.get_successors(vertex.id)))
            if len(successors) == 0:
                continue
            self.get_successors(vertex.id)
            print("[check] vertex: {} successors: {}".format(vertex.id, successors))
            num_successors = len(successors)
            for i in range(num_successors):
                vertex_id, props = self.recv_vertex()
                if vertex_id not in successors:
                    print("[-] check_graph_edges {} incorrect successor {}".format(
                        vertex.id, vertex_id
                    ))
                    return False
                del successors[successors.index(vertex_id)]
            if len(successors) > 0:
                print("[-] Missing successors for {}: {} (Also this should have hung?)".format(
                    vertex.id, successors
                ))
                return False

            predecessors = list(map(lambda e: e.head, graph.get_predecessors(vertex.id)))
            if len(predecessors) == 0:
                continue
            self.get_predecessors(vertex.id)
            num_predecessors = len(predecessors)
            for i in range(num_predecessors):
                vertex_id, props = self.recv_vertex()
                if vertex_id not in predecessors:
                    print("[-] check_graph_edges {} incorrect predecessor {}".format(
                        vertex.id, vertex_id
                    ))
                    return False
                del predecessors[predecessors.index(vertex_id)]
            if len(predecessors) > 0:
                print("[-] Missing predecessors for {}: {} (Also this should have hung?)".format(
                    vertex.id, predecessors
                ))
                return False
        return True 
    
    def check_properties(self, graph):
        for vertex in graph.vertices:
            if len(vertex.properties) == 0:
                continue
            for key in vertex.properties:
                prop = vertex.properties[key]
                self._comms.sendall(
                    "for x in get({}) where get(x) in has_property(\"{}\");".format(
                        vertex.id, prop.key
                    ))
                vertex_id, props = self.recv_vertex()
                if vertex_id != vertex.id:
                    print("[-] has_property fail")
                    return False
                self._comms.sendall(
                    "for x in get({}) where get(x) in property_is(\"{}\", \"{}\");".format(
                        vertex.id, prop.key, prop.value
                    ))
                vertex_id, props = self.recv_vertex()
                if vertex_id != vertex.id:
                    print("[-] property_is fail")
                    return False
        return True


    def check_graph(self, graph):
        return self.check_graph_vertices(graph) and \
               self.check_graph_edges(graph) and \
               self.check_properties(graph)

        


print("SEED={}".format(SEED))
print("LENGTH={}".format(LENGTH))


for i in range(LENGTH):
    print("[i] i = {}".format(i))
    num_changes = random.randint(1, 100)

    client = Client(HOST, PORT)
    graph = Graph()
    for i in range(num_changes):
        changes = Changes(graph)
        client.apply_changes(changes)
        if not client.check_graph(graph):
            print("check graph failed")
            sys.exit(-1)

client.exit()
sys.exit(0)
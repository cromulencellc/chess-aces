import graph
import random
import sys

from enum import Enum


class ChangeType(Enum):
    ADD_VERTEX = 0
    REMOVE_VERTEX = 1
    SET_PROPERTY = 2
    REMOVE_PROPERTY = 3
    ADD_SUCCESSOR = 4
    ADD_PREDECESSOR = 5
    REMOVE_SUCCESSOR = 6
    REMOVE_PREDECESSOR = 7


class Change:
    '''
        A change is a graph modification
    '''
    def __init__(self, change_type: ChangeType):
        self._type = change_type
    
    @property
    def typ(self):
        return self._type

    def set_node_id(self, node_id: int):
        self.node_id = node_id
        return self
    
    def set_successor_id(self, successor_id: int):
        self.successor_id = successor_id
        return self
    
    def set_predecessor_id(self, predecessor_id: int):
        self.predecessor_id = predecessor_id
        return self
    
    def set_property(self, prop: graph.Property):
        self.property = prop
        return self
    
    def set_key(self, key: str):
        self.key = key
        return self


class Changes:
    def __init__(self, graph):
        self.graph = graph
        self.changes = []

        change_type = random.randint(0, len(ChangeType) - 1)
        change_type = ChangeType(change_type)

        if change_type == ChangeType.ADD_VERTEX:
            self.add_vertex()
        elif change_type == ChangeType.REMOVE_VERTEX:
            self.remove_vertex()
        elif change_type == ChangeType.SET_PROPERTY:
            self.set_property()
        elif change_type == ChangeType.REMOVE_PROPERTY:
            self.remove_property()
        elif change_type == ChangeType.ADD_SUCCESSOR:
            self.add_successor()
        elif change_type == ChangeType.ADD_PREDECESSOR:
            self.add_predecessor()
        elif change_type == ChangeType.REMOVE_SUCCESSOR:
            self.remove_successor()
        elif change_type == ChangeType.REMOVE_PREDECESSOR:
            self.remove_predecessor()
        else:
            print("Invalid change type!: {}".format(change_type))
            sys.exit(-1)

    def add_vertex(self):
        vertex_id = self.graph.random_add_vertex()
        self.changes.append(
            Change(ChangeType.ADD_VERTEX).set_node_id(vertex_id))
    
    def remove_vertex(self):
        if self.graph.vertices_empty:
            self.add_vertex()
        vertex_id = self.graph.random_remove_vertex()
        self.changes.append(
            Change(ChangeType.REMOVE_VERTEX).set_node_id(vertex_id))
    
    def set_property(self):
        if self.graph.vertices_empty:
            self.add_vertex()
        vertex_id, p = self.graph.random_set_property()
        self.changes.append(
            Change(ChangeType.SET_PROPERTY)
                .set_node_id(vertex_id)
                .set_property(p))

    def remove_property(self):
        if self.graph.vertices_empty:
            self.add_vertex()
        if len(self.graph.get_vertices_with_properties()) == 0:
            self.set_property()
        vertex_id, key = self.graph.random_remove_property()
        self.changes.append(
            Change(ChangeType.REMOVE_PROPERTY)
                .set_node_id(vertex_id)
                .set_key(key)
        )
    
    def add_successor(self):
        while len(self.graph.get_pairs_without_edges()) == 0:
            self.add_vertex()
        head, tail = self.graph.random_add_edge()
        self.changes.append(
            Change(ChangeType.ADD_SUCCESSOR)
                .set_predecessor_id(head)
                .set_successor_id(tail)
        )
    
    def add_predecessor(self):
        while len(self.graph.get_pairs_without_edges()) == 0:
            self.add_vertex()
        head, tail = self.graph.random_add_edge()
        self.changes.append(
            Change(ChangeType.ADD_PREDECESSOR)
                .set_predecessor_id(head)
                .set_successor_id(tail)
        )
    
    def remove_successor(self):
        if len(self.graph.edges) == 0:
            self.add_successor()
        head, tail = self.graph.random_remove_successor()
        self.changes.append(
            Change(ChangeType.REMOVE_SUCCESSOR)
                .set_predecessor_id(head)
                .set_successor_id(tail)
        )
    
    def remove_predecessor(self):
        if len(self.graph.edges) == 0:
            self.add_successor()
        head, tail = self.graph.random_remove_successor()
        self.changes.append(
            Change(ChangeType.REMOVE_PREDECESSOR)
                .set_predecessor_id(head)
                .set_successor_id(tail)
        )

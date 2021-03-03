import random

class Property:
    def __init__(self, key, value):
        self._key = key
        self._value = value
    
    @property
    def key(self):
        return self._key
    
    @property
    def value(self):
        return self._value


class Vertex:
    def __init__(self, id_: int):
        if type(id_) != int:
            print("type not int")
            raise
        self._id = id_
        self._next_key = 0
        self._properties = {}
    
    def get_next_key_value_pair(self):
        next_key = self._next_key
        self._next_key += 1
        return "key_{}".format(next_key), "value_{}".format(next_key)
    
    def get_next_property(self):
        key, value = self.get_next_key_value_pair()
        return Property(key, value)

    def set_property(self, p: Property):
        self._properties[p.key] = p
    
    def get_property(self, key: str) -> Property:
        return self._properties[key]

    def remove_property(self, key: str):
        del self._properties[key]
    
    @property
    def properties(self):
        return self._properties

    @property
    def id(self):
        return self._id
    
    @property
    def properties_len(self):
        return len(self._properties)

    def random_set_property(self):
        '''
            Adds a new property to the Vertex
            Returns the Property added
        '''
        p = self.get_next_property()
        self.set_property(p)
        return p
    
    def random_remove_property(self):
        '''
            Removes a random property from the Vertex
            Returns the key of the property removed
        '''
        property_keys = list(self._properties.keys())
        key = random.choice(property_keys)
        self.remove_property(key)
        return key


class Edge:
    def __init__(self, head, tail):
        self._head = head
        self._tail = tail
    
    @property
    def head(self):
        return self._head
    
    @property
    def tail(self):
        return self._tail


class Graph:
    def __init__(self):
        self._next_vertex_id = 1
        self._vertices = []
        self._edges = []
    
    def get_next_vertex_id(self):
        next_vertex_id = self._next_vertex_id
        self._next_vertex_id += 1
        return next_vertex_id
    
    def add_vertex(self, vertex_id):
        if vertex_id in self.vertex_ids:
            return
        vertex = Vertex(vertex_id)
        self._vertices.append(vertex)
        return vertex
    
    def remove_vertex(self, vertex_id):
        vertex = self.get_vertex(vertex_id)
        self._vertices = list(filter(lambda v: v.id != vertex_id, self.vertices))
        self._edges = \
            list(filter(
                lambda e: e.head != vertex_id and e.tail != vertex_id,
                self.edges
            ))
        return vertex
    
    def get_vertex(self, vertex_id):
        for vertex in self._vertices:
            if vertex.id == vertex_id:
                return vertex
    
    @property
    def vertices(self):
        return self._vertices
    
    @property
    def vertices_len(self) -> int:
        return len(self._vertices)

    @property
    def vertices_empty(self) -> bool:
        return self.vertices_len == 0

    @property
    def vertex_ids(self):
        return list(map(lambda v: v.id, self._vertices))
    
    @property
    def edges(self):
        return self._edges
    
    def edge_exists(self, head, tail):
        return any(map(lambda e: e.head == head and e.tail == tail, self.edges))
    
    def add_edge(self, head, tail):
        self._edges.append(Edge(head, tail))
    
    def get_successors(self, head):
        return list(filter(lambda x: x.head == head, self._edges))
    
    def get_predecessors(self, tail):
        return list(filter(lambda x: x.tail == tail, self._edges))

    def get_vertices_with_properties(self):
        return list(filter(lambda v: v.properties_len > 0, self.vertices))
    
    def get_pairs_without_edges(self):
        '''
            Returns predecessor -> successor ids for vertices which do not have
            edges between them
        '''
        pairs = []
        for p in self.vertices:
            for s in self.vertices:
                pairs.append((p.id, s.id))
        pairs = list(filter(lambda p: not self.edge_exists(p[0], p[1]), pairs))
        return pairs
    
    def random_get_vertex(self) -> Vertex:
        '''
            Randomly pick an existing vertex
            Returns the vertex
        '''
        if self.vertices_empty:
            print("Tried to get a random vertex, but vertices empty")
            return None
        
        return random.choice(self.vertices)

    def random_add_vertex(self) -> int:
        '''
            Add a vertex
            Returns the vertex id added
        '''
        vertex_id = self.get_next_vertex_id()
        self.add_vertex(vertex_id)
        return vertex_id
    
    def random_remove_vertex(self) -> int:
        '''
            Randomly removes a vertex
            If no vertices exist, returns None
            Else returns the vertex id removed
        '''
        if self.vertices_empty:
            return None
        
        vertex = self.random_get_vertex()
        self.remove_vertex(vertex.id)
        return vertex.id
    
    def random_set_property(self) -> (int, Property):
        '''
            Adds a property to a randomly selected Vertex
            If no vertices exist, returns None
            Else returns the vertex id and Property
        '''
        if self.vertices_empty:
            return None
        
        vertex = self.random_get_vertex()
        p = vertex.random_set_property()
        return vertex.id, p

    def random_remove_property(self) -> (int, str):
        '''
            Removes a random property from a randomly selected Vertex
            If no vertices exist with properties, returns None
            Else returnes the vertex id, and the property key removed
        '''
        vertices_with_properties = self.get_vertices_with_properties()
        if len(vertices_with_properties) == 0:
            return None
        
        vertex = random.choice(vertices_with_properties)
        key = vertex.random_remove_property()
        return vertex.id, key

    def random_add_edge(self) -> (int, int):
        '''
            Adds an edge between two random vertices
            If two or more vertices without an edge between them do not exist,
            returns None
            Else returns the predecessor id, and successor id
        '''
        pairs = self.get_pairs_without_edges()
        if len(pairs) == 0:
            return None

        head, tail = random.choice(pairs)
        self.add_edge(head, tail)
        return head, tail
    
    def random_remove_successor(self) -> (int, int):
        '''
            Removes a random edge between two vertices
            If no edges exist to remove, returns None
            Else returns predecessor id, and successor id
        '''
        if len(self.edges) == 0:
            return None
        
        index = random.randint(0, len(self.edges) - 1)
        edge = self.edges[index]
        del self.edges[index]
        return edge.head, edge.tail
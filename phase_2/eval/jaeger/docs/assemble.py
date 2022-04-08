FILES = [
    "JS_DOFSS.md",
    "DEVICES.md",
    "PROTOCOLS.md",
    "BUS_PROTOCOL.md",
    "ACCESS_CONTROL_PROTOCOL.md",
    "RF_COMMS_PROTOCOL.md",
    "FILE_STORAGE_PROTOCOL.md",
    "PERIPHERALS.md"
]

def read_file(filename):
    fh = open(filename, "rb")
    contents = bytes()

    while True:
        tmp = fh.read()
        if len(tmp) == 0:
            break
        else:
            contents += tmp
    return contents

final_markdown = bytes()
for filename in FILES:
    print(filename)
    final_markdown += read_file(filename)
    final_markdown += b"\n\n"

fh = open("README.md", "wb")
fh.write(final_markdown)
fh.close()
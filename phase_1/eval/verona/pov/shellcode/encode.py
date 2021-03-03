import sys

fh = open(sys.argv[1], "rb")
raw = fh.read()
fh.close()

print(".data")
for byte in raw:
    print("db {}".format(byte))
FROM ubuntu:18.04
RUN apt-get update && apt-get install -y pmccabe gdb strace

COPY base_data /data
CMD while :; do /service/build/bryant; done

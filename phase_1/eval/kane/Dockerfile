FROM ubuntu:18.04
RUN apt-get update && apt-get install -y pmccabe gdb strace

CMD while :; do /service/build/kane; done

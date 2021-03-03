FROM ubuntu:18.04
RUN apt-get update && apt-get install -y build-essential clang clang-7 flex bison pmccabe gdb strace libstdc++-8-dev xxd
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-7 700 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-7 700 && \
    update-alternatives --install /usr/bin/clang-cpp clang-cpp /usr/bin/clang-cpp-7 700

RUN mkdir /data

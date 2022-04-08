FROM debian:buster

RUN apt-get update && \
    apt-get install -y \
        autoconf \
        automake \
        autotools-dev \
        curl \
        python3 \
        python \
        libmpc-dev \
        libmpfr-dev \
        libgmp-dev \
        gawk \
        build-essential \
        bison \
        flex \
        texinfo \
        gperf \
        libtool \
        patchutils \
        bc \
        zlib1g-dev \
        libexpat-dev \
        git && \
    apt-get clean

RUN mkdir /build
WORKDIR /build

RUN git clone --depth=1 --recursive https://github.com/riscv/riscv-gnu-toolchain

RUN cd /build/riscv-gnu-toolchain && \
    mkdir build && \
    cd build && \
    ../configure --prefix=/opt/riscv32 --with-arch=rv32im

RUN cd /build/riscv-gnu-toolchain/build && \
    make -j 16

ENV PATH="/opt/riscv32/bin:${PATH}"
ENV CC=riscv32-unknown-elf-gcc
ENV STRIP=riscv32-unknown-elf-strip
ENV AR=riscv32-unknown-elf-ar

WORKDIR /devices

FROM debian:buster

RUN apt-get update && \
    apt-get install -y \
        libfl2 \
        libmpc3 \
        make && \
    apt-get clean

COPY --from=0 /opt/riscv32 /opt/riscv32

ENV PATH="/opt/riscv32/bin:${PATH}"
ENV CC=riscv32-unknown-elf-gcc
ENV STRIP=riscv32-unknown-elf-strip
ENV AR=riscv32-unknown-elf-ar
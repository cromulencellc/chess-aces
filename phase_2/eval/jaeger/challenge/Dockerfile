FROM ubuntu:18.04 AS build
RUN apt-get update && apt-get install -y build-essential clang-7 pmccabe gdb
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-7 700 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-7 700 && \
    update-alternatives --install /usr/bin/clang-cpp clang-cpp /usr/bin/clang-cpp-7 700

ADD emulator /emulator

RUN cd /emulator && \
    make -j $(nproc)

FROM ubuntu:18.04

RUN mkdir /base_data
RUN mkdir /service

RUN echo -n TEST_TOKEN > /TOKEN

ADD base_data/acd2 /base_data/
ADD base_data/maint /base_data/
ADD base_data/rcd /base_data/
ADD base_data/fsd /base_data/

COPY --from=BUILD /emulator/challenge/challenge /service
COPY --from=BUILD /emulator/maintenance_test/maintenance_test /service

RUN chmod +x /service/*

RUN ln -s /base_data /service/devices

WORKDIR /service

ENV PORT=5000

CMD ["./challenge"]
FROM registry.mlb.cromulence.com/chess/cs_base:latest

RUN export DEBIAN_FRONTEND=noninteractive
RUN ln -fs /usr/share/zoneinfo/America/New_York /etc/localtime
RUN apt-get update
RUN apt-get install -y tzdata build-essential clang-7 gdb cloc --fix-missing
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-7 700 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-7 700 && \
    update-alternatives --install /usr/bin/clang-cpp clang-cpp /usr/bin/clang-cpp-7 700
RUN dpkg-reconfigure --frontend noninteractive tzdata

ADD ./ /home/challenge
ADD base_data /data

RUN chown -R chess:chess /home
RUN chown -R chess:chess /data
RUN mkdir /service
RUN chown -R chess:chess /service

ENV TCP_PORT_1=3004
ENV PORT=3004
ENV HOST=challenge_container
ENV CXX=clang-7
ENV CHESS=Chess_System

RUN echo -n 09vee78cnw0mjw3bLOiPPmyn15mwmlks > /token
RUN chown 600 /token

WORKDIR /home/challenge

RUN tar -xf capstone-4.0.2.tar

WORKDIR /home/challenge/capstone-4.0.2

#RUN ls

RUN ./make.sh
RUN ./make.sh install

WORKDIR /home/challenge

RUN make clean
RUN make
RUN cp -r build /service
WORKDIR /

CMD /service/build/arborway.bin

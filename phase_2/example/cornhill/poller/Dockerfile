FROM registry.mlb.cromulence.com/chess/cs_base:latest

RUN export DEBIAN_FRONTEND=noninteractive
RUN ln -fs /usr/share/zoneinfo/America/New_York /etc/localtime
RUN apt-get update
RUN apt-get install -y tzdata build-essential clang-7 gdb python3 libsasl2-dev libssl-dev --fix-missing
RUN apt-get install -y openssh-client net-tools vim
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-7 700 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-7 700 && \
    update-alternatives --install /usr/bin/clang-cpp clang-cpp /usr/bin/clang-cpp-7 700
RUN dpkg-reconfigure --frontend noninteractive tzdata

ADD ./ /home/challenge

RUN chown -R chess:chess /home

ENV TCP_PORT_1=389
ENV PORT=389
ENV HOST=challenge_server
ENV CXX=clang-7
ENV CHESS=Chess_System
ENV ISCLIENT=1
ENV TOOLPATH=/home/challenge/clients/tools/
ENV TERM=linux
ENV TERMINFO=/etc/terminfo

RUN echo -n 09vee78cnw0mjw3bLOiPPmyn15mwmlks > /token
RUN chown 600 /token

RUN pip3 install pwntools

#WORKDIR /home/challenge/clients

#RUN make clean
#RUN make depend
#RUN make

WORKDIR /home/challenge

CMD ["python3", "cornhill-poller.py"]

FROM registry.mlb.cromulence.com/chess/cs_base:latest

RUN export DEBIAN_FRONTEND=noninteractive
RUN ln -fs /usr/share/zoneinfo/America/New_York /etc/localtime
RUN apt-get update
RUN apt-get install -y tzdata build-essential clang-7 gdb python3 libsasl2-dev libssl-dev --fix-missing
RUN apt-get install -y openssh-server net-tools
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-7 700 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-7 700 && \
    update-alternatives --install /usr/bin/clang-cpp clang-cpp /usr/bin/clang-cpp-7 700
RUN dpkg-reconfigure --frontend noninteractive tzdata

ADD ./ /home/challenge

RUN echo chess:chess | chpasswd

RUN chown -R chess:chess /home

ENV PORT=9999
ENV HOST=challenge_server
ENV CXX=clang-7
ENV CHESS=Chess_System
ENV ISCLIENT=1

RUN echo -n 09vee78cnw0mjw3bLOiPPmyn15mwmlks > /token
RUN chown 600 /token

WORKDIR /home/challenge/

RUN make clean
RUN make depend
RUN make

RUN /home/challenge/clients/tools/ldapmodify -h || :
RUN /home/challenge/clients/tools/ldapsearch -h || :

RUN mkdir -p /run/sshd

CMD ["/usr/sbin/sshd", "-D"]


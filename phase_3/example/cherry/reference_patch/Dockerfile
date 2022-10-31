FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_06_02
#From ubuntu:latest

RUN apt-get -y update
RUN apt-get install -y build-essential vim libbsd-dev
RUN apt-get install -y netcat-traditional less gdb

RUN wget -q -O- https://github.com/hugsy/gef/raw/master/scripts/gef.sh | sh

RUN echo chess:chess | chpasswd

ADD ./ /home/challenge
RUN mkdir -p /srv/cvsroot
RUN cp -R /home/challenge/base_data/* /srv/cvsroot/

RUN mkdir /service
RUN chown -R chess:chess /service
RUN chown -R chess:chess /srv/cvsroot/*

ENV PORT=2401
ENV HOST=challenge_container
ENV CHESS=Chess_System

RUN echo -n 09vee78cnw0mjw3bLOiPPmyn15mwmlks > /token
RUN chmod 644 /token

WORKDIR /home/challenge/cvs-1.12.13+real
RUN ./configure
RUN make 

WORKDIR /home/challenge/cvs-1.12.13+real/src

CMD ./cvs -f --allow-root=/srv/cvsroot pserver

FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_04_22

RUN apt-get update && apt-get -y install libpcre3-dev

ADD ./ /home/challenge
ADD ./base_data /data

RUN mkdir /service
RUN chown -R chess:chess /service

RUN mkdir /etc/maradns
RUN cp /data/darpa* /etc/maradns/
RUN cp /data/mararc /etc/

ENV PORT=53
ENV HOST=challenge_container
ENV CHESS=Chess_System

RUN echo -n 09vee78cnw0mjw3bLOiPPmyn15mwmlks > /token
RUN chmod 644 /token

WORKDIR /home/challenge/MaraDNS
RUN ./configure
RUN make 
RUN python3 /data/fixip.py eth0 /etc/mararc

CMD ./server/maradns

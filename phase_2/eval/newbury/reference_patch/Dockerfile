FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_04_22

RUN apt-get update && apt-get -y install libpcre3-dev

ADD ./ /home/challenge
ADD ./base_data /data

RUN mkdir /service
RUN chown -R chess:chess /service

ENV PORT=6667
ENV HOST=challenge_container
ENV CHESS=Chess_System

RUN echo -n 09vee78cnw0mjw3bLOiPPmyn15mwmlks > /token
RUN chmod 644 /token

WORKDIR /home/challenge/lighttpd-1.4.59
RUN ./configure
RUN make 
RUN make install

CMD /home/challenge/lighttpd-1.4.59/src/lighttpd -D -f /data/lighttpd.conf

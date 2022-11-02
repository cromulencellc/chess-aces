FROM registry.mlb.cromulence.com/chess/cs_base/phase_2:2021_06_02
##chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_06_02

ADD ./ /home/challenge
ADD base_data /data
ADD /lighttpd-1.4.59/configure /home/challenge/

RUN apt-get update -y
RUN apt-get install -y libpcre3-dev
RUN apt-get install -y autoconf
RUN apt-get install -y automake
RUN apt-get install -y pkg-config

RUN mkdir /service
RUN chown -R chess:chess /service

ENV PORT=3040
ENV HOST=challenge_container
ENV CHESS=Chess_System

RUN echo -n Qfafs8Mk4Ut1D3nhfHqvrOL0mfg7LIBI > /token
RUN chmod 644 /token

WORKDIR /home/challenge/lighttpd-1.4.59
RUN chmod +x ./configure
RUN ./configure
RUN make clean 
RUN cp /home/challenge/configure .
RUN chmod +x ./configure
RUN ./configure
RUN make install

CMD /home/challenge/lighttpd-1.4.59/src/lighttpd -D -f /data/lighttpd.conf

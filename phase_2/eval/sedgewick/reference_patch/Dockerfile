FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16

ADD ./ /home/challenge
ADD base_data /data

RUN useradd -ms /bin/bash ftp
RUN echo ftp:ftp | chpasswd
RUN cp /data/* /home/ftp
RUN chown ftp:ftp /home/ftp/*

RUN cp /data/* /home/chess
RUN chown chess:chess /home/chess/*

RUN mkdir /service
RUN mkdir /usr/local/var
RUN chown -R chess:chess /service

ENV PORT=21
ENV HOST=challenge_container
ENV CHESS=Chess_System

RUN echo -n 09vee78cnw0mjw3bLOiPPmyn15mwmlks > /token
RUN chmod 644 /token

WORKDIR /home/challenge/proftpd-1.3.7a
RUN make clean
RUN ./configure
RUN make

CMD /home/challenge/proftpd-1.3.7a/proftpd -X -n -c /home/challenge/proftpd-1.3.7a/sample-configurations/basic.conf 

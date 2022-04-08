FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16

ADD ./ /home/challenge
ADD base_data /data

RUN apt-get install php -y

RUN chown -R chess:chess /home
RUN chown -R root:chess /data
RUN chmod 775 /data
RUN chown root:chess /data/*
RUN chmod 640 /data/*

RUN mkdir /service
RUN chown -R chess:chess /service

ENV PORT=9999
ENV HOST=challenge_container
ENV CHESS=Chess_System

RUN echo -n 09vee78cnw0mjw3bLOiPPmyn15mwmlks > /token
RUN chmod 644 /token

WORKDIR /home/challenge
RUN make clean
RUN make

RUN ls -al /data/

RUN ls -al /

USER chess
CMD /home/challenge/build/templeton.bin

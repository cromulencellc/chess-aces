#FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16
FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_06_02

RUN apt-get install -y libjemalloc-dev
RUN apt-get install -y xsltproc

ADD ./ /home/challenge
ADD base_data /data

RUN chown -R chess:chess /home
RUN chown -R chess:chess /data
RUN mkdir /service
RUN chown -R chess:chess /service

ENV TCP_PORT_1=1883
ENV PORT=1883
ENV HOST=challenge_container
ENV CHESS=Chess_System

RUN echo -n 09vee78cnw0mjw3bLOiPPmyn15mwmlks > /token
RUN chmod 600 /token

WORKDIR /home/challenge/mosquitto

RUN mkdir build
WORKDIR /home/challenge/mosquitto/build
RUN cmake ..
RUN make
RUN cp ./src/mosquitto /service/mosquitto.bin
RUN cp -R ./plugins/ /service/
WORKDIR /

CMD /service/mosquitto.bin -c /data/mosquitto.conf

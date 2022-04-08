FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16

RUN apt-get install -y libjemalloc-dev

ADD ./ /home/challenge
ADD base_data /data

RUN chown -R chess:chess /home
RUN chown -R chess:chess /data
RUN mkdir /service
RUN chown -R chess:chess /service

RUN echo admin:`dd if=/dev/urandom bs=512 count=1 2>/dev/null | md5sum | cut -f 1 -d " "` > /data/passwords.txt
RUN echo chess:chess >> /data/passwords.txt

ENV TCP_PORT_1=1883
ENV PORT=1883
ENV HOST=challenge_container
ENV CHESS=Chess_System

RUN echo -n 09vee78cnw0mjw3bLOiPPmyn15mwmlks > /data/token
RUN chown 600 /data/token

WORKDIR /home/challenge/mosquitto-2.0.4

RUN make clean
RUN make
RUN cp ./src/mosquitto /service/mosquitto.bin
WORKDIR /

CMD /service/mosquitto.bin -c /data/mosquitto.conf

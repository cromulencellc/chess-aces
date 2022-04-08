FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_04_14

RUN export DEBIAN_FRONTEND=noninteractive

ADD ./ /home/mosquitto/mqtt_broker

ENV PORT=1883
ENV HOST=challenge_container
ENV CHESS=Chess_System

WORKDIR /home/mosquitto/mqtt_broker

CMD ["mosquitto", "-v"]

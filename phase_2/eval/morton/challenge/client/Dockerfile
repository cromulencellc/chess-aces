FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_04_14

RUN export DEBIAN_FRONTEND=noninteractive
ADD ./ /home/mosquitto/client

ENV PORT=1883
ENV HOST=challenge_server
ENV CHESS=Chess_System
ENV ISCLIENT=1
ENV LD_LIBRARY_PATH=/home/mosquitto/client/lib
ENV PATH="/home/mosquitto/client:${PATH}"

WORKDIR /home/mosquitto/client

RUN chmod +x mosquitto_sub
CMD ["./mosquitto_sub", "-t", "app/request", "-v", "-d", "-h", "challenge_server", "-p", "1883", "--remove-retained", "-i", "Thermometer-1"]

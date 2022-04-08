FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_04_14

RUN export DEBIAN_FRONTEND=noninteractive

ADD ./ /home/mosquitto/pov

ENV PORT=1883
ENV HOST=challenge_server
ENV CHESS=Chess_System
ENV ISCLIENT=1
ENV LD_LIBRARY_PATH=/home/mosquitto/pov/lib
ENV PATH="/home/mosquitto/pov:${PATH}"
ENV TERM=linux
ENV TERMINFO=/etc/terminfo

WORKDIR /home/mosquitto/pov/pov_src

RUN chmod +x pov

WORKDIR /home/mosquitto/pov

CMD ["./pov_src/pov", "-h", "challenge_server", "-p", "1883"]

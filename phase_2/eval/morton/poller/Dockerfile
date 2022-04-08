FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_04_14

RUN export DEBIAN_FRONTEND=noninteractive

ADD ./ /home/mosquitto/poller

ENV PORT=1883
ENV HOST=challenge_server
ENV CHESS=Chess_System
ENV ISCLIENT=1
ENV LD_LIBRARY_PATH=/home/mosquitto/poller/lib
ENV PATH="/home/mosquitto/poller:${PATH}"
ENV TERM=linux
ENV TERMINFO=/etc/terminfo

WORKDIR /home/mosquitto/poller/poller_src

RUN chmod +x poller

CMD ["./poller", "-h", "challenge_server", "-p", "1883"]

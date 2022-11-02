FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16

COPY . /poller
WORKDIR /poller

ENV HOST=challenge_container
ENV PORT=1883
ENV TERM=linux
ENV TERMINFO=/etc/terminfo

RUN pip3 install paho-mqtt-1.5.1.tar.gz

CMD ["python3", "foster-poller.py"]

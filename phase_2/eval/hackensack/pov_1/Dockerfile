#FROM python:3
FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16

ENV TERM=linux
ENV TERMINFO=/etc/terminfo

RUN mkdir /hackensack-pov-1
COPY . /hackensack-pov-1
WORKDIR /hackensack-pov-1

RUN pip3 install paho-mqtt-1.5.1.tar.gz

CMD ["python3", "./hackensack.pov.1.py"]

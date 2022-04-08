#FROM python:3
FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16

RUN apt-get update

ENV TERM=linux
ENV TERMINFO=/etc/terminfo
ENV TOOLS=/usr/bin/

RUN mkdir /templeton-pov-1
COPY . /templeton-pov-1
WORKDIR /templeton-pov-1

CMD ["python3", "./templeton.pov.1.py"]

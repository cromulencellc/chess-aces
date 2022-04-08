#FROM python:3
FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16

RUN apt-get update

ENV TERM=linux
ENV TERMINFO=/etc/terminfo
ENV TOOLS=/usr/bin/

RUN mkdir /templeton-pov-2
COPY . /templeton-pov-2
WORKDIR /templeton-pov-2

CMD ["python3", "./templeton.pov.2.py"]

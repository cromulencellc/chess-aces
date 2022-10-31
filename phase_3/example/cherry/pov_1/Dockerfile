#FROM python:3
#FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16
#FROM ubuntu:latest
FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_06_02

RUN apt-get update
RUN apt-get install -y build-essential vim libbsd-dev
RUN apt-get install -y python3 python3-pip

RUN pip3 install pwntools

ENV TERM=linux
ENV TERMINFO=/etc/terminfo
ENV TOOLS=/usr/bin/
ENV PORT=2401
ENV HOST=challenge_container
ENV CVSBIN=/cherry-pov-1/cvs-1.12.13+real/src/cvs

RUN mkdir /cherry-pov-1
COPY . /cherry-pov-1

WORKDIR /cherry-pov-1

CMD ["python3", "./cherry.pov.1.py"]

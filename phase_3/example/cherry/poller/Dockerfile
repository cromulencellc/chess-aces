#FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16
FROM ubuntu:latest

RUN apt-get -y update
RUN apt-get install -y build-essential python3

COPY . /poller
WORKDIR /poller

ENV HOST=challenge_container
ENV PORT=2401
ENV TERM=linux
ENV TERMINFO=/etc/terminfo

CMD ["python3", "cherry.poller.py"]

#FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16
FROM ubuntu:latest

RUN apt-get update
RUN apt-get install -y python3 python3-pip
RUN pip3 install requests

COPY . /poller
WORKDIR /poller

ENV HOST=challenge_container
ENV PORT=8000
ENV TERM=linux
ENV TERMINFO=/etc/terminfo

CMD ["python3", "justison.poller.py"]

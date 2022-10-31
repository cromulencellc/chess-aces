#FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16
FROM ubuntu:latest

RUN apt-get update
RUN apt-get install -y python3
COPY . /poller
WORKDIR /poller

ENV HOST=challenge_container
ENV PORT=6667
ENV TERM=linux
ENV TERMINFO=/etc/terminfo
ENV SEED=1269265341779030219

CMD ["python3", "inagua.poller.py"]

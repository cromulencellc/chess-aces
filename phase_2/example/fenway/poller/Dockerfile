#FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16
FROM registry.mlb.cromulence.com/chess/cs_base:latest

COPY . /poller
WORKDIR /poller

ENV HOST=challenge_container
ENV PORT=3445
ENV TERM=linux
ENV TERMINFO=/etc/terminfo
ENV SEED=2953136187624

CMD ["python3", "fenway-poller.py"]

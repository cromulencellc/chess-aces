##FROM registry.mlb.cromulence.com/chess/cs_base/phase_2:2021_06_02
FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_06_02

ENV TERM=linux
ENV TERMINFO=/etc/terminfo
ENV TOOLS=/usr/bin/

RUN mkdir /darwin-pov-3
COPY . /darwin-pov-3
WORKDIR /darwin-pov-3

CMD ["python3", "./darwin.pov.3.py"]

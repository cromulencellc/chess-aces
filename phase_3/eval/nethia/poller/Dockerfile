FROM registry.mlb.cromulence.com/chess/cs_base/phase_2:2021_06_02
#FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_06_02

RUN mkdir /nethia-poller
COPY . /nethia-poller
WORKDIR /nethia-poller
CMD ["python3", "./nethia.poller.py"]

FROM registry.mlb.cromulence.com/chess/cs_base/phase_2:2021_06_02
#FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_06_02

RUN mkdir /nethia-pov-1
COPY . /nethia-pov-1
WORKDIR /nethia-pov-1
CMD ["python3", "./nethia.pov.1.py"]

FROM registry.mlb.cromulence.com/chess/cs_base/phase_2:2021_06_02
#FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_06_02

RUN mkdir /nethia-pov-2
COPY . /nethia-pov-2
WORKDIR /nethia-pov-2
CMD ["python3", "./nethia.pov.2.py"]

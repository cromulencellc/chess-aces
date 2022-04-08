#FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16
FROM registry.mlb.cromulence.com/chess/cs_base:latest

RUN mkdir /fenway-pov
COPY . /fenway-pov
WORKDIR /fenway-pov
CMD ["python3",  "fenway-pov.py"]

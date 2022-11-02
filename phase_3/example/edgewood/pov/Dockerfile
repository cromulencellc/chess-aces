        # FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_3:2021_12-27
FROM registry.mlb.cromulence.com/chess/cs_base/phase_3:2021_12_27

RUN mkdir -p /pov
COPY . /pov
WORKDIR /pov

CMD ["python3", "pov.py"]

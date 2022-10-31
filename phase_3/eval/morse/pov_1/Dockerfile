FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_06_02

ENV TERM=linux
ENV TERMINFO=/etc/terminfo
ENV TOOLS=/usr/bin/

RUN mkdir -p /pov
COPY . /pov
WORKDIR /pov

CMD ["python3", "pov_1.py"]

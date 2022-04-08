#FROM python:3
FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16

ENV TERM=linux
ENV TERMINFO=/etc/terminfo
ENV TOOLS=/usr/bin/

RUN ifconfig 
RUN mkdir /pitsmoor-pov-2
COPY . /pitsmoor-pov-2
WORKDIR /pitsmoor-pov-2

CMD ["python3", "./pitsmoor.pov.2.py"]

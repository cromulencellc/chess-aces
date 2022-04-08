#FROM python:3
FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16

ENV TERM=linux
ENV TERMINFO=/etc/terminfo
ENV TOOLS=/usr/bin/

RUN ifconfig 
RUN mkdir /newbury-pov-1
COPY . /newbury-pov-1
WORKDIR /newbury-pov-1

CMD ["python3", "./newbury.pov.1.py"]

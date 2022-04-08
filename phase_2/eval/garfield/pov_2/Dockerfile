#FROM python:3
FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16

ENV TERM=linux
ENV TERMINFO=/etc/terminfo

RUN mkdir /garfield-pov-2
COPY . /garfield-pov-2
WORKDIR /garfield-pov-2
CMD ["python3", "./garfield.pov.2.py"]

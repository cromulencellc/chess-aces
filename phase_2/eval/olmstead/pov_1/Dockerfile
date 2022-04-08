#FROM python:3
FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16

RUN apt-get update
RUN apt-get install -y ldap-utils

ENV TERM=linux
ENV TERMINFO=/etc/terminfo
ENV TOOLS=/usr/bin/

RUN mkdir /olmstead-pov-1
COPY . /olmstead-pov-1
WORKDIR /olmstead-pov-1

CMD ["python3", "./olmstead.pov.1.py"]

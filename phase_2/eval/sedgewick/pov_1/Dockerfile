#FROM python:3
FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16

RUN apt-get update
RUN pip3 install netifaces

ENV TERM=linux
ENV TERMINFO=/etc/terminfo
ENV TOOLS=/usr/bin/

RUN ifconfig 
RUN mkdir /sedgewick-pov-1
COPY . /sedgewick-pov-1
WORKDIR /sedgewick-pov-1

CMD ["python3", "./sedgewick.pov.1.py"]

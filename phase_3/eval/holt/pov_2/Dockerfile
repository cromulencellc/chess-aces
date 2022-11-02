#FROM python:3
FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16

ENV TERM=linux
ENV TERMINFO=/etc/terminfo
ENV TOOLS=/usr/bin/

RUN ifconfig 
RUN mkdir /holt-pov-2
COPY . /holt-pov-2
WORKDIR /holt-pov-2
RUN chmod +x ./holt.pov.2.sh

CMD ["./holt.pov.2.sh"]

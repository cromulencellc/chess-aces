#FROM python:3
FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16

ENV TERM=linux
ENV TERMINFO=/etc/terminfo
ENV TOOLS=/usr/bin/
ENV PORT=6667
ENV HOST=challenge_container

RUN ifconfig 
RUN mkdir /inagua-pov-1
COPY . /inagua-pov-1
WORKDIR /inagua-pov-1

CMD ["python3", "./inagua.pov.1.py"]

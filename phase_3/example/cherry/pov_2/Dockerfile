FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_06_02

RUN apt-get update
RUN apt-get install -y build-essential vim libbsd-dev
RUN apt-get install -y python3 python3-pip

RUN pip3 install pwntools

ENV TERM=linux
ENV TERMINFO=/etc/terminfo
ENV TOOLS=/usr/bin/
ENV PORT=2401
ENV HOST=challenge_container

RUN mkdir /cherry-pov-2
COPY . /cherry-pov-2

WORKDIR /cherry-pov-2

CMD ["python3", "./cherry.pov.2.py"]

FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_06_02

ENV TERM=linux
ENV TERMINFO=/etc/terminfo
ENV TOOLS=/usr/bin/

RUN groupadd --gid 1001 node \
  && useradd --uid 1001 --gid node --shell /bin/bash --create-home node

RUN mkdir -p /poller
COPY . /poller
WORKDIR /poller

RUN python3 get-pip.py
RUN chown -R node:node /poller
USER node

RUN pip install beautifulsoup4
RUN pip install python-socketio
RUN pip install flask

CMD ["python3", "poller.py"]

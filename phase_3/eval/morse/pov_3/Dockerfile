FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_06_02

ENV TERM=linux
ENV TERMINFO=/etc/terminfo
ENV TOOLS=/usr/bin/

RUN apt-get update 
RUN apt-get install -y nodejs yarn
RUN apt-get install -y wget gnupg ca-certificates procps libxss1 \
  && wget -q -O - https://dl-ssl.google.com/linux/linux_signing_key.pub | apt-key add - \
  && sh -c 'echo "deb [arch=amd64] http://dl.google.com/linux/chrome/deb/ stable main" >> /etc/apt/sources.list.d/google.list' \
  && apt-get update \
  && apt-get install -y google-chrome-stable \
  && rm -rf /var/lib/apt/lists/* \
  && wget --quiet https://raw.githubusercontent.com/vishnubob/wait-for-it/master/wait-for-it.sh -O /usr/sbin/wait-for-it.sh \
  && chmod +x /usr/sbin/wait-for-it.sh

RUN groupadd --gid 1001 node \
  && useradd --uid 1001 --gid node --shell /bin/bash --create-home node

RUN mkdir -p /node/pov && \
    chown -R node:node /node/pov
COPY . /node/pov
WORKDIR /node/pov
RUN npm install
RUN python3 get-pip.py

USER node

RUN pip install beautifulsoup4
RUN pip install python-socketio
RUN pip install flask

EXPOSE 8888

CMD ["python3", "pov_3.py"]
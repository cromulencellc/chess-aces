FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_06_02
#FROM node:lts

USER root

RUN groupadd --gid 1001 node \
  && useradd --uid 1001 --gid node --shell /bin/bash --create-home node

RUN apt-get update -y \
    && apt-get -y install build-essential git \
    && apt-get install -y curl \
    && curl -fsSL https://deb.nodesource.com/setup_16.x | bash - \
    && apt-get install -y nodejs 

RUN npm install -g npm@8.5.5

ENV PORT=4567
ENV HOST=challenge_container
ENV CHESS=Chess_System
ENV NODE_OPTIONS="--max-old-space-size=1024"
RUN echo -n BVVNAJpEsazxvkDpueBR9fkwBShMzpE9 > /token
RUN chmod 644 /token

RUN mkdir -p /usr/src/app && \
    chown -R node:node /usr/src/app
WORKDIR /usr/src/app

ARG NODE_ENV
ENV NODE_ENV $NODE_ENV

COPY --chown=node:node install/package.json /usr/src/app/package.json

USER node

RUN npm install --verbose --only=prod && \
    npm cache clean --force

COPY --chown=node:node . /usr/src/app

ENV NODE_ENV=production \
    daemon=false \
    silent=false

EXPOSE 4567

#CMD bash
CMD ./nodebb setup; ./nodebb start
#CMD test -n "${SETUP}" && ./nodebb setup || node ./nodebb build; node ./nodebb start

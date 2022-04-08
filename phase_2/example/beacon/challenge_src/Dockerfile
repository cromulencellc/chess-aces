FROM chessinternal.apogee-research.com:5005/apogee/testbed_environment/cs_base_phase_2:2021_03_10

RUN mkdir /data

ADD ./ /mnt/challenge
ADD base_data /data

ENV TCP_PORT_1=3028
ENV PORT=3028
ENV HOST=challenge_container
ENV CHESS=Chess_System

RUN echo -n 09vee78cnw0mjw3bLOiPPmyn15mwmlks > /token
RUN chown 600 /token

WORKDIR /mnt/challenge

CMD ["node", "src/main.js"]

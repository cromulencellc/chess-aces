#FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_06_02
FROM registry.mlb.cromulence.com/chess/cs_base/phase_3:2022_03_10

RUN apt install -y libpcre2-dev
RUN apt install -y libsodium-dev
RUN apt install -y libc-ares-dev
RUN apt install -y libargon2-dev
RUN apt install -y libjansson-dev
RUN apt install -y libgeoip-dev

RUN echo -n 09vee78cnw0mjw3bLOiPPmyn15mwmlks > /token
RUN chmod 600 /token


ADD ./ /home/challenge
ADD ./base_data/ /home/challenge/base_data

WORKDIR /home/challenge/unrealircd-6.0.2

RUN ./Config
RUN make
RUN make install

WORKDIR /root
RUN cp /home/challenge/base_data/modules.sources.list ./unrealircd/conf
RUN cp /home/challenge/base_data/modules.custom.conf ./unrealircd/conf
RUN cp /home/challenge/base_data/unrealircd.conf ./unrealircd/conf
RUN cp -r /home/challenge/base_data/tls ./unrealircd/conf

USER root
EXPOSE 6667 6697

WORKDIR /root/unrealircd
CMD ["./bin/unrealircd", "-F"]
#CMD ./unrealircd start; sleep 10000

FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16

RUN apt-get install -y automake autoconf pkgconf
ADD ./ /home/challenge
ADD base_data /data

RUN mkdir /service
RUN chown -R chess:chess /service

ENV PORT=6667
ENV HOST=challenge_container
ENV CHESS=Chess_System

RUN echo -n 09vee78cnw0mjw3bLOiPPmyn15mwmlks > /token
RUN chmod 644 /token

WORKDIR /home/challenge/ngircd
RUN ./autogen.sh
RUN ./configure
RUN make

CMD /home/challenge/ngircd/src/ngircd/ngircd -n -f ./doc/sample-ngircd.conf

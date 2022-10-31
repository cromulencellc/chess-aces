FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_04_22

ADD ./ /home/challenge

RUN mkdir /service
RUN chown -R chess:chess /service

ENV PORT=6667
ENV HOST=challenge_container
ENV CHESS=Chess_System

RUN echo -n 09vee78cnw0mjw3bLOiPPmyn15mwmlks > /token
RUN chmod 644 /token

WORKDIR /home/challenge/ngircd-26.1
RUN ./configure
RUN make 

CMD /home/challenge/ngircd-26.1/src/ngircd/ngircd -n -f /home/challenge/ngircd-26.1/doc/sample-ngircd.conf


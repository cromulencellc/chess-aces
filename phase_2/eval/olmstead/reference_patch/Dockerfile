FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16

ADD ./ /home/challenge
ADD base_data /data

RUN chown -R chess:chess /home
RUN chown -R chess:chess /data
RUN mkdir /service
RUN chown -R chess:chess /service

ENV TCP_PORT_1=389
ENV PORT=389
ENV HOST=challenge_container
ENV CHESS=Chess_System

RUN echo -n 09vee78cnw0mjw3bLOiPPmyn15mwmlks > /token
RUN chown 600 /token

WORKDIR /home/challenge/openldap-2.4.56

RUN make clean
RUN make depend
RUN make
WORKDIR /home/challenge/openldap-2.4.56/servers/slapd

RUN python3 /data/set_rootdn_pass.py /data/slapd.conf
RUN cat /data/slapd.conf | grep rootpw

CMD ./slapd -f /data/slapd.conf -d 65535

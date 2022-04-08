FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16

ADD ./ /home/challenge

RUN chown -R chess:chess /home
RUN apt-get update
RUN apt-get install -y ldap-utils

ENV TCP_PORT_1=389
ENV PORT=389
ENV HOST=challenge_container
ENV CHESS=Chess_System
ENV TERM=linux
ENV TERMINFO=/etc/terminfo
ENV SEED=6688039122457923251

WORKDIR /home/challenge
#RUN tar -xvf openldap-2.4.56.tar

#WORKDIR /home/challenge/openldap-2.4.56

#RUN ./configure --enable-slapd=no

#RUN make depend
#RUN make

#ENV TOOLS=/home/challenge/openldap-2.4.56/clients/tools/

WORKDIR /home/challenge

CMD python3 ./olmstead-poller.py

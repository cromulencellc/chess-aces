FROM registry.mlb.cromulence.com/chess/cs_base/phase_2:2021_06_02
##chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_06_02

ADD ./ /home/challenge
ADD base_data /srv/ftp/
RUN chmod -R 755 /srv/ftp/
RUN chown -R 791:791 /srv/ftp/
RUN chown -R 792:792 /srv/ftp/chess

RUN apt-get update -y
RUN apt-get install proftpd -y

ENV PORT=21
ENV HOST=challenge_container
ENV CHESS=Chess_System

RUN echo -n 09vee78cnw0mjw3bLOiPPmyn15mwmlks > /token
RUN chmod 644 /token

WORKDIR /home/challenge/proftpd-master
RUN chmod +x ./configure
RUN chmod +x /home/challenge/proftpd-master/modules/glue.sh
RUN ./configure --with-modules=mod_ident --enable-dso --enable-ctrls --enable-nls
RUN make clean 
RUN make
RUN make install
WORKDIR /home/challenge
RUN cp -r ./proftpd /etc
WORKDIR /etc/proftpd
RUN chmod 660 ftpd.passwd
RUN chmod 660 ftpd.group

CMD /home/challenge/proftpd-master/proftpd -n -d[10] -c /etc/proftpd/proftpd.conf

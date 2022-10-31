FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_06_02
#FROM registry.mlb.cromulence.com/chess/cs_base/phase_2:2021_06_02

RUN apt-get update -y
RUN apt-get install -y libpcre3-dev
RUN apt-get install -y build-essential
RUN apt-get install -y libapr1-dev 
RUN apt-get install -y libaprutil1-dev
RUN apt-get install -y autoconf
RUN apt-get install -y automake
RUN apt-get install -y pkg-config
RUN apt-get install -y python3.8
RUN apt-get install -y python3.8-dev

ENV PORT=3040
ENV HOST=challenge_container
ENV CHESS=Chess_System

RUN echo -n Qfafs8Mk4Ut1D3nhfHqvrOL0mfg7LIBI > /token
RUN chmod 644 /token

ADD ./ /home/challenge
ADD base_data /data

WORKDIR /home/challenge/httpd-2.4.51
RUN ./configure --prefix=/usr/local/apache2 --with-crypto --with-included-apr --with-mpm=prefork
RUN make clean
RUN make install
RUN cp -r /home/challenge/conf/* /usr/local/apache2/conf/
RUN cp -r /home/challenge/conf/.passwd /usr/local/apache2/conf/

WORKDIR /home/challenge/httpd-2.4.51/mod_python
RUN ./configure --with-python=/usr/bin/python3.8 --with-apxs=/usr/local/apache2/bin/apxs
RUN make
RUN make install
RUN cp src/.libs/mod_python.so /usr/local/apache2/modules

WORKDIR /usr/local/apache2

CMD /usr/local/apache2/bin/apachectl -k start -DFOREGROUND 

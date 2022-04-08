FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16

ADD ./ /home/challenge
ADD base_data /data

RUN chown -R chess:chess /home
RUN apt-get update

RUN apt-get install php -y

ENV PORT=9999
ENV HOST=challenge_container
ENV CHESS=Chess_System
ENV TERM=linux
ENV TERMINFO=/etc/terminfo

WORKDIR /home/challenge

CMD python3 ./templeton-poller.py

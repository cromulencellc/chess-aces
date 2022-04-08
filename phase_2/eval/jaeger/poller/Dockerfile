FROM debian:buster

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y python3 && \
    apt-get clean

RUN mkdir /poller

ADD jaeger_lib /poller/jaeger_lib
ADD poller.py /poller/

WORKDIR /poller

CMD ["python3", "poller.py"]
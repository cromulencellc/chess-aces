FROM debian:buster

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y python3 && \
    apt-get clean

RUN mkdir /pov

ADD jaeger_lib /pov/jaeger_lib
ADD pov.py /pov/

WORKDIR /pov

CMD ["python3", "pov.py"]
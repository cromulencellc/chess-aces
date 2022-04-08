FROM debian:latest

RUN apt-get update && apt-get install -y python3 && apt-get clean

COPY . /poller
WORKDIR /poller

ENV HOST=arborway
ENV PORT=3004

CMD ["python3", "arborway-poller.py"]

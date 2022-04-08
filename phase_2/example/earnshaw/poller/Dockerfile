FROM debian:latest

RUN apt-get update && apt-get install -y python3 && apt-get clean

COPY . /poller
WORKDIR /poller

ENV HOST=challenge_container
ENV PORT=21

CMD ["python3", "earnshaw-poller.py"]

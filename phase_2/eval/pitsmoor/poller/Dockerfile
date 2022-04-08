FROM debian:latest

RUN apt-get update && apt-get install -y python3 && apt-get clean

COPY . /poller
WORKDIR /poller

ENV HOST=challenge_container
ENV PORT=6667

CMD ["python3", "pitsmoor.poller.py"]

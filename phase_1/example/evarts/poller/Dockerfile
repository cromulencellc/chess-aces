FROM python:2
RUN apt update && apt install -y git libssl-dev libffi-dev build-essential && pip install --upgrade pip && pip install --upgrade pwntools
RUN mkdir /evarts-poller
COPY . /evarts-poller
WORKDIR /evarts-poller
CMD ["./evarts-poller.py"]

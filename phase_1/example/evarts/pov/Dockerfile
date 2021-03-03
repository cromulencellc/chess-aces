FROM python:2
RUN apt update && apt install -y git libssl-dev libffi-dev build-essential && pip install --upgrade pip && pip install --upgrade pwntools
RUN mkdir /evarts-pov
COPY . /evarts-pov
WORKDIR /evarts-pov
CMD ["./evarts-pov.py"]

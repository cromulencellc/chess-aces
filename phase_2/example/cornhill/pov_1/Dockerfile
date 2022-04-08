FROM python:3
RUN mkdir /cornhill-pov-1
COPY . /cornhill-pov-1
WORKDIR /cornhill-pov-1

ENV PORT=9999
RUN export DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install -y build-essential net-tools --fix-missing

CMD ["python3", "./cornhill.pov1.py"]

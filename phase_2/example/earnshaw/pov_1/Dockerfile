FROM python:3

RUN apt-get update
RUN apt-get install -y python3-pip

RUN pip install pwntools
ENV TERM=linux
ENV TERMINFO=/etc/terminfo

RUN mkdir /earnshaw-pov-1
COPY . /earnshaw-pov-1
WORKDIR /earnshaw-pov-1
CMD ["python3", "./earnshaw.pov.1.py"]

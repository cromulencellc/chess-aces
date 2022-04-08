FROM python:3

RUN apt-get update
RUN apt-get install -y python3-pip

RUN pip install pwntools
ENV TERM=linux
ENV TERMINFO=/etc/terminfo

RUN mkdir /earnshaw-pov-3
COPY . /earnshaw-pov-3
WORKDIR /earnshaw-pov-3
CMD ["python3", "./earnshaw.pov.3.py"]

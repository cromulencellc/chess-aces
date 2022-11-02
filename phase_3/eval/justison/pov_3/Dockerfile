FROM python:3
RUN mkdir /justison-pov-3
RUN pip3 install requests
RUN pip3 install pwntools
COPY . /justison-pov-3
WORKDIR /justison-pov-3
CMD ["python3", "./justison.pov.3.py"]

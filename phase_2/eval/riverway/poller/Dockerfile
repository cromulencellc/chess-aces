FROM python:3


RUN pip install --no-cache-dir --upgrade pip && pip install --no-cache-dir websocket-client

COPY poller.py /

CMD ["python3", "./poller.py"]

FROM node:16-alpine3.15
RUN mkdir /poller
COPY . /poller
WORKDIR /poller
RUN npm i
CMD ["npx ./poller.js"]

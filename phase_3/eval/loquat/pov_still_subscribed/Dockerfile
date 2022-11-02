FROM node:16-alpine3.15

RUN mkdir /pov
COPY . /pov
WORKDIR /pov

RUN npm i
RUN npx tsc
CMD ["node ./build/main.js"]

FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2020_11_16                       
WORKDIR /app

COPY src/*.json ./

RUN npm install

RUN mkdir -p Logs

RUN echo 12345678901234567890123456789012 > /token

COPY src/ .

CMD ["node", "app.js"]


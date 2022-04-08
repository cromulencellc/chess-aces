FROM chessinternal.apogee-research.com:5005/apogee/testbed_environment/cs_base_playwright:2020_11_23

WORKDIR /poller

COPY base_data /data
COPY . /poller
CMD ["node", "poller.js"]

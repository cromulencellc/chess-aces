FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_06_02

RUN mkdir /data

RUN mkdir challenge

COPY ./wt-4.5.0 challenge/

RUN mkdir build

WORKDIR build

RUN cmake -DMULTI_THREADED=false ../

RUN make

RUN make -C examples

WORKDIR ../examples/hangman

CMD ../../build/examples/hangman/hangman.wt --docroot /static/docroot --http-listen 0:3025

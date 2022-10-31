FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_06_02

RUN apt-get update -y
RUN apt-get install -y cmake
RUN apt-get install -y build-essential
RUN apt-get install -y bison flex
RUN apt-get install -y libboost-all-dev
RUN apt-get install -y rapidjson-dev 
RUN apt-get install -y openssl
RUN apt-get install -y libssl-dev

ENV PORT=3040
ENV HOST=challenge_container
ENV CHESS=Chess_System

RUN echo -n LbXTH3aWmPTXcgjQsHjBKsSgTyy45LpG > /token
RUN chmod 644 /token

ADD ./ /home/challenge
ADD base_data /data

WORKDIR /home/challenge/DikuMUD3

RUN cmake .
RUN make -j8

WORKDIR /home/challenge/DikuMUD3/vme/bin
CMD ./vme & ./mplex -p 3040

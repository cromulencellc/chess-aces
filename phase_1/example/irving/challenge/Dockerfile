FROM ubuntu:18.04
RUN apt-get update && apt-get install -y build-essential clang-7 pmccabe gdb --fix-missing
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-7 700 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-7 700 && \
    update-alternatives --install /usr/bin/clang-cpp clang-cpp /usr/bin/clang-cpp-7 700
RUN mkdir -p /mnt
WORKDIR /mnt
RUN useradd -ms /bin/bash chess
RUN rm -rf /home/chess/*
RUN /bin/echo -en "helloworld\n" > /home/chess/data
RUN /bin/echo -en "chess\nstuff\n" > /home/chess/data2
RUN chown chess:chess /home/chess/data
RUN chown chess:chess /home/chess/data2
RUN echo -en "931606baaa7a2b4ef61198406f8fc3f4" > /token
EXPOSE 3004

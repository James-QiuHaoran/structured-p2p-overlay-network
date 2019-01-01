FROM ubuntu:18.04

COPY ./ /hgfreval/

WORKDIR /hgfreval/test

RUN apt-get update
RUN apt-get install -y libboost-log-dev libboost-random-dev libprotobuf-dev libprotobuf10 protobuf-compiler

EXPOSE 2000

CMD mkdir ../test/log && ../bin/eval_server ../test/log 2000
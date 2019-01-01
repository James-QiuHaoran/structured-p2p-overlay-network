
FROM ubuntu:18.04

COPY ./ /hgfreval/

WORKDIR /hgfreval/

RUN apt-get update
RUN apt-get install -y build-essential cmake libboost-log-dev libboost-random-dev libprotobuf-dev libprotobuf10 protobuf-compiler
RUN ./build.sh

EXPOSE 20202 30303
ENV bootstrap_ip "10.0.0.1"

WORKDIR /hgfreval/test
CMD mkdir log && ../bin/eval_client 20202 30303 bootstrap_ip 2000
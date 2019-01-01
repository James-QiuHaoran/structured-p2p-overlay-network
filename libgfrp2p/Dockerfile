
FROM ubuntu:18.04

COPY ./ /hgfreval/

WORKDIR /hgfreval/

RUN apt-get update && apt-get install -y build-essential cmake libprotobuf-dev libprotobuf10 protobuf-compiler
RUN mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug && make && cd ..

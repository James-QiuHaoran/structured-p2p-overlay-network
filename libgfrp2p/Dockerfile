
FROM jitaodocker/hgfr_ubuntu

COPY ./ /hgfreval/

WORKDIR /hgfreval/

RUN mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug && make && cd ..

# Hidden Geographical Fractal Random Ring (HGFRR) P2P Network

## TODO List

- Insert logging for broadcasting, sending, receiving messages for ethereum;
- Write script to handle multiple broadcast;
- Bootstrap nodes implementation on docker (config)
	- distribute IDs and construct the network (only for evaluation: use a central server)

---

## Dependencies (in APT Names)
- Boost
	- libboost-log-dev
	- libboost-random-dev
- Protobuf
	- libprotobuf-dev
	- libprotobuf
	- protobuf-compiler

## Design of Wire Protocol
Protobuf schema to be designed

## Design of Bootstrap Packet Format
See `evaluation/proto/bootstrat_message.proto`
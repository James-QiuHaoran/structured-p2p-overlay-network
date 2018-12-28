# Hidden Geographical Fractal Random Ring (HGFRR) P2P Network

## TODO List

- Bootstrap nodes implementation on docker (config);
- Refactor Kad+Gossip;
	- `push` version [Done]
	- `pull` version [TODO]
- Test Kad+Gossip against Eth;
- Evaluation;

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
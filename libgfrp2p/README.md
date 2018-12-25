# Hidden Geographical Fractal Random Ring (HGFRR) P2P Network

## TODO List

- Benchmark/block-bench (catch workload/network flow) [end-to-end system]
- Deal with multiple broadcast
- Bootstrap nodes implementation on docker (config)
	- distribute IDs and construct the network (only for evaluation: use a central server)
- Implement Kademlia, Random Graph
	- form structure
	- broadcast (gossip)
		- Push & TTL
		- Push & UID
		- Pull-version
	- insert traffic control simulation
	- insert time recording, number of messages recording functions (data retrieval)
- Code refactoring

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
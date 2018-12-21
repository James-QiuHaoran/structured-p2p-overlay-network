# Hidden Geographical Fractal Random Ring (HGFRR) P2P Network

## TODO List

- Debug transport
- Implement script to retrieve time of send/recv, number of messages for one broadcast
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

## Dependencies
- libboost-system-dev
- libboost-log-dev

## Design of Wire Protocol
```
8-byte sender-id || 1-byte command || command-specific field
```
- id: 64-bit randomly generated unsigned integer
- command: 8-bit unsigned integer
    |cmd_id|cmd_name|cmd_field|
    |---|---|---|
    |0x01|ping|n/a|
    |0x02|pong|n/a|
    TODO: add more
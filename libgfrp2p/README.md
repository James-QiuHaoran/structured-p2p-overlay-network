# Hidden Geographical Fractal Random Ring (HGFRR) P2P Network

## TODO List

- Implement time recording, number of messages recording functions (data retrieval)
- Bootstrap nodes implementation
	- docker configurations
	- distribute IDs and construct the network (only for evaluation: use a central server)
- Chord, Pastry, Tapestry Implementation
	- form structure
	- broadcast (gossip)
	- insert time recording, number of messages recording functions (data retrieval)

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
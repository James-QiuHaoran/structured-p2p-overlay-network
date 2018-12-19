# Hidden Geographical Fractal Random Ring (HGFRR) P2P Network

## TODO List

- implement time recording, number of messages recording functions
- Boostrap Server Implementation
	- docker
	- distribute IDs and construct the network
- Chord, Pastry, Tapestry Implementation
	- insert time recording, number of messages recording functions

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
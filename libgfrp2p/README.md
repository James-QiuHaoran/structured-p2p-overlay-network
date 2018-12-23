# Hidden Geographical Fractal Random Ring (HGFRR) P2P Network

## TODO List

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

`grep "Received data: Hello World" output | cut -d' ' -f7 | sort | uniq | wc -l`

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

## Design of Bootstrap Packet Format
- Schema: ``` 1-byte command id in uint8 || command-specific field ```

- id: Even number for node -> bootstrap; odd number for bootstrap -> node

	|command id| command name| comamnd-specific field | description |
	| --- | --- | --- | --- |
	| 0x00 | init | 2-byte: bootstrap port in uint16 | node registers itself at bootstrap |
	| 0x01 | config | 4-byte: run id in uint32 <br> <br> 4-byte: id in bit string <br> <br> 4-byte:  num_nodes_in_dist in uint32 <br> 4-byte: num_cnodes_in_dist in uint32 <br> 4-byte: num_nodes_in_city in uint32 <br> 4-byte: num_cnodes_in_city in uint32 <br> 4-byte: num_nodes_in_state in uint32 <br> 4-byte: num_cnodes_in_state in uint32 <br> 4-byte: num_nodes_in_country in uint32  <br> 4-byte: num_cnodes_in_country in uint32  <br> 4-byte: num_nodes_in_continent in uint32 <br> <br> 4-byte: # node table entries <br> in uint32 n x 8 bytes: node id \| node ip |  bootstrap tells the node the structure of the network and the node->ip mapping|
	| 0x02 | config ack | 1-byte: status {0x00: ok, 0x01: failed} | node tells bootstrap whether the config is fine |
	| 0x03 | broadcast | 4-byte: workload in bytes | bootstrap asks a node to generate given workload and broadcast |
	| 0x05 | pull log | n/a | bootstrap asks a node to upload its log |
	| 0x06 | push log | variable length: log in string | node uploads the log | 
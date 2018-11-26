#include <string>
#include <unordered_set>

#include "node.h"

// UDP packet format
class Packet {
private:
    // Can be extended with hash, signature, etc.
    std::string data;

public:
    // Costructor
    Packet(std::string data);

    // Assemble to UDP datagram
    virtual std::string pack() const;
    // Dissemble from UDP datagram
    virtual void unpack(std::string datagram);
};

/* Wire protocol for discovery only
 * Define for the need of node network maintenance */
class WireProtocol {
private:
    // Encapsulating low-level stuff
    void send(const Node& node, std::string data);

public:
    virtual void ping(const Node& node);
    virtual void pong(const Node& node);
    // TODO: summarize and add more

};


// Node table and maintenance
class NodeTable {
private:
    std::unordered_set<Node> nodes;

public:
    // Get the nodes to peer up with
    std::unordered_set<Node> get_neighbors();

};
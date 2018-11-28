#include <string>
#include <unordered_set>
#include <memory>

#include "node.h"
#include "transport.h"

// UDP packet format
class Packet {
private:
    // Can be extended with hash, signature, etc.
    std::string data;

public:
    // Assemble to UDP datagram
    virtual std::string pack() const;
    // Dissemble from UDP datagram
    virtual void unpack(std::string datagram);

    // Packet design-specific functions
    std::string get_data() const;
};

/* Wire protocol for discovery only
 * Define for the need of node network maintenance */
class WireProtocol: private Receiver {
private:
    // UDP server for discovery
    AsyncUDPServer udp_server;
    
    // Implement udp receive call back
    virtual void receive(const std::string& ip, unsigned short port, const std::string& data) override;
   
    // Encapsulating low-level stuff
    void send(const Node& node, std::string data);

public:
    // Constructor: port for peer discovery must be provided
    WireProtocol(unsigned short port);

    // Run the protocol
    void run();

    virtual void send_ping(const Node& node);
    virtual void send_pong(const Node& node);
    // TODO: summarize and add more

    virtual void receive_ping(const Node& node);
    virtual void receive_pong(const Node& node);
};


// Node table and maintenance
class NodeTable {
private:

    std::unordered_set<Node> nodes;


    // Maintenance loop
    void maintenance();
public:

    // Get the nodes to peer up with
    std::unordered_set<Node> get_neighbors();

    // Join the network and run the maintenance loop
    void start();
    // Leave the network
    void stop();
};
#include <string>

// Model of nodes
class Node {
private:
    // P2P network info
    std::string id;
    unsigned long level;
    bool is_contact_node;
    
    // Low-level info
    std::string ip;
    unsigned int port;

public:
    // Constructor
    Node(std::string id, unsigned long level, std::string ip, unsigned int port);

    // Getters
    std::string get_id() const;
    unsigned long get_level() const;
    bool get_is_contact_node() const;

    std::string get_ip() const;
    unsigned int get_port() const;

    // Setters
    void set_level(unsigned long new_level);
    void set_is_contact_node(bool new_is_contact_node);

    void set_ip(std::string new_ip);
    void set_port(std::string new_port);

};

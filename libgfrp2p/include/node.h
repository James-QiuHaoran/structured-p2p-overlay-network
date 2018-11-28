#include <string>

/* Model of nodes
 * For data storage only
 * Mutex lock might be neede */
class Node {
private:
    // P2P network info
    std::string id;
    unsigned long level;
    // using cnode = contact node
    bool is_current_ring_cnode;
    bool is_sub_ring_cnode;
    
    // Low-level info
    std::string ip;
    unsigned short port;

public:
    // Constructor
    Node(std::string id, std::string ip, unsigned short port);

    // Getters
    std::string get_id() const;
    unsigned long get_level() const;
    bool get_is_current_ring_cnode() const;
    bool get_is_sub_ring_cnode() const;

    std::string get_ip() const;
    unsigned int get_port() const;

    // Setters
    void set_level(unsigned long new_level);
    void set_is_current_ring_cnode(bool new_value);
    void set_is_sub_ring_cnode(bool new_value);

    void set_ip(const std::string& new_ip);
    void set_port(unsigned int new_port);
};
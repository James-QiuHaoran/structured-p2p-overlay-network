#include <string>
#include <chrono>

/* Model of nodes
 * For data storage only */
class Node {
private:
    // P2P network info
    std::string id;
    // unsigned long level;
    // a node may have multiple ids, each level has 1 id

    // Low-level info
    std::string ip;
    unsigned short port;

    // using cnode = contact node
    // bool is_current_ring_cnode;
    // bool is_sub_ring_cnode;
    std::chrono::time_point<std::chrono::system_clock> last_ping;
    std::chrono::time_point<std::chrono::system_clock> last_pong;
    

public:
    // Constructor
    Node(std::string id, std::string ip, unsigned short port);

    // Getters
    std::string get_id() const;
    // unsigned long get_level() const;
    // bool get_is_current_ring_cnode() const;
    // bool get_is_sub_ring_cnode() const;

    std::string get_ip() const;
    unsigned int get_port() const;

    std::chrono::time_point<std::chrono::system_clock> get_last_ping() const;
    std::chrono::time_point<std::chrono::system_clock> get_last_pong() const;
    

    // Setters
    // void set_level(unsigned long new_level);
    // void set_is_current_ring_cnode(bool new_value);
    // void set_is_sub_ring_cnode(bool new_value);
    void set_ip(const std::string& new_ip);
    void set_port(unsigned int new_port);
    
    void set_last_pong_now();
    void set_last_ping_now();
};

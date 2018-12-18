#ifndef NODE_H
#define NODE_H

#include <string>
#include <chrono>

/* Model of nodes
 * For data storage only */
class Node {
private:
    // p2p network info
    std::string id;

    // Low-level info
    std::string ip;
    unsigned short port;

    std::chrono::time_point<std::chrono::system_clock> last_ping;
    std::chrono::time_point<std::chrono::system_clock> last_pong;
    
public:
    // Constructor
    Node(std::string id, std::string ip, unsigned short port);

    // Getters
    std::string get_id() const;
    std::string get_ip() const;
    unsigned int get_port() const;

    std::chrono::time_point<std::chrono::system_clock> get_last_ping() const;
    std::chrono::time_point<std::chrono::system_clock> get_last_pong() const;
    

    // Setters
    void set_id(const std::string& id);
    void set_ip(const std::string& new_ip);
    void set_port(unsigned int new_port);
    
    void set_last_pong_now();
    void set_last_ping_now();
};

#endif
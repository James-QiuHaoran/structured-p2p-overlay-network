#ifndef NODE_TABLE_ETH_H
#define NODE_TABLE_ETH_H

#include <string>
#include <memory>
#include <mutex>
#include <vector>

#include <boost/log/trivial.hpp>

#include "node.h"

// Node table and maintenance
class NodeTableETH {
private:
    std::string self_id;

    // routing table
    std::vector<std::shared_ptr<Node>> table;

    // copy of pointer, use only when locked
    std::shared_ptr<Node> get_node(const std::string& id);
    std::shared_ptr<Node> copy_node(const std::shared_ptr<Node>& node);
 
public:
    NodeTableETH();
    NodeTableETH(const std::string& self_id);

    // thread safty
    std::mutex *mlock;

    // all operations must get lock, except those involving const members only
    /* self operations */
    std::string get_self_id() const;

    // set-up (used for evaluation)
    void add_node_to_table(const Node &node);
    void remove_node_from_table(const std::string &id);
    void set_table(std::vector<std::shared_ptr<Node>> table);
    std::vector<std::shared_ptr<Node>> get_table();

    /* storage operations */
    bool has_node(const std::string& id);
    std::shared_ptr<Node> get_node_copy(const std::string& id); // deep copy of node information

    /* thread safe node operations */
    void set_node_last_pong_now(const std::string& id);
    void set_node_last_ping_now(const std::string& id);

    /* domain logic */
    // all nodes returned are deep copy for thread safty
    std::vector<std::shared_ptr<Node>> get_peer_set();
    std::shared_ptr<Node> get_peer(const std::string& id);
};

#endif
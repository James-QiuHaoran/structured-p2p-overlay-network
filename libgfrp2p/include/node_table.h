#ifndef NODE_TABLE_H
#define NODE_TABLE_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <mutex>

#include <boost/log/trivial.hpp>

#include "node.h"

using boost::numeric_cast;

// If an id exists in multiple maps, they must be the same object
struct Ring {
    unsigned long ring_level;                                              // level of the ring
    std::unordered_map<std::string, std::shared_ptr<Node>> contact_nodes;  // contact nodes of the ring
    std::shared_ptr<Node> predecessor;                                     // successor within the ring
    std::shared_ptr<Node> successor;                                       // predecessor within the ring
    std::unordered_map<std::string, std::shared_ptr<Node>> peer_set;       // peers in an unordered map
    std::vector<std::shared_ptr<Node>> peer_list;                          // used for broadcast within ring
};

// Node table and maintenance
class NodeTable {
private:
    const std::string self_id;

    // store nodes in level-referenced sets
    std::vector<Ring> tables;

    // thread safty
    std::mutex mlock;

    // copy of pointer, use only when locked
    std::shared_ptr<Node> get_node(unsigned long level, const std::string& id);
    std::shared_ptr<Node> copy_node(const std::shared_ptr<Node>& node);
 
public:
    NodeTable(const std::string& self_id);

    // all operations must get lock, except those involving const members only
    /* self operations */
    std::string get_self_id() const;

    /* storage operations */
    bool has_node(unsigned long level, const std::string& id);
    // deep copy of node information
    std::shared_ptr<Node> get_node_copy(unsigned long level, const std::string& id);

    /* thread safe node operations */
    void set_node_last_pong_now(unsigned long level, const std::string& id);
    void set_node_last_ping_now(unsigned long level, const std::string& id);

    /* domain logic */
    bool is_contact_node(unsigned long level);
    // all nodes returned are deep copy for thread safty
    std::unordered_set<std::shared_ptr<Node>> get_contact_nodes(unsigned long level);
    std::unordered_set<std::shared_ptr<Node>> get_successor(unsigned long level);
    std::unordered_set<std::shared_ptr<Node>> get_predecessor(unsigned long level);
    std::unordered_set<std::shared_ptr<Node>> get_peer_set(unsigned long level);
    std::shared_ptr<Node> get_peer(unsigned long level, const std::string& id);  // get the particular peer by id
    
    // for broadcast in ring (k-ary distributed spanning tree)
    std::shared_ptr<Node> get_peer_by_order(unsigned long level, int order);     // get the particular peer by order
    int get_peer_list_size(unsigned long level);                                 // get the size of the peer_list in the ring
};

#endif
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <mutex>

#include <boost/log/trivial.hpp>

#include "node.h"

struct Ring {
    // If an id exists in multiple maps, they must be the same object
    std::unordered_map<std::string, std::shared_ptr<Node>> contact_nodes;
    std::unordered_map<std::string, std::shared_ptr<Node>> predecessors;
    std::unordered_map<std::string, std::shared_ptr<Node>> successors;
};

// Node table and maintenance
class NodeTable {

private:
    
    const std::string self_id;

    unsigned long self_level;

    // Store nodes in level-referenced sets
    std::vector<Ring> table;

    // Thread safty
    std::mutex mlock;

    // Copy of pointer, use only when locked
    std::shared_ptr<Node> get_node(const std::string& id);
    std::shared_ptr<Node> copy_node(const std::shared_ptr<Node>& node);
 
public:
    NodeTable(const std::string& self_id);

    // All operations except those involving const members only must get lock

    /* Self operations */
    std::string get_self_id() const;
    unsigned long get_self_level();

    /* Storage operations */
    bool has_node(const std::string& id);
    // Deep copy of node information
    std::shared_ptr<Node> get_node_copy(const std::string& id);

    /* Thread safe node operations */
    void set_node_last_pong_now(const std::string& id);
    void set_node_last_ping_now(const std::string& id);

    /* Domain logic */
    bool is_contact_node(unsigned long level);
    // All nodes returned are deep copy for thread safty
    std::unordered_set<std::shared_ptr<Node>> get_contact_nodes(unsigned long level);
    std::unordered_set<std::shared_ptr<Node>> get_successors(unsigned long level);
    std::unordered_set<std::shared_ptr<Node>> get_predecessors(unsigned long level);
    
};
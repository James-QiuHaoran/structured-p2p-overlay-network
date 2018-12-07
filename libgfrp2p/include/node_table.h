#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <mutex>

#include <boost/log/trivial.hpp>

#include "node.h"

struct Ring {
    std::unordered_set<std::shared_ptr<Node>> contact_nodes;
    std::unordered_set<std::shared_ptr<Node>> predecessors;
    std::unordered_set<std::shared_ptr<Node>> successors;
};

// Node table and maintenance
class NodeTable {

private:
    
    const std::string self_id;

    unsigned long self_level;

    // Store nodes in <id, node> hash table;
    std::vector<Ring> table;

    // Thread safty
    std::mutex mlock;

public:
    NodeTable(const std::string& self_id);

    // All operations should be  
    /* Self operations */
    std::string get_self_id() const;
    unsigned long get_self_level();

    /* Storage operations */
    bool has_node(const std::string& id);
    // Deep copy of node information
    std::shared_ptr<Node> get_node(const std::string& id);

    /* Thread safe node operations */
    void set_node_last_pong_now(const std::string& id);
    void set_node_last_ping_now(const std::string& id);

    /* Domain logic */
    bool is_contact_node(unsigned long level);
    std::unordered_set<std::shared_ptr<Node>> get_contact_nodes(unsigned long level);
    std::unordered_set<std::shared_ptr<Node>> get_successors(unsigned long level);
    std::unordered_set<std::shared_ptr<Node>> get_predecessors(unsigned long level);
    
    

};
#include "node_table.h"

/* private functions */
std::shared_ptr<Node> NodeTable::get_node(unsigned long level, const std::string& id) {
    auto ring = tables[level];

    // find from contact nodes
    auto contact_nodes_iter = ring.contact_nodes.find(id);
    if (contact_nodes_iter != ring.contact_nodes.end()) 
        return contact_nodes_iter->second;
    
    // find from peer list
    auto peer_set_iter = ring.peer_set.find(id);
    if (peer_set_iter != ring.peer_set.end())
        return peer_set_iter->second;
    
    // not found
    return std::shared_ptr<Node>();
}

std::shared_ptr<Node> NodeTable::copy_node(const std::shared_ptr<Node>& node) {
    return std::make_shared<Node>(*node);
}

/* public functions */
NodeTable::NodeTable() {
    this->mlock = new std::mutex();
}
NodeTable::NodeTable(const std::string& self_id):
    self_id(self_id) { 
        this->mlock = new std::mutex();
        BOOST_LOG_TRIVIAL(debug) << "NodeTable is created for node with ID " + self_id;
    }

std::string NodeTable::get_self_id() const {
    return this->self_id;
}

void NodeTable::add_table(Ring ring) {
    this->tables.push_back(ring);
}

void NodeTable::remove_table(unsigned long level) {
    this->tables.erase(this->tables.begin() + level);
}

void NodeTable::set_tables(std::vector<Ring> tables) {
    /*this->reset_tables();
    for (auto table : tables) {
        Ring ring;
        ring.ring_level = table.ring_level;
        ring.contact_nodes = table.contact_nodes;
        ring.peer_set = table.peer_set;
        ring.peer_list = table.peer_list;
        ring.successor = table.successor;
        ring.predecessor = table.predecessor;
        this->tables.push_back(ring);
    }*/
    this->tables = tables;
}

void NodeTable::reset_tables() {
    this->tables.clear();
}

std::vector<Ring> NodeTable::get_tables() {
    return this->tables;
}

bool NodeTable::has_node(unsigned long level, const std::string& id) {
    std::lock_guard<std::mutex> lock(*mlock);
    auto ring = this->tables[level];
    if (ring.contact_nodes.find(id) != ring.contact_nodes.end() || ring.peer_set.find(id) != ring.peer_set.end())
        return true;
    return false;
}

std::shared_ptr<Node> NodeTable::get_node_copy(unsigned long level, const std::string& id) {
    std::lock_guard<std::mutex> lock(*mlock);
    auto node = this->get_node(level, id);
    if (node)
        return this->copy_node(node);
    return std::shared_ptr<Node>();
}

void NodeTable::set_node_last_ping_now(unsigned long level, const std::string& id) {
    std::lock_guard<std::mutex> lock(*mlock);
    auto node = this->get_node(level, id);
    if (!node) return;
    node->set_last_ping_now();
}

void NodeTable::set_node_last_pong_now(unsigned long level, const std::string& id) {
    std::lock_guard<std::mutex> lock(*mlock);
    auto node = this->get_node(level, id);
    if (!node) return;
    node->set_last_pong_now();
}

/* domain logic functions */
bool NodeTable::is_contact_node(unsigned long level) {
    std::lock_guard<std::mutex> lock(*mlock);
    if (level > this->tables.size() - 1) {
        // BOOST_LOG_TRIVIAL(debug) << "not in that ring";
        return false;  // does not resides in the ring of that level
    } else { //  (level <= this->tables.size() - 1)
        // BOOST_LOG_TRIVIAL(debug) << "in higher ring";
        return true;   // resides in higher levels -> must be one of the contact nodes of that level's ring
    }/* else {
        BOOST_LOG_TRIVIAL(debug) << "not in one level higher";
        return false;  // does not resides in one level higher
    }*/
}

std::unordered_set<std::shared_ptr<Node>> NodeTable::get_contact_nodes(unsigned long level) {
    std::lock_guard<std::mutex> lock(*mlock);
    std::unordered_set<std::shared_ptr<Node>> result;

    // does not reside in that level's ring
    if (level > this->tables.size() - 1)
        return result;

    // return contact nodes of the ring
    auto ring = this->tables.at(level);
    for (const auto& kv: ring.contact_nodes) {
        result.insert(this->copy_node(kv.second));
    }  
    return result;
}

std::shared_ptr<Node> NodeTable::get_successor(unsigned long level) {
    std::lock_guard<std::mutex> lock(*mlock);
    std::shared_ptr<Node> result;

    // does not reside in that level's ring
    if (level > this->tables.size() - 1)
        return result;

    // return contact nodes of the ring
    auto ring = this->tables.at(level);
    result = this->copy_node(ring.successor);
    return result;
}

std::shared_ptr<Node> NodeTable::get_predecessor(unsigned long level) {
    std::lock_guard<std::mutex> lock(*mlock);
    std::shared_ptr<Node> result;
    
    // does not reside in that level's ring
    if (level > this->tables.size() - 1)
        return result;

    // return contact nodes of the ring
    auto ring = this->tables.at(level);
    result = this->copy_node(ring.predecessor);
    return result;
}

std::unordered_set<std::shared_ptr<Node>> NodeTable::get_peer_set(unsigned long level) {
    std::lock_guard<std::mutex> lock(*mlock);
    std::unordered_set<std::shared_ptr<Node>> result;

    // does not reside in that level's ring
    if (level > this->tables.size() - 1)
        return result;

    // return contact nodes of the ring
    auto ring = this->tables.at(level);
    for (const auto& kv: ring.peer_set) {
        result.insert(this->copy_node(kv.second));
    }  
    return result;
}

std::shared_ptr<Node> NodeTable::get_peer(unsigned long level,  const std::string &id) {
    // does not reside in that level's ring
    if (level > this->tables.size() - 1)
        return std::shared_ptr<Node>();

    // return the particular node
    return this->get_node_copy(level, id);
}

std::shared_ptr<Node> NodeTable::get_peer_by_order(unsigned long level, int order) {
    // does not reside in that level's ring
    if (level > this->tables.size() - 1)
        return std::shared_ptr<Node>();

    // return the particular node
    auto ring = tables[level];
    return this->copy_node(ring.peer_list.at(order));
}

int NodeTable::get_node_id_in_vector(unsigned long level, const std::string& id) {
    // does not reside in that level's ring
    if (level > this->tables.size() - 1)
        return -1;

    auto ring = this->tables.at(level);
    int i = 0;
    for (auto& node : ring.peer_list) {
        if (node->get_id() == id)
            return i;
        else
            i++;
    }
    return -1;
}

int NodeTable::get_peer_list_size(unsigned long level) {
    // does not reside in that level's ring
    if (level > this->tables.size() - 1)
        return -1;

    // return the end id
    auto ring = this->tables.at(level);
    return (ring.peer_list.size() - 1);
}

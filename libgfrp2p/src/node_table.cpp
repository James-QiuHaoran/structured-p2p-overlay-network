#include "node_table.h"

/* Private functions */
std::shared_ptr<Node> NodeTable::get_node(const std::string& id) {
    for (const auto& r : table) {
        auto contact_nodes_iter = r.contact_nodes.find(id),
             predecessors_iter = r.predecessors.find(id),
             successors_iter = r.successors.find(id);

        if (contact_nodes_iter != r.end()) 
            return *contact_nodes_iter;
        if (contact_nodes_iter != r.end()) 
            return *predecessors_iter;
        if (contact_nodes_iter != r.end()) 
            return *successors_iter;
    }
    return std::shared_ptr<Node>();
}

std::shared_ptr<Node> NodeTable::copy_node(const std::shared_ptr<Node>& node) {
    return std::make_shared<Node>(*node);
}

/* Public functions */
NodeTable::NodeTable(const std::string& self_id):
    self_id(self_id), self_level(0) { }

std::string NodeTable::get_self_id() const {
    return this->self_id;
}

unsigned long NodeTable::get_self_level() {
    std::lock_guard<std::mutex> lock(this->mlock);
    return this->self_level;
}

bool NodeTable::has_node(const std::string& id) {
    std::lock_guard<std::mutex> lock(this->mlock);
    for (const auto& r : table) {
        if (r.contact_nodes.find(id) != r.end() || 
            r.predecessors.find(id) != r.end() || 
            r.successors.find(id) != r.end())
        
            return true;
    }
    return false;
}

std::shared_ptr<Node> NodeTable::get_node_copy(const std::string& id) {
    std::lock_guard<std::mutex> lock(this->mlock);
    for (const auto& r : table) {
        auto contact_nodes_iter = r.contact_nodes.find(id),
             predecessors_iter = r.predecessors.find(id),
             successors_iter = r.successors.find(id);

        if (contact_nodes_iter != r.end()) 
            return copy_node(*contact_nodes_iter);
        if (contact_nodes_iter != r.end()) 
            return copy_node(*predecessors_iter);
        if (contact_nodes_iter != r.end()) 
            return copy_node(*successors_iter);
    }
    return std::shared_ptr<Node>();
}

void NodeTable::set_node_last_ping_now(const std::string& id) {
    std::lock_guard<std::mutex> lock(this->mlock);
    auto node = this->get_node(id);
    if (!node) return;
    node->set_last_ping_now();
}

void NodeTable::set_node_last_pong_now(const std::string& id) {
    std::lock_guard<std::mutex> lock(this->mlock);
    auto node = this->get_node(id);
    if (!node) return;
    node->set_last_pong_now();
}

bool NodeTable::is_contact_node(unsigned long level) {
    std::lock_guard<std::mutex> lock(this->mlock);
    return level < this->self_level;
}

std::unordered_set<std::shared_ptr<Node>> NodeTable::get_contact_nodes(unsigned long level) {
    std::lock_guard<std::mutex> lock(this->mlock);
    std::unordered_set<std::shared_ptr<Node>> result;
    if (level > this->self_level) return result;

    for (const auto& kv: this->table.at(level).contact_nodes) {
        result.insert(this->copy_node(kv.second));
    }  
    return result;
}

std::unordered_set<std::shared_ptr<Node>> NodeTable::get_successors(unsigned long level) {
    std::lock_guard<std::mutex> lock(this->mlock);
    std::unordered_set<std::shared_ptr<Node>> result;
    if (level > this->self_level) return result;

    for (const auto& kv: this->table.at(level).successors) {
        result.insert(this->copy_node(kv.second));
    }  
    return result;
}

std::unordered_set<std::shared_ptr<Node>> NodeTable::get_predecessors(unsigned long level) {
    std::lock_guard<std::mutex> lock(this->mlock);
    std::unordered_set<std::shared_ptr<Node>> result;
    if (level > this->self_level) return result;

    for (const auto& kv: this->table.at(level).predecessors) {
        result.insert(this->copy_node(kv.second));
    }  
    return result;
}

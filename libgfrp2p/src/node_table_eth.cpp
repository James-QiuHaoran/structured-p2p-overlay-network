#include "node_table_eth.h"

/* private functions */
std::shared_ptr<Node> NodeTableETH::get_node(const std::string& id) {
    // find from the table
    for (auto node : this->table) {
        if (node->get_id() == id) {
            return node;
        }
    }
    
    // not found
    return std::shared_ptr<Node>();
}

std::shared_ptr<Node> NodeTableETH::copy_node(const std::shared_ptr<Node>& node) {
    return std::make_shared<Node>(*node);
}

/* public functions */
NodeTableETH::NodeTableETH() {
    this->mlock = new std::mutex();
}

NodeTableETH::NodeTableETH(const std::string& self_id):
    self_id(self_id) { 
        this->mlock = new std::mutex();
        std::cout << "NodeTable is created for node with ID " + self_id << "\n";
    }

std::string NodeTableETH::get_self_id() const {
    return this->self_id;
}

void NodeTableETH::add_node_to_table(const Node &node) {
    this->table.push_back(std::make_shared<Node>(node));
}

void NodeTableETH::remove_node_from_table(const std::string &id) {
    int i = 0;
    for (auto node : this->table) {
        if (node->get_id() == id) {
            this->table.erase(this->table.begin() + i);
            return;
        }
        i++;
    }
    return;
}

void NodeTableETH::set_table(std::vector<std::shared_ptr<Node>> table) {
    this->table = table;
}

std::vector<std::shared_ptr<Node>> NodeTableETH::get_table() {
    return this->table;
}

bool NodeTableETH::has_node(const std::string& id) {
    for (auto node : this->table) {
        if (node->get_id() == id) {
            return true;
        }
    }
    return false;
}

std::shared_ptr<Node> NodeTableETH::get_node_copy(const std::string& id) {
    std::lock_guard<std::mutex> lock(*mlock);

    for (auto node : this->table) {
        if (node->get_id() == id) {
            return this->copy_node(node);
        }
    }
    return std::shared_ptr<Node>();
}

void NodeTableETH::set_node_last_ping_now(const std::string& id) {
    std::lock_guard<std::mutex> lock(*mlock);
    auto node = this->get_node(id);
    if (!node) 
        return;
    node->set_last_ping_now();
}

void NodeTableETH::set_node_last_pong_now(const std::string& id) {
    std::lock_guard<std::mutex> lock(*mlock);
    auto node = this->get_node(id);
    if (!node) 
        return;
    node->set_last_pong_now();
}

/* domain logic functions */
std::vector<std::shared_ptr<Node>> NodeTableETH::get_peer_set() {
    std::lock_guard<std::mutex> lock(*mlock);
    std::vector<std::shared_ptr<Node>> result;

    for (const auto& node : this->table) {
        result.push_back(this->copy_node(node));
    }

    return result;
}

std::shared_ptr<Node> NodeTableETH::get_peer(const std::string& id) {
    return this->get_node_copy(id);
}

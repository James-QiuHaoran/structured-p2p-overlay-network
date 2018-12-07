#include "node.h"

// Constructors
// Node::Node(std::string id, std::string ip, unsigned short port): id(id), level(0), 
//     is_current_ring_cnode(false), is_sub_ring_cnode(false), ip(ip), port(port) { // }
Node::Node(std::string id, std::string ip, unsigned short port): id(id), 
    ip(ip), port(port), last_ping(std::chrono::system_clock::now()), last_pong(std::chrono::system_clock::now()) { }

// Getters
std::string Node::get_id() const { return this->id; }
// unsigned long Node::get_level() const { return this->level; }
// bool Node::get_is_current_ring_cnode() const { return this->is_current_ring_cnode; }
// bool Node::get_is_sub_ring_cnode() const { return this->is_sub_ring_cnode; }

std::string Node::get_ip() const { return this->ip; }
unsigned int Node::get_port() const { return this->port; }

// Setters, side effect might be needed
// void Node::set_level(unsigned long new_level) { this->level = new_level; }
// void Node::set_is_current_ring_cnode(bool new_value) { this->is_current_ring_cnode = new_value; }
// void Node::set_is_sub_ring_cnode(bool new_value) { this->is_sub_ring_cnode = new_value; }

void Node::set_ip(const std::string& new_ip) { this->ip = new_ip; }
void Node::set_port(unsigned int new_port) { this->port = new_port; }

void Node::set_last_ping_now() {this->last_ping = std::chrono::system_clock::now(); }
void Node::set_last_pong_now() {this->last_pong = std::chrono::system_clock::now(); }
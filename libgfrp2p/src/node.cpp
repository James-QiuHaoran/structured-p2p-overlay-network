#include "node.h"

// Constructors
Node::Node(std::string id, std::string ip, unsigned short port): 
	id(id), 
    ip(ip), 
    port(port), 
    last_ping(std::chrono::system_clock::now()), 
    last_pong(std::chrono::system_clock::now()) { }

// Getters
std::string Node::get_id() const { return this->id; }
std::string Node::get_ip() const { return this->ip; }
unsigned int Node::get_port() const { return this->port; }

// Setters, side effect might be needed
void Node::set_id(const std::string& id) { this-> id = id; }
void Node::set_ip(const std::string& new_ip) { this->ip = new_ip; }
void Node::set_port(unsigned int new_port) { this->port = new_port; }

void Node::set_last_ping_now() {this->last_ping = std::chrono::system_clock::now(); }
void Node::set_last_pong_now() {this->last_pong = std::chrono::system_clock::now(); }
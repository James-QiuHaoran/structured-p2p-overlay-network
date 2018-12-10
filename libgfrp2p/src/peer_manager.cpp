#include "peer_manager.h"

// constructor
PeerManager::PeerManager() {

}

Message::Message() {

}

PeerError::PeerError() {

}

// getters
Node Message::get_sender() const {
	return this->sender;
}

Node Message::get_receiver() const {
	return this->receiver;
}

int Message::get_from_level() const {
	return this->from_level;
}

int Message::get_direction() const {
	return this->direction;
}

std::string Message::get_messageID() const {
	return this->messageID;
}

std::string Message::get_messageHash() const {
	return this->messageHash;
}

int Message::get_type() const {
	return this->type;
}

std::string PeerError::get_errorType() const {
	return this->errorType;
}

std::string PeerError::get_errorMessage() const {
	return this->errorMessage;
}

// setters
void Message::set_sender(Node sender) {
	this->sender = sender;
}

void Message::set_receiver(Node receiver) {
	this->receiver = receiver;
}

void Message::set_from_level(int level) {
	this->from_level = level;
}

void Message::set_direction(int direction) {
	this->direction = direction;
}

void Message::set_messageID(string messageId) {
	this->messageId = messageId;
}

void Message::set_messageHash(string messageHash) {
	this->messageHash = messageHash;
}

void Message::set_type(string type) {
	this->type = type;
}

void PeerError::set_errorType(string type) {
	this->errorType = type;
}

void PeerError::set_errorMessage(string message) {
	this->errorMessage = message;
}

std::shared_ptr<Node> PeerManager::get_node() {
	return this->node;
}

NodeTable get_node_table() {
	return this->node_table;
}

// a node wants to broadcast a message
void PeerManager::broadcast(const Message &msg, unsigned long current_level) {
	// get all contact nodes of the current ring
	std::unordered_set<std::shared_ptr<Node>> contact_nodes = this->node_table.get_contact_nodes(current_level);

	// randomly select one contact from the contact nodes [TODO]
	std::shared_ptr<Node> contact_node = contact_nodes.begin();

	// ask contact node to broadcast
	send(contact_node, msg);
}

// a node wants to multicast to the contact nodes of the current level
void multicast_to_contact_nodes(const Message &msg, unsigned long current_level) {
	// get all contact nodes of the current ring
	std::unordered_set<std::shared_ptr<Node>> contact_nodes = this->node_table.get_contact_nodes(current_level);

	// multicast to all contact nodes of the same level
	for (auto node = contact_nodes.begin(); node != contact_nodes.end(); node++) {
		send(node, msg);
	}
}

void PeerManager::send(std::shared_ptr<Node> node, const Message &msg) {
	// using wire protcol - TCP Transportation
	AsyncTCPServer tcp_server = new AsyncTCPServer();

	// generate data to send
	std::string data = "Hello World!";

	// send
	tcp_server.send(node->get_ip(), node->get_port(), data);
}

// broadcast upwards to the contact nodes of the upper level ring
// recursive function
void PeerManager::broadcast_up(const Message &msg, unsigned long current_level) {
	// get all contact nodes from the upper level ring
	std::unordered_set<std::shared_ptr<Node>> contact_nodes_upper = this->node_table.get_contact_nodes(current_level);

	// already reach the highest level, start to broadcast downwards
	if (contact_nodes_upper == NULL) {
		broadcast_within_ring(msg, current_level);
		return;
	}

	// randomly select one contact from the contact nodes [TODO]
	std::shared_ptr<Node> contact_node = contact_nodes_upper.begin();

	// ask contact node in the upper ring to broadcast
	send(contact_node, msg);

	return;
}

// broadcast to the nodes within the ring (k-ary distributed spanning tree)
void PeerManager::broadcast_within_ring(const Message &msg, unsigned long current_level, int k) {
	// should be recursive
	int end_ID = this->node_table.get_end_ID(current_level);
	int i = 0;
	int current_id = this->node->get_id() + k^i;
	while (current_id <= end_ID) {
		if (k^i <= this->node->get_id()) {
			continue;
		} else {
			std::shared_ptr<Node> node = this->node_table.get_peer(current_level, current_id);
			// wrap the message [TODO]
			send(node, msg);
		}
	}
}

// broadcast downwards to the contact nodes of the lower level ring
// recursive function
void PeerManager::broadcast_down(const Message &msg, unsigned long current_level) {
	// get all contact nodes from the upper level ring
	std::unordered_set<std::shared_ptr<Node>> contact_nodes_lower = this->node_table.get_contact_nodes(current_level);

	// already the lowest level
	if (contact_nodes_lower == NULL) {
		return;
	}

	// randomly select one contact from the contact nodes [TODO]
	std::shared_ptr<Node> contact_node = contact_nodes_lower.begin();

	// ask contact node in the upper ring to broadcast
	send(contact_node, msg);

	return;
}

void PeerManager::on_receive(const Message &msg) {
	if (msg.get_direction() == 0) {
		// message comes from same level ring
		if (msg.get_type() == 0) {
			// to broadcast upwards
			broadcast_up(msg, msg.get_from_level());
		} else if (msg.get_type() == 1) {
			// to receive this message
			// [TODO]
		}
	} else if (msg.get_direction() == 1) {
		// message comes from lower level ring
		if (this->node_table.is_contact_node(msg.get_from_level() + 1)) {
			// it's a contact node
			if (msg.get_type() == 0) {
				// to broadcast upwards
				broadcast_up(msg, msg.get_from_level());
			} else if (msg.get_type() == 1) {
				// to receive this message
				// [TODO]
			}
		} else {
			// not a contact node
			if (msg.get_type() == 0) {
				// to broadcast upwards
				// send to contact node to let it broadcast
				broadcast(msg, msg.get_from_level() + 1);
			} else if (msg.get_type() == 1) {
				// to receive this message
				// [TODO]
			}
		}
	} else if (msg.get_direction() == -1) {
		// message comes from upper level ring (can only be informing election results)
		if (this->node_table.is_contact_node(msg.get_from_level() + 1)) {
			// it's a contact node
			// multicast to other contact nodes
			multicast_to_contact_nodes(msg, msg.get_from_level() - 1);
		} else {
			// not a contact node
			// multicast to contact nodes
			multicast_to_contact_nodes(msg, msg.get_from_level() - 1);
		}
	}
	return;
}

std::unordered_set<shared_ptr<Node>> PeerManager::contact_node_election() {
	// random_IDs = []
	// for i = 0 -> NUM_CONTACT_NODES:
	//	random_IDs.append(generate_random_number_using_SGX() mod num_nodes)

	// after contact node elected, broadcast the result
	// broadcast_within_ring(random_IDs)
	// multicast_upper_ring(random_IDs)
	// multicast_lower_ring(random_IDs)
}

// on a node join
void on_new_connection(shared_ptr<Node> node) {

}

// on a node leave
void on_lost_connection(shared_ptr<Node> node) {

}

// create and initialize a peer
void PeerManager::create_peer() {

}

// connect to the network
void PeerManager::connect() {

}

// start the server
void PeerManager::start() {

}

// stop the peer
void PeerManager::stop() {

}


/*
// to be put in node_table.cpp
void PeerManager::detect_node_left() {
	if (liveness_check_predecessor(this.predecessor, this.pre-predecessor) == false)
		in_ring_broadcast()  // broadcast to update node info within the smallest ring
	if (liveness_check_successor(this.successor, this.suc-successor) == false)
		in_ring_broadcast()  // broadcast to update node info within the smallest ring
}

bool PeerManager::liveness_check_predecessor(predecessor, pre-predecessor) {
	return_obj = ping(predecessor.ip)
	if (return_obj.msg == 'TIMEOUT') {
		// check with its pre-predecessor (Remote-Procedure-Call)
		status = check_your_successor(pre-predecessor.ip)
		if (status == false) {
			// recheck
			return_obj = ping(predecessor.ip)
			if (return_obj.msg == 'TIMEOUT')
				return false;  // timeout
			else
				return true;   // alive
		}
		else
			return true;   // alive
	}
}

bool PeerManager::liveness_check_successor(successor, suc-successor) {
	return_obj = ping(successor.ip)
	if (return_obj.msg == 'TIMEOUT') {
		// check with its suc-successor (Remote-Procedure-Call)
		status = check_your_predecessor(suc-successor.ip)
		if (status == false) {
			// recheck
			return_obj = ping(successor.ip)
			if (return_obj.msg == 'TIMEOUT')
				return false;  // timeout
			else
				return true;   // alive
		}
		else
			return true;   // alive
	}
}

// remote procedure call executed by its pre-predecessor
bool PeerManager::check_your_predecessor() {
	return_obj = ping(this.predecessor.ip)
	if (return_obj.msg == 'TIMEOUT')
		return false;
	else
		return true;
}

// remote procedure call executed by its suc-successor
bool PeerManager::check_your_sucessor() {
	return_obj = ping(this.successor.ip)
	if (return_obj.msg == 'TIMEOUT')
		return false;
	else
		return true;
}
*/
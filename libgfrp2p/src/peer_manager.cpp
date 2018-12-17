#include "peer_manager.h"
#include <cstdlib>

// generate a random alpha-numeric string of length len
std::string random_string(size_t length) {
    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}

// constructor
PeerManager::PeerManager() {}

Message::Message() {}

Message::Message(std::string messageID, int type, unsigned long from_level, Node sender, Node receiver):
	messageID(messageID),
	type(type),
	from_level(from_level),
	sender(sender),
	receiver(receiver) {}

PeerError::PeerError() {}

PeerError::PeerError(std::string errorType, std::string errorMessage):
	errorType(errorType),
	errorMessage(errorMessage) {}

// getters
Node Message::get_sender() const { return this->sender; }

Node Message::get_receiver() const { return this->receiver; }

unsigned long Message::get_from_level() const { return this->from_level; }

std::string Message::get_messageID() const { return this->messageID; }

int Message::get_type() const { return this->type; }

std::string PeerError::get_errorType() const { return this->errorType; }

std::string PeerError::get_errorMessage() const { return this->errorMessage; }

std::shared_ptr<Node> PeerManager::get_node() { return this->node; }

NodeTable get_node_table() { return this->node_table; }

// setters
void Message::set_sender(const Node &sender) { this->sender = sender; }

void Message::set_receiver(const Node &receiver) { this->receiver = receiver; }

void Message::set_from_level(unsigned long level) { this->from_level = level; }

void Message::set_messageID(string messageId) { this->messageId = messageId; }

void Message::set_type(string type) { this->type = type; }

void PeerError::set_errorType(string type) { this->errorType = type; }

void PeerError::set_errorMessage(string message) { this->errorMessage = message; }

void PeerManager::set_node(std::shared_ptr<Node> node) { this->node = node; }

void PeerManager::set_node_table(NodeTable node_table) { this->node_table = node_table; }

// a node wants to broadcast a message
void PeerManager::broadcast(const Message &msg, unsigned long current_level) {
	// get all contact nodes of the current ring
	std::unordered_set<std::shared_ptr<Node>> contact_nodes = this->node_table.get_contact_nodes(current_level);

	// randomly select one contact from the contact nodes
	int random_id = rand() % contact_nodes.size();
	int i = 0;
	for (auto contact_node : contact_nodes) {
		if (i == random_id)
			break;
		else
			i++;
	}

	// ask contact node to broadcast
	send(contact_node, msg);
}

// a node wants to multicast to the contact nodes of the current level
void multicast_to_contact_nodes(const Message &msg, unsigned long current_level) {
	// get all contact nodes of the current ring
	std::unordered_set<std::shared_ptr<Node>> contact_nodes = this->node_table.get_contact_nodes(current_level);

	// multicast to all contact nodes of the same level
	for (auto node : contact_nodes) {
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

	// randomly select one contact from the contact nodes
	int random_id = rand() % contact_nodes.size();
	int i = 0;
	for (auto contact_node : contact_nodes) {
		if (i == random_id)
			break;
		else
			i++;
	}

	// ask contact node in the upper ring to broadcast
	send(contact_node, msg);

	return;
}

// broadcast to the nodes within the ring (k-ary distributed spanning tree)
void PeerManager::broadcast_within_ring(const Message &msg, unsigned long current_level, int k) {
	// should be recursive
	int end_ID = this->node_table.get_end_ID(current_level);
	int i = 0;
	int current_id;
	while (current_id <= end_ID) {
		int current_id = this->node->get_id() + k^i;
		if (k^i <= this->node->get_id()) {
			i++;
			continue;
		} else {
			std::shared_ptr<Node> node = this->node_table.get_peer_by_order(current_level, current_id);
			send(node, msg);
			i++;
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

	// randomly select one contact from the contact nodes
	int random_id = rand() % contact_nodes.size();
	int i = 0;
	for (auto contact_node : contact_nodes) {
		if (i == random_id)
			break;
		else
			i++;
	}

	// ask contact node in the upper ring to broadcast
	send(contact_node, msg);

	return;
}

// on receiving a message
void PeerManager::on_receive(const Message &msg) {
	switch(msg.get_type()) {
		case 0 : {
			std::cout << "Broadcast Upwards - from the lower level\n";
			if (!this->node_table.is_contact_node(msg.get_from_level()+1)) {
				// if not contact node
				Message msg(random_string(MSG_HASH_LENGTH), 1, msg.get_from_level()+1, this->node, NULL);
				this->broadcast_up(msg, msg.get_from_level()+1);
			} else if (this->node_table.get_contact_nodes(msg.get_from_level()+2).size() == 0) {
				// has been the top ring, start to broadcast downwards
				Message msg(random_string(MSG_HASH_LENGTH), 2, msg.get_from_level()+1, this->node, NULL);
				this->broadcast_within_ring(msg, msg.get_from_level()+1);
			} else {
				// keep broadcast upwards
				Message msg(random_string(MSG_HASH_LENGTH), 0, msg.get_from_level()+1, this->node, NULL);
				this->broadcast_up(msg, msg.get_from_level()+1);
			}
			break;
		} case 1 : {
			std::cout << "Broadcast Upwards - from the same level\n";
			if (!this->node_table.is_contact_node(msg.get_from_level())) {
				// if not contact node
				Message msg(random_string(MSG_HASH_LENGTH), 1, msg.get_from_level(), this->node, NULL);
				this->broadcast_up(msg, msg.get_from_level());
			} else if (this->node_table.get_contact_nodes(msg.get_from_level()+1).size() == 0) {
				// has been the top ring, start to broadcast downwards
				Message msg(random_string(MSG_HASH_LENGTH), 2, msg.get_from_level(), this->node, NULL);
				this->broadcast_within_ring(msg, msg.get_from_level());
			} else {
				// keep broadcast upwards
				Message msg(random_string(MSG_HASH_LENGTH), 0, msg.get_from_level(), this->node, NULL);
				this->broadcast_up(msg, msg.get_from_level());
			}
			break;
		} case 2 : {
			std::cout << "Broadcast Downwards\n";
			if (msg.get_from_level() == 1) {
				// has been the bottom ring, receive the message
				std::cout << "Message Received [touch the end-point]\n";
			} else {
				// keep broadcast downwards
				Message msg(random_string(MSG_HASH_LENGTH), 2, msg.get_from_level()-1, this->node, NULL);
				this->broadcast_within_ring(msg, msg.get_from_level()-1);
			}
			break;
		} case 3 : {
			std::cout << "Election Result Broadcast Upwards & Downwards One Level\n";
			// continue to broadcast within ring

			// downwards to all nodes of the lower level ring
			Message lower_ring_msg(random_string(MSG_HASH_LENGTH), 3, level, this->node, NULL);
			if (level != 0)
				this->broadcast_within_ring(lower_ring_msg, level-1, k);
			break;
		} case 4 : {
			std::cout << "Election Result Received\n";
		} default : {
			std::cout << "Unknown Message Type\n";
			break;
		}
	}

	return;
}

// elect the contact nodes for the next period
std::unordered_set<shared_ptr<Node>> PeerManager::contact_node_election(unsigned long level) {
	std::unordered_set<int> random_IDs;
	int num = 0;
	int num_peers = this->node_table.get_end_id(level);
	while (num < NUM_CONTACT_NODES) {
		int random_ID = rand() % num_peers;
		if (random_IDs.find(random_ID) == random_IDs.end()) {
			random_IDs.insert(random_ID);
			num++;
		}
	}

	// after contact nodes are elected, broadcast the result
	int k = 2;
	Message within_ring_msg(random_string(MSG_HASH_LENGTH), 3, level, this->node, NULL);
	broadcast_within_ring(within_ring_msg, level, k);

	Message upper_ring_msg(random_string(MSG_HASH_LENGTH), 3, level, this->node, NULL);
	if (this->node_table.has_upper_ring(level))
		multicast_to_contact_nodes(upper_ring_msg, level+1);

	Message lower_ring_msg(random_string(MSG_HASH_LENGTH), 3, level, this->node, NULL);
	if (level != 0)
		broadcast_within_ring(lower_ring_msg, level-1, k);
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
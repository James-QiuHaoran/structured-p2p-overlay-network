#include "peer_manager.h"

// constructor
PeerManager::PeerManager() {

}

Message::Message() {

}

PeerError::PeerError() {

}


// getters
std::string Message::get_sender() const {
	return this->sender;
}

std::string Message::get_receiver() const {
	return this->receiver;
}

std::string Message::get_messageID() const {
	return this->messageID;
}

std::string Message::get_messageHash() const {
	return this->messageHash;
}

std::string Message::get_type() const {
	return this->type;
}

std::string PeerError::get_errorType() const {
	return this->errorType;
}

std::string PeerError::get_errorMessage() const {
	return this->errorMessage;
}

// setters
void Message::set_sender(string sender) {
	this->sender = sender;
}

void Message::set_receiver(string receiver) {
	this->receiver = receiver;
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

std::unordered_set<Node> PeerManager::get_contact_nodes_this() {
	return this->contact_nodes_this;
}

std::unordered_set<Node> get_contact_nodes_upper() {
	return this->contact_nodes_upper;
}

// a node want to broadcast
void PeerManager::broadcast(Message msg) {
	Node contact_node = get_contact_nodes_this();

	// wrap the message [TODO]

	// ask contact node to broadcast
	send(contact_node, msg);
}

void PeerManager::send(Node node, Message msg) {}

// broadcast upward to the contact nodes of the upper level ring
void PeerManager::broadcast_up(Message msg) {
	Node contact_nodes_upper = get_contact_nodes_upper();

	// wrap the message [TODO]

	// ask contact node in the upper ring to broadcast
	send(contact_node, msg);
}

void PeerManager::broadcast_down(Message msg) {
	// if it's a contact_node then broadcast to the ring using the k-ary distributed spanning tree
}

void PeerManager::on_receive(Message msg) {
	bool isContactNode = is_contact_node_this();
	if (isContactNode) {
		if (msg.sender.level < msg.receiver.level) {
			broadcast_down(msg);
		} else {
			broadcast_up(msg);
		}
	} else {
		if (msg.sender.level < msg.receiver.level) {
			Node contact_node = get_contact_nodes_this();
			// wrap the message [TODO]
			// ask contact node to broadcast
			send(contact_node, msg);
		} else {
			// do something regarding the message
		}
	}
}

void PeerManager::on_node_lost_connection() {
	// if a neighbor does not reply your heart beat message
}

void PeerManager::on_node_leave() {
	// if a node leave is detected and verified
}

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

std::unordered_set<Node> PeerManager::contact_node_election() {
	// random_IDs = []
	// for i = 0 -> NUM_CONTACT_NODES:
	//	random_IDs.append(generate_random_number_using_SGX() mod num_nodes)
}

// after contact node elected, broadcast the result
void PeerManager::election_result_broadcast() {
	// broadcast_within_ring(random_IDs)
	// multicast_upper_ring(random_IDs)
}
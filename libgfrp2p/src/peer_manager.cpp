#include "peer_manager.h"

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

// constructors
PeerManager::PeerManager() {}

PeerManager::PeerManager(unsigned short port) {}

PeerManager::PeerManager(Node node, NodeTable node_table) {
	this->node = std::make_shared<Node>(node);
	this->node_table = std::make_shared<NodeTable>(node_table);
}

Message::Message() {}

Message::Message(std::string messageID, int type, unsigned long from_level, std::string sender_id, std::string receiver_id):
	message_id(messageID),
	type(type),
	from_level(from_level),
	sender_id(sender_id),
	receiver_id(receiver_id) {
		this->node_order = 0;
	}

PeerError::PeerError() {}

PeerError::PeerError(std::string errorType, std::string errorMessage):
	errorType(errorType),
	errorMessage(errorMessage) {}

// getters
std::string Message::get_sender_id() const { return this->sender_id; }

std::string Message::get_receiver_id() const { return this->receiver_id; }

unsigned long Message::get_from_level() const { return this->from_level; }

std::string Message::get_message_id() const { return this->message_id; }

int Message::get_node_order() const { return this->node_order; }

int Message::get_type() const { return this->type; }

std::string PeerError::get_errorType() const { return this->errorType; }

std::string PeerError::get_errorMessage() const { return this->errorMessage; }

std::shared_ptr<Node> PeerManager::get_node() { return this->node; }

std::shared_ptr<NodeTable> PeerManager::get_node_table() { return this->node_table; }

// setters
void Message::set_sender_id(const std::string &sender_id) { this->sender_id = sender_id; }

void Message::set_receiver_id(const std::string &receiver_id) { this->receiver_id = receiver_id; }

void Message::set_from_level(unsigned long level) { this->from_level = level; }

void Message::set_message_id(std::string msgID) { this->message_id = msgID; }

void Message::set_node_order(int order) { this->node_order = order; }

void Message::set_type(int type) { this->type = type; }

void PeerError::set_errorType(std::string type) { this->errorType = type; }

void PeerError::set_errorMessage(std::string message) { this->errorMessage = message; }

void PeerManager::set_node(std::shared_ptr<Node> node) { this->node = node; }

void PeerManager::set_node_table(std::shared_ptr<NodeTable> node_table) { this->node_table = node_table; }

// send message using transport layer 
// using wire protcol - TCP Transportation
void PeerManager::send(std::shared_ptr<Node> node, const Message &msg) {
	// generate data to send
	std::string data = node->get_id() + "," + 
					   this->node->get_id() + "," +
					   msg.get_message_id() + "," + 
					   std::to_string(msg.get_type()) + "," + 
					   std::to_string(msg.get_from_level()) + "," + 
					   std::to_string(msg.get_node_order());

	// send
	this->tcp_server->send(node->get_ip(), node->get_port(), data);

	return;
}

// a node wants to broadcast a message
void PeerManager::broadcast(Message msg, unsigned long current_level) {
	// get all contact nodes of the current ring
	std::unordered_set<std::shared_ptr<Node>> contact_nodes = this->node_table->get_contact_nodes(current_level);

	// randomly select one contact from the contact nodes
	int random_id = rand() % contact_nodes.size();
	int i = 0;
	std::shared_ptr<Node> receiver;
	for (auto contact_node : contact_nodes) {
		if (i == random_id) {
			receiver = contact_node;
			break;
		}
		else
			i++;
	}

	// ask contact node to broadcast
	msg.set_receiver_id(receiver->get_id());
	this->send(receiver, msg);

	return;
}

// a node wants to multicast to the contact nodes of the current level
void PeerManager::multicast_to_contact_nodes(Message msg, unsigned long current_level) {
	// get all contact nodes of the current ring
	std::unordered_set<std::shared_ptr<Node>> contact_nodes = this->node_table->get_contact_nodes(current_level);

	// multicast to all contact nodes of the same level
	for (auto node : contact_nodes) {
		std::shared_ptr<Node> receiver = node;
		msg.set_receiver_id(receiver->get_id());
		this->send(node, msg);
	}

	return;
}

// broadcast upwards to the contact nodes of the upper level ring
// recursive function
void PeerManager::broadcast_up(Message msg, unsigned long current_level) {
	// get all contact nodes from the upper level ring
	std::unordered_set<std::shared_ptr<Node>> contact_nodes_upper = this->node_table->get_contact_nodes(current_level);

	// already reach the highest level, start to broadcast downwards
	if (contact_nodes_upper.size() == 0) {
		int k = 2;
		broadcast_within_ring(msg, current_level, k);
		return;
	}

	// randomly select one contact from the contact nodes
	int random_id = rand() % contact_nodes_upper.size();
	int i = 0;
	std::shared_ptr<Node> receiver;
	for (auto contact_node : contact_nodes_upper) {
		if (i == random_id) {
			receiver = contact_node;
			break;
		}
		else
			i++;
	}

	// ask contact node in the upper ring to broadcast
	msg.set_receiver_id(receiver->get_id());
	this->send(receiver, msg);

	return;
}

// broadcast to the nodes within the ring (k-ary distributed spanning tree)
void PeerManager::broadcast_within_ring(Message msg, unsigned long current_level, int k) {
	// should be recursive
	int end_ID = this->node_table->get_peer_list_size(current_level);
	int i = 0;
	int current_id;
	int node_id = msg.get_node_order();
	int node_id_in_vector = this->node_table->get_node_id_in_vector(current_level, this->node->get_id());
	while (current_id <= end_ID) {
		current_id = node_id + pow(k, i);
		if (pow(k, i) <= node_id) {
			i++;
			continue;
		} else {
			std::shared_ptr<Node> node = this->node_table->get_peer_by_order(current_level, node_id_in_vector + pow(k, i));
			std::shared_ptr<Node> receiver = node;
			msg.set_receiver_id(receiver->get_id());
			this->send(receiver, msg);
			i++;
		}
	}
}

// broadcast downwards to the contact nodes of the lower level ring
// recursive function
void PeerManager::broadcast_down(Message msg, unsigned long current_level) {
	// get all contact nodes from the lower level ring
	std::unordered_set<std::shared_ptr<Node>> contact_nodes_lower = this->node_table->get_contact_nodes(current_level-1);

	// already the lowest level
	if (contact_nodes_lower.size() != 0) {
		return;
	}

	// randomly select one contact from the contact nodes
	int random_id = rand() % contact_nodes_lower.size();
	int i = 0;
	std::shared_ptr<Node> receiver;
	for (auto contact_node : contact_nodes_lower) {
		if (i == random_id) {
			receiver = contact_node;
			break;
		}
		else
			i++;
	}

	// ask contact node in the upper ring to broadcast
	msg.set_receiver_id(receiver->get_id());
	this->send(receiver, msg);

	return;
}

// on receiving a packet
void PeerManager::receive(const std::string& ip, unsigned short port, const std::string &data) {
	BOOST_LOG_TRIVIAL(info) << "TCP - receive: Packet received from " + ip + ":" + std::to_string(port) + "\nData: " + data;

	// parsing data
	std::size_t pos_start = 0;
	std::size_t pos_end = data.find(",", 0);
	std::string sender_id = data.substr(pos_start, pos_end);
	pos_start = pos_end + 1;
	pos_end = data.find(",", pos_end+1);
	std::string receiver_id = data.substr(pos_start, pos_end);
	pos_start = pos_end + 1;
	pos_end = data.find(",", pos_end+1);
	std::string messageID = data.substr(pos_start, pos_end);
	pos_start = pos_end + 1;
	pos_end = data.find(",", pos_end+1);
	int message_type = std::stoi(data.substr(pos_start, pos_end));
	pos_start = pos_end + 1;
	pos_end = data.find(",", pos_end+1);
	int message_from_level = std::stoi(data.substr(pos_start, pos_end));
	pos_start = pos_end + 1;
	pos_end = data.find(",", pos_end+1);
	int data_node_id = std::stoi(data.substr(pos_start, pos_end));

	Message msg = Message(messageID, message_type, message_from_level, sender_id, receiver_id);
	msg.set_node_order(data_node_id);

	// enter control flow
	this->on_receive(msg);

	return;
}

// on receiving a message
void PeerManager::on_receive(const Message &msg) {
	// simulate traffic control - delay according to ID difference
	std::string sender_id = msg.get_sender_id();
	std::string receiver_id = this->node->get_id();
	int sleep_time = 0;
	if (sender_id.substr(ID_CONTINENT_START, ID_CONTINENT_START+ID_CONTINENT_LEN) != receiver_id.substr(ID_CONTINENT_START, ID_CONTINENT_START+ID_CONTINENT_LEN)) {
		sleep_time = random_num_in_range(150, 200);
        	std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
	} else if (sender_id.substr(ID_COUNTRY_START, ID_COUNTRY_START+ID_COUNTRY_LEN) != receiver_id.substr(ID_COUNTRY_START, ID_COUNTRY_START+ID_COUNTRY_LEN)) {
		sleep_time = random_num_in_range(100, 150);
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
	} else if (sender_id.substr(ID_STATE_START, ID_STATE_START+ID_STATE_LEN) != receiver_id.substr(ID_STATE_START, ID_STATE_START+ID_STATE_LEN)) {
		sleep_time = random_num_in_range(50, 100);
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
	} else if (sender_id.substr(ID_CITY_START, ID_CITY_START+ID_CITY_LEN) != receiver_id.substr(ID_CITY_START, ID_CITY_START+ID_CITY_LEN)) {
		sleep_time = random_num_in_range(20, 50);
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
	}

	// control flow
	switch(msg.get_type()) {
		case 0 : {
			std::cout << "Broadcast Upwards - from the lower level\n";
			if (!this->node_table->is_contact_node(msg.get_from_level()+1)) {
				// if not contact node
				Message msg(random_string(MSG_HASH_LENGTH), 1, msg.get_from_level()+1, this->node->get_id(), NULL);
				this->broadcast_up(msg, msg.get_from_level()+1);
			} else if (this->node_table->get_contact_nodes(msg.get_from_level()+2).size() == 0) {
				// has been the top ring, start to broadcast downwards
				Message msg(random_string(MSG_HASH_LENGTH), 2, msg.get_from_level()+1, this->node->get_id(), NULL);
				int k = 2;
				this->broadcast_within_ring(msg, msg.get_from_level()+1, k);
			} else {
				// keep broadcast upwards
				Message msg(random_string(MSG_HASH_LENGTH), 0, msg.get_from_level()+1, this->node->get_id(), NULL);
				this->broadcast_up(msg, msg.get_from_level()+1);
			}
			break;
		} case 1 : {
			std::cout << "Broadcast Upwards - from the same level\n";
			if (!this->node_table->is_contact_node(msg.get_from_level())) {
				// if not contact node
				Message msg(random_string(MSG_HASH_LENGTH), 1, msg.get_from_level(), this->node->get_id(), NULL);
				this->broadcast_up(msg, msg.get_from_level());
			} else if (this->node_table->get_contact_nodes(msg.get_from_level()+1).size() == 0) {
				// has been the top ring, start to broadcast downwards
				Message msg(random_string(MSG_HASH_LENGTH), 2, msg.get_from_level(), this->node->get_id(), NULL);
				int k = 2;
				this->broadcast_within_ring(msg, msg.get_from_level(), k);
			} else {
				// keep broadcast upwards
				Message msg(random_string(MSG_HASH_LENGTH), 0, msg.get_from_level(), this->node->get_id(), NULL);
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
				Message msg(random_string(MSG_HASH_LENGTH), 2, msg.get_from_level()-1, this->node->get_id(), NULL);
				int k = 2;
				this->broadcast_within_ring(msg, msg.get_from_level()-1, k);
			}
			break;
		} case 3 : {
			std::cout << "Election Result Broadcast Upwards & Downwards One Level\n";
			// continue to broadcast within ring

			// downwards to all nodes of the lower level ring
			Message lower_ring_msg(random_string(MSG_HASH_LENGTH), 3, msg.get_from_level(), this->node->get_id(), NULL);
			int k = 2;
			if (msg.get_from_level() != 0)
				this->broadcast_within_ring(lower_ring_msg, msg.get_from_level()-1, k);
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
void PeerManager::contact_node_election(unsigned long level) {
	std::unordered_set<int> random_IDs;
	int num = 0;
	int num_peers = this->node_table->get_peer_list_size(level);
	while (num < NUM_CONTACT_NODES) {
		int random_ID = rand() % num_peers;
		if (random_IDs.find(random_ID) == random_IDs.end()) {
			random_IDs.insert(random_ID);
			num++;
		}
	}

	// after contact nodes are elected, broadcast the result
	int k = 2;
	Message within_ring_msg(random_string(MSG_HASH_LENGTH), 3, level, this->node->get_id(), NULL);
	broadcast_within_ring(within_ring_msg, level, k);

	// get all contact nodes from the upper level ring
        std::unordered_set<std::shared_ptr<Node>> contact_nodes_upper = this->node_table->get_contact_nodes(level);
        if (contact_nodes_upper.size() != 0) {
		Message upper_ring_msg(random_string(MSG_HASH_LENGTH), 3, level, this->node->get_id(), NULL);
		multicast_to_contact_nodes(upper_ring_msg, level+1);
	}

	Message lower_ring_msg(random_string(MSG_HASH_LENGTH), 3, level, this->node->get_id(), NULL);
	if (level != 0)
		broadcast_within_ring(lower_ring_msg, level-1, k);
}

// on a node join
void on_new_connection(std::shared_ptr<Node> node) {

}

// on a node leave
void on_lost_connection(std::shared_ptr<Node> node) {

}

// start the server
void PeerManager::start() {
	if (DEBUG_PM)
		std::cout << "Starting the TCP server on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]\n";
    this->tcp_server = new AsyncTCPServer(std::make_shared<PeerManager>(*this), this->node->get_port());
    
    if (DEBUG_PM)
		std::cout << "Running the TCP server on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]\n";
    this->tcp_server->run();
}

// stop the peer
void PeerManager::stop() {
	if (DEBUG_PM)
		std::cout << "Stopping the TCP server on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]\n";
    this->tcp_server->stop();
}

// random number generated uniformly from [low, high]
int PeerManager::random_num_in_range(int low, int high) {
	boost::random::uniform_int_distribution<> dist(low, high);
	return dist(gen);
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

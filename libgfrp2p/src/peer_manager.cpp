#include "peer_manager.h"

// HASH - generate a random alpha-numeric string of length len
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

PeerManager::PeerManager(const std::shared_ptr<Node>& node, const std::shared_ptr<NodeTable>& node_table, const std::string &start_time): 
	node(node), node_table(node_table) {
	// this->node = std::make_shared<Node>(node);
	// this->node_table = std::make_shared<NodeTable>(node_table);
	this->start_time = start_time;
}

PeerError::PeerError() {}

PeerError::PeerError(std::string errorType, std::string errorMessage):
	errorType(errorType),
	errorMessage(errorMessage) {}

// getters

std::string PeerError::get_errorType() const { return this->errorType; }

std::string PeerError::get_errorMessage() const { return this->errorMessage; }

std::shared_ptr<Node> PeerManager::get_node() { return this->node; }

std::shared_ptr<NodeTable> PeerManager::get_node_table() { return this->node_table; }

// setters
void PeerError::set_errorType(std::string type) { this->errorType = type; }

void PeerError::set_errorMessage(std::string message) { this->errorMessage = message; }

void PeerManager::set_node(std::shared_ptr<Node> node) { this->node = node; }

void PeerManager::set_node_table(std::shared_ptr<NodeTable> node_table) { this->node_table = node_table; }

// send message using transport layer 
// using wire protcol - TCP Transportation
void PeerManager::send(std::shared_ptr<Node> node, const Message &msg, const std::string &data, std::unordered_set<std::string> sent_ids) {
	// if the receiver is itself
	if (msg.get_receiver_id() == msg.get_sender_id()) {
		this->on_receive(msg, data, sent_ids);
		return;
	}

	// generate sent_ids_string
	std::string sent_ids_string = "";
	for (auto sent_id : sent_ids) {
		sent_ids_string += sent_id + ",";
	}
	if (sent_ids_string.length() > 0)
		sent_ids_string.erase(sent_ids_string.length() - 1, 1);

	// generate data to send
	// data format: sender_id,receiver_id,msg_id,type,from_level,node_order,data
	std::string data_string = this->node->get_id() + "," + 
							   node->get_id() + "," +
							   msg.get_message_id() + "," + 
							   std::to_string(msg.get_type()) + "," + 
							   std::to_string(msg.get_from_level()) + "," + 
							   std::to_string(msg.get_node_order()) + "," +
							   sent_ids_string + "|" +
							   data;

	// for message logging
	this->msg_table.insert_sent(msg);

	BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "Send msg - (" << msg.get_type() << ") | " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "] -> " << "[" << node->get_ip() << ":" << node->get_port() << "]" << " | FL: " << msg.get_from_level();

	// send via TCP
	this->tcp_server->send(node->get_ip(), node->get_port(), data_string);

	return;
}

// a node wants to broadcast a message
void PeerManager::broadcast(const std::string &data) {
	// wrap the data into a Message
	Message msg(random_string(MSG_HASH_LENGTH), 1, 0, this->node->get_id(), "");

	// get all contact nodes of the current ring
	std::unordered_set<std::shared_ptr<Node>> contact_nodes = this->node_table->get_contact_nodes(0);

	// randomly select one contact from the contact nodes
	// int random_id = rand() % contact_nodes.size();
	std::string random_id = std::string(32, '0');
	int i = 0;
	std::shared_ptr<Node> receiver;
	for (auto contact_node : contact_nodes) {
		// if (i == random_id) {
		if (contact_node->get_id() == random_id) {
			receiver = contact_node;
			break;
		}
		else
			i++;
	}

	// ask contact node to broadcast
	msg.set_receiver_id(receiver->get_id());

	BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "Broadcast msg - (" << msg.get_type() << ") | " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "] -> " << "[" << receiver->get_ip() << ":" << receiver->get_port() << "]" << " | FL: " << msg.get_from_level();
	
	std::unordered_set<std::string> sent_ids;
	this->send(receiver, msg, data, sent_ids);

	return;
}

// a node wants to multicast to the contact nodes of the current level
void PeerManager::multicast_to_contact_nodes(Message msg, unsigned long current_level, const std::string &data) {
	// get all contact nodes of the current ring
	std::unordered_set<std::shared_ptr<Node>> contact_nodes = this->node_table->get_contact_nodes(current_level);

	// multicast to all contact nodes of the same level
	for (auto node : contact_nodes) {
		std::shared_ptr<Node> receiver = node;
		msg.set_receiver_id(receiver->get_id());
		std::unordered_set<std::string> sent_ids;
		this->send(node, msg, data, sent_ids);
	}

	return;
}

// broadcast upwards to the contact nodes of the upper level ring
// recursive function
void PeerManager::broadcast_up(Message msg, unsigned long current_level, const std::string &data) {
	// get all contact nodes from the upper level ring
	std::unordered_set<std::shared_ptr<Node>> contact_nodes_upper = this->node_table->get_contact_nodes(current_level+1);

	std::unordered_set<std::string> sent_ids;

	// already reach the highest level, start to broadcast downwards
	if (contact_nodes_upper.size() == 0) {
		int k = 2;
		BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - Received data: " << data;
		this->broadcast_within_ring(msg, current_level, k, data, sent_ids);
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

	BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "Broadcast Up msg - (" << msg.get_type() << ") | " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "] -> " << "[" << receiver->get_ip() << ":" << receiver->get_port() << "]" << " | FL: " << msg.get_from_level();

	this->send(receiver, msg, data, sent_ids);

	return;
}

// broadcast to the nodes within the ring (k-ary distributed spanning tree)
void PeerManager::broadcast_within_ring(Message msg, unsigned long current_level, int k, const std::string &data, std::unordered_set<std::string> sent_ids) {
	// should be recursive
	int end_ID = this->node_table->get_peer_list_size(current_level);
	int i = 0;
	int node_order = msg.get_node_order(); // relative to the one who starts this broadcast-within-ring
	int node_id_in_vector = this->node_table->get_node_id_in_vector(current_level, this->node->get_id());
	int current_id = node_order;

	// regions sent - avoid repeated messages
	std::unordered_set<std::string> sent_ids_for_receivers;
	for (auto sent_id : sent_ids)
		sent_ids_for_receivers.insert(sent_id);

	int level_to_id_start[6] = {ID_SINGLE_START, ID_DISTRICT_START, ID_CITY_START, ID_STATE_START, ID_COUNTRY_START, ID_CONTINENT_START};
	int level_to_id_length[6] = {ID_SINGLE_LEN, ID_DISTRICT_LEN, ID_CITY_LEN, ID_STATE_LEN, ID_COUNTRY_LEN, ID_CONTINENT_LEN};

	std::string this_id = this->node->get_id().substr(level_to_id_start[current_level], level_to_id_length[current_level]);
	sent_ids.insert(this_id);
	sent_ids_for_receivers.insert(this_id);

	// form sent_ids_for_receivers
	while (node_order + pow(k, i) <= end_ID) {
		current_id = node_order + pow(k, i);
		if (pow(k, i) <= node_order) {
			i++;
			continue;
		} else {
			int target_node_id_in_vector = node_id_in_vector + pow(k, i);
			if (target_node_id_in_vector > end_ID)
				target_node_id_in_vector -= end_ID + 1;
			std::shared_ptr<Node> receiver = this->node_table->get_peer_by_order(current_level, target_node_id_in_vector);
			std::string region_id = receiver->get_id().substr(level_to_id_start[current_level], level_to_id_length[current_level]);
			sent_ids_for_receivers.insert(region_id);
			i++;
		}
	}

	i = 0;
	// send to other peers
	while (node_order + pow(k, i) <= end_ID) {
		current_id = node_order + pow(k, i);
		if (pow(k, i) <= node_order) {
			i++;
			continue;
		} else {
			int target_node_id_in_vector = node_id_in_vector + pow(k, i);
			if (target_node_id_in_vector > end_ID)
				target_node_id_in_vector -= end_ID + 1;

			std::shared_ptr<Node> node = this->node_table->get_peer_by_order(current_level, target_node_id_in_vector);
			std::shared_ptr<Node> receiver = node;
			msg.set_receiver_id(receiver->get_id());
			msg.set_node_order(current_id);

			// set type of the message
			std::string region_id = receiver->get_id().substr(level_to_id_start[current_level], level_to_id_length[current_level]);
			if (current_level == 0) {
				// no need to broadcast downwards
				msg.set_type(5);
			} else {
				std::string id_lower = receiver->get_id().substr(level_to_id_start[0], level_to_id_length[0]);
				if (id_lower != std::string(level_to_id_length[0], '0')) {
				// if (sent_ids.find(region_id) != sent_ids.end()) {
					// no need to broadcast downwards in that region
					msg.set_type(5);
				} else {
					// broadcast both downwards in that region and withing ring
					msg.set_type(2);

					// mark that region to be "sent"
					sent_ids.insert(region_id);
				}
			}

			BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "Broadcast W/ Ring " << node_order << "+" << k << "^" << i << " - (" << msg.get_type() << ") | " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "] -> " << "[" << receiver->get_ip() << ":" << receiver->get_port() << "]" << " | FL: " << msg.get_from_level();

			this->send(receiver, msg, data, sent_ids_for_receivers);
			i++;
		}
	}

	return;
}

// broadcast downwards to the contact nodes of the lower level ring
// recursive function
void PeerManager::broadcast_down(Message msg, unsigned long current_level, const std::string &data) {
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

	BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "Broadcast Down msg - (" << msg.get_type() << ") | " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "] -> " << "[" << receiver->get_ip() << ":" << receiver->get_port() << "]" << " | FL: " << msg.get_from_level();

	std::unordered_set<std::string> sent_ids;
	this->send(receiver, msg, data, sent_ids);

	return;
}

// on receiving a packet
void PeerManager::receive(const std::string& ip, unsigned short port, const std::string &data) {
	// parsing data
	std::size_t pos_start = 0;
	std::size_t pos_end = data.find(",", 0);
	std::string sender_id = data.substr(pos_start, pos_end-pos_start);
	pos_start = pos_end + 1;
	pos_end = data.find(",", pos_end+1);
	std::string receiver_id = data.substr(pos_start, pos_end-pos_start);
	pos_start = pos_end + 1;
	pos_end = data.find(",", pos_end+1);
	std::string messageID = data.substr(pos_start, pos_end-pos_start);
	pos_start = pos_end + 1;
	pos_end = data.find(",", pos_end+1);
	int message_type = std::stoi(data.substr(pos_start, pos_end-pos_start));
	pos_start = pos_end + 1;
	pos_end = data.find(",", pos_end+1);
	int message_from_level = std::stoi(data.substr(pos_start, pos_end-pos_start));
	pos_start = pos_end + 1;
	pos_end = data.find(",", pos_end+1);
	int data_node_id = std::stoi(data.substr(pos_start, pos_end-pos_start));
	pos_start = pos_end + 1;
	std::string data_in_msg = data.substr(pos_start);

	// retrieve sent_ids from data_in_msg
	std::unordered_set<std::string> sent_ids;
	pos_end = data_in_msg.find("|");
	std::string sent_ids_string = data_in_msg.substr(0, pos_end);
	data_in_msg.erase(0, pos_end+1);
	pos_start = 0;
	pos_end = sent_ids_string.find(",", 0);
	while(pos_end != std::string::npos) {
		sent_ids.insert(sent_ids_string.substr(pos_start, pos_end-pos_start));
		pos_start = pos_end + 1;
		pos_end = sent_ids_string.find(",", pos_start);
	}
	if (pos_start != sent_ids_string.size()-1) {
		std::string sent_id = sent_ids_string.substr(pos_start, sent_ids_string.size()-pos_start);
		if (sent_id.length() != 0)
			sent_ids.insert(sent_id);
	}

	Message msg = Message(messageID, message_type, message_from_level, sender_id, receiver_id);

	msg.set_node_order(data_node_id);

	BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "Received msg from wire - (" << msg.get_type() << ") | " << " -> " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "] | FL: " << msg.get_from_level();

	// for message logging
	this->msg_table.insert_received(msg);
	
	// enter control flow
	this->on_receive(msg, data_in_msg, sent_ids);

	return;
}

// on receiving a message
void PeerManager::on_receive(const Message &msg, const std::string &data, std::unordered_set<std::string> sent_ids) {
	BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "On Receive msg - (" << msg.get_type() << ") | " << " -> " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "] | FL: " << msg.get_from_level();

	std::string sender_id = msg.get_sender_id();
	std::string receiver_id = this->node->get_id();

	// simulate traffic control - delay according to ID difference
	int sleep_time = 0;
	if (sender_id.substr(ID_CONTINENT_START, ID_CONTINENT_START+ID_CONTINENT_LEN) != receiver_id.substr(ID_CONTINENT_START, ID_CONTINENT_START+ID_CONTINENT_LEN)) {
		sleep_time = random_num_in_range(160, 200);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
	} else if (sender_id.substr(ID_COUNTRY_START, ID_COUNTRY_START+ID_COUNTRY_LEN) != receiver_id.substr(ID_COUNTRY_START, ID_COUNTRY_START+ID_COUNTRY_LEN)) {
		sleep_time = random_num_in_range(120, 160);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
	} else if (sender_id.substr(ID_STATE_START, ID_STATE_START+ID_STATE_LEN) != receiver_id.substr(ID_STATE_START, ID_STATE_START+ID_STATE_LEN)) {
		sleep_time = random_num_in_range(80, 120);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
	} else if (sender_id.substr(ID_CITY_START, ID_CITY_START+ID_CITY_LEN) != receiver_id.substr(ID_CITY_START, ID_CITY_START+ID_CITY_LEN)) {
		sleep_time = random_num_in_range(40, 80);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
	} else if (sender_id.substr(ID_DISTRICT_START, ID_DISTRICT_START+ID_DISTRICT_LEN) != receiver_id.substr(ID_DISTRICT_START, ID_DISTRICT_START+ID_DISTRICT_LEN)) {
		sleep_time = random_num_in_range(0, 40);
		std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
	}

	// control flow
	switch(msg.get_type()) {
		case 0 : {
			BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "[MSG] Broadcast Upwards - from the lower level";
			if (!this->node_table->is_contact_node(msg.get_from_level()+1)) {
				// if not contact node
				Message msg_new(random_string(MSG_HASH_LENGTH), 1, msg.get_from_level()+1, this->node->get_id(), "");
				this->broadcast_up(msg_new, msg_new.get_from_level(), data);
			} else if (this->node_table->get_contact_nodes(msg.get_from_level()+2).size() == 0) {
				// has been the top ring, start to broadcast downwards
				Message msg_new(random_string(MSG_HASH_LENGTH), 2, msg.get_from_level()+1, this->node->get_id(), "");
				int k = 2;
				msg_new.set_node_order(0);
				BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - Received data: " << data;

				// within ring
				this->broadcast_within_ring(msg_new, msg_new.get_from_level(), k, data, sent_ids);

				// downwards
				int i = 0;
				while (msg.get_from_level() >= (unsigned long)i) {
					Message msg_down(random_string(MSG_HASH_LENGTH), 2, msg.get_from_level()-(unsigned long)i, this->node->get_id(), "");
					msg_down.set_node_order(0);
					k = 2;
					std::unordered_set<std::string> sent_ids_empty;
					this->broadcast_within_ring(msg_down, msg_down.get_from_level(), k, data, sent_ids_empty);
					i++;
				}
			} else {
				// keep broadcast upwards
				Message msg_new(random_string(MSG_HASH_LENGTH), 0, msg.get_from_level()+1, this->node->get_id(), "");
				this->broadcast_up(msg_new, msg_new.get_from_level(), data);
			}
			break;
		} case 1 : {
			BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "[MSG] Broadcast Upwards - from the same level";
			if (!this->node_table->is_contact_node(msg.get_from_level())) {
				// if not contact node
				Message msg_new(random_string(MSG_HASH_LENGTH), 1, msg.get_from_level(), this->node->get_id(), "");
				this->broadcast_up(msg_new, msg_new.get_from_level(), data);
			} else if (this->node_table->get_contact_nodes(msg.get_from_level()+1).size() == 0) {
				// has been the top ring, start to broadcast downwards
				Message msg_new(random_string(MSG_HASH_LENGTH), 2, msg.get_from_level(), this->node->get_id(), "");
				msg_new.set_node_order(0);
				int k = 2;
				BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - Received data: " << data;

				// within ring
				this->broadcast_within_ring(msg_new, msg_new.get_from_level(), k, data, sent_ids);

				// downwards
				int i = 1;
				while (msg.get_from_level() >= (unsigned long)i) {
					Message msg_down(random_string(MSG_HASH_LENGTH), 2, msg.get_from_level()-(unsigned long)i, this->node->get_id(), "");
					msg_down.set_node_order(0);
					k = 2;
					std::unordered_set<std::string> sent_ids_empty;
					this->broadcast_within_ring(msg_down, msg_down.get_from_level(), k, data, sent_ids_empty);
					i++;
				}
			} else {
				// keep broadcast upwards
				Message msg_new(random_string(MSG_HASH_LENGTH), 0, msg.get_from_level(), this->node->get_id(), "");
				this->broadcast_up(msg_new, msg_new.get_from_level(), data);
			}
			break;
		} case 2 : {
			BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "[MSG] Broadcast Within Ring & Downwards - I: Within Ring of level " << msg.get_from_level();
			
			if (msg.get_from_level() == this->node_table->get_top_level())
				BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - Received data: " << data;
			
			// within ring
			Message msg_new(random_string(MSG_HASH_LENGTH), 2, msg.get_from_level(), this->node->get_id(), "");
			msg_new.set_node_order(msg.get_node_order());
			int k = 2;
			this->broadcast_within_ring(msg_new, msg_new.get_from_level(), k, data, sent_ids);

			// downwards
			if (msg.get_from_level() >= (unsigned long)1) {
				BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "[MSG] Broadcast Within Ring & Downwards - II: Downwards to level " << msg.get_from_level() - 1;
				// keep broadcast downwards
				int i = 1;
				while (msg.get_from_level() >= (unsigned long)i) {
					Message msg_down(random_string(MSG_HASH_LENGTH), 2, msg.get_from_level()-(unsigned long)i, this->node->get_id(), "");
					msg_down.set_node_order(0);
					int k = 2;
					std::unordered_set<std::string> sent_ids_empty;
					this->broadcast_within_ring(msg_down, msg_down.get_from_level(), k, data, sent_ids_empty);
					i++;
				}
			}
			break;
		} case 5 : {
			BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "[MSG] Only Broadcast Within Ring of level " << msg.get_from_level();
			
			if (msg.get_from_level() == this->node_table->get_top_level())
				BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - Received data: " << data;
			
			// within ring
			Message msg_new(random_string(MSG_HASH_LENGTH), 2, msg.get_from_level(), this->node->get_id(), "");
			msg_new.set_node_order(msg.get_node_order());
			int k = 2;
			this->broadcast_within_ring(msg_new, msg_new.get_from_level(), k, data, sent_ids);

			break;
		} case 3 : {
			BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "[MSG] Election Result Broadcast Upwards & Downwards One Level";
			// continue to broadcast within ring

			// downwards to all nodes of the lower level ring
			Message lower_ring_msg(random_string(MSG_HASH_LENGTH), 3, msg.get_from_level(), this->node->get_id(), "");
			int k = 2;
			if (msg.get_from_level() != 0)
				this->broadcast_within_ring(lower_ring_msg, msg.get_from_level()-1, k, data, sent_ids);
			break;
		} case 4 : {
			BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "[MSG] Election Result Received";
			break;
		} default : {
			BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "[MSG] Unknown Message Type";
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
	Message within_ring_msg(random_string(MSG_HASH_LENGTH), 3, level, this->node->get_id(), "");
	std::string data = "Election Result";
	std::unordered_set<std::string> sent_ids_1;
	broadcast_within_ring(within_ring_msg, level, k, data, sent_ids_1);

	// get all contact nodes from the upper level ring
        std::unordered_set<std::shared_ptr<Node>> contact_nodes_upper = this->node_table->get_contact_nodes(level);
        if (contact_nodes_upper.size() != 0) {
		Message upper_ring_msg(random_string(MSG_HASH_LENGTH), 3, level, this->node->get_id(), "");
		multicast_to_contact_nodes(upper_ring_msg, level+1, data);
	}

	Message lower_ring_msg(random_string(MSG_HASH_LENGTH), 3, level, this->node->get_id(), "");
	if (level != 0) {
		std::unordered_set<std::string> sent_ids_2;
		broadcast_within_ring(lower_ring_msg, level-1, k, data, sent_ids_2);
	}

	return;
}

// on a node join
void on_new_connection(std::shared_ptr<Node> node) {
	return;
}

// on a node leave
void on_lost_connection(std::shared_ptr<Node> node) {
	return;
}

// start the server
void PeerManager::start() {
	BOOST_LOG_TRIVIAL(trace) << "Starting the TCP server on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]";
    this->tcp_server = new AsyncUDPServer(std::static_pointer_cast<Receiver>(this->shared_from_this()), this->node->get_port());
    
	BOOST_LOG_TRIVIAL(trace) << "Running the TCP server on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]";
    this->tcp_server->run();

    return;
}

// stop the peer
void PeerManager::stop() {
	BOOST_LOG_TRIVIAL(trace) << "Stopping the TCP server on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]";
    this->tcp_server->stop();

    return;
}

// random number generated uniformly from [low, high]
int PeerManager::random_num_in_range(int low, int high) {
	boost::random::uniform_int_distribution<> dist(low, high);
	return dist(gen);
}

// write messages received and sent to the file system
void PeerManager::log_messages() {
	std::ofstream myfile;
	myfile.open("../log/"+this->node->get_id()+"-"+this->start_time+".csv");

	myfile << Message::csv_header;
	myfile << this->msg_table.to_csv_string();

	myfile.close();
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

#include "peer_manager_eth.h"

// constructors
PeerManagerETH::PeerManagerETH() {
	this->mode = PeerManagerETH::PUSH;
}

PeerManagerETH::PeerManagerETH(unsigned short port) {
	this->mode = PeerManagerETH::PUSH;
}

PeerManagerETH::PeerManagerETH(const std::shared_ptr<Node>& node, const std::shared_ptr<NodeTableETH>& node_table, const std::string &start_time): 
	node(node), node_table(node_table) {
	this->start_time = start_time;

	this->mode = PeerManagerETH::PUSH;
}

PeerError::PeerError() {}

PeerError::PeerError(std::string errorType, std::string errorMessage):
	errorType(errorType),
	errorMessage(errorMessage) {}

// getters
std::string PeerError::get_errorType() const { return this->errorType; }

std::string PeerError::get_errorMessage() const { return this->errorMessage; }

std::shared_ptr<Node> PeerManagerETH::get_node() { return this->node; }

std::shared_ptr<NodeTableETH> PeerManagerETH::get_node_table() { return this->node_table; }

// setters
void PeerError::set_errorType(std::string type) { this->errorType = type; }

void PeerError::set_errorMessage(std::string message) { this->errorMessage = message; }

void PeerManagerETH::set_node(std::shared_ptr<Node> node) { this->node = node; }

void PeerManagerETH::set_node_table(std::shared_ptr<NodeTableETH> node_table) { this->node_table = node_table; }

void PeerManagerETH::set_mode(unsigned short mode) { this->mode = mode; }

// send message using transport layer 
// using wire protcol - TCP Transportation
void PeerManagerETH::send(std::shared_ptr<Node> node, const Message &msg, const std::string &data) {
	// generate data to send
	// data format: sender_id,receiver_id,broadcast_id,msg_id,type,ttl,data
	std::string data_string = this->node->get_id() + "," + 
							   node->get_id() + "," +
							   msg.get_broadcast_id() + "," +
							   msg.get_message_id() + "," +
							   std::to_string(msg.get_type()) + "," + 
							   std::to_string(msg.get_TTL()) + "," + 
							   data;

	// for message logging
	Message inserted_msg = this->msg_table.insert_sent(msg);
	this->append_message_record(inserted_msg);

	if (this->mode == PeerManagerETH::PUSH)
		BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "Send msg | " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "] -> " << "[" << node->get_ip() << ":" << node->get_port() << "]";
	else if (this->mode == PeerManagerETH::PULL) {
		if (msg.get_type() == 1)
			BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "Send invitation | " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "] -> " << "[" << node->get_ip() << ":" << node->get_port() << "]";
		else if (msg.get_type() == 2)
			BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "Send data | " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "] -> " << "[" << node->get_ip() << ":" << node->get_port() << "]";
		else if (msg.get_type() == 3) {
			BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "Send request | " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "] -> " << "[" << node->get_ip() << ":" << node->get_port() << "]";
		}
	}

	// BOOST_LOG_TRIVIAL(debug) << data;

	// send via TCP
	this->tcp_server->send(node->get_ip(), node->get_port(), data_string);

	return;
}

// a node wants to broadcast a message
void PeerManagerETH::broadcast(const std::string &data, int ttl, std::string broadcastID) {
	// generate data hash
    std::size_t data_hash = std::hash<std::string>{}(data);

	bool new_broadcast = false;
	std::stringstream ss;
	if (broadcastID == "") {
		ss.str("");
	    ss.clear();
	    ss << std::setw(NUM_MSG_LIMIT) << std::setfill('0') << this->broadcasted_msgs.size();
		broadcastID = this->node->get_id()+ss.str();
		new_broadcast = true;
	}

	// wrap the data into a Message
	ss.str("");
	ss.clear();
	ss << std::setw(MSG_HASH_LENGTH) << std::setfill('0') << this->msg_table.num_msgs_in_total();
	// std::string message_id = this->random_string_of_length(MSG_HASH_LENGTH);
	std::string message_id = this->node->get_id() + ss.str();
	Message msg(broadcastID, message_id, this->node->get_id(), "");
	msg.set_TTL(ttl);

	if (new_broadcast) {
		this->broadcasted_msgs.push_back(msg.get_broadcast_id());
		this->data_map.insert({std::to_string(data_hash), data});
	}

	// get all nodes in the routing table
	std::vector<std::shared_ptr<Node>> routing_table = this->node_table->get_peer_set();

	// receiver list
	std::unordered_set<std::shared_ptr<Node>> receiver_list;
	std::unordered_set<int> random_ids;

	// randomly select nodes to broadcast
    for (int i = 0; i < NUM_RECEIVERS_ETH; i++) {
        int id = this->random_num_in_range(0, TABLE_SIZE_ETH-1);
        if (random_ids.find(id) == random_ids.end()) {
            random_ids.insert(id);
            receiver_list.insert(routing_table[id]);
        } else {
            i--;
        }
    }

    // send out messages
	for (auto receiver : receiver_list) {
		msg.set_receiver_id(receiver->get_id());

		BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "Broadcast msg | " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "] -> " << "[" << receiver->get_ip() << ":" << receiver->get_port() << "]";
		
		if (this->mode == PeerManagerETH::PUSH)
			// push version
			this->send(receiver, msg, data);
		else {
			// pull version
			this->send_inv(receiver, std::to_string(data_hash), broadcastID);
		}
	}

	return;
}

// send data hash as invitation
void PeerManagerETH::send_inv(std::shared_ptr<Node> node, const std::string &data_hash, const std::string &broadcast_id) {
	// BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "Send invitation | " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "] -> " << "[" << node->get_ip() << ":" << node->get_port() << "]";
	std::stringstream ss;
	ss.str("");
	ss.clear();
	ss << std::setw(MSG_HASH_LENGTH) << std::setfill('0') << this->msg_table.num_msgs_in_total();
	// std::string message_id = this->random_string_of_length(MSG_HASH_LENGTH);
	std::string message_id = this->node->get_id() + ss.str();
	Message msg(broadcast_id, message_id, this->node->get_id(), node->get_id());
	msg.set_type(1);
	send(node, msg, data_hash);
}

// send real data
void PeerManagerETH::send_data(std::shared_ptr<Node> node, const std::string &data, const std::string &broadcast_id) {
	// BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "Send data | " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "] -> " << "[" << node->get_ip() << ":" << node->get_port() << "]";
	std::stringstream ss;
	ss.str("");
	ss.clear();
	ss << std::setw(MSG_HASH_LENGTH) << std::setfill('0') << this->msg_table.num_msgs_in_total();
	// std::string message_id = this->random_string_of_length(MSG_HASH_LENGTH);
	std::string message_id = this->node->get_id() + ss.str();
	Message msg(broadcast_id, message_id, this->node->get_id(), node->get_id());
	msg.set_type(2);
	send(node, msg, data);
}

// send request for data
void PeerManagerETH::send_request(std::shared_ptr<Node> node, const std::string &data_hash, const std::string &broadcast_id) {
	// BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "Send request | " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "] -> " << "[" << node->get_ip() << ":" << node->get_port() << "]";
	std::stringstream ss;
	ss.str("");
	ss.clear();
	ss << std::setw(MSG_HASH_LENGTH) << std::setfill('0') << this->msg_table.num_msgs_in_total();
	// std::string message_id = this->random_string_of_length(MSG_HASH_LENGTH);
	std::string message_id = this->node->get_id() + ss.str();
	Message msg(broadcast_id, message_id, this->node->get_id(), node->get_id());
	msg.set_type(3);
	send(node, msg, data_hash);
}

// on receiving a packet
void PeerManagerETH::receive(const std::string& ip, unsigned short port, const std::string &data) {
	// parsing data
	// data format: sender_id,receiver_id,broadcast_id,msg_id,type,ttl,data
	std::size_t pos_start = 0;
	std::size_t pos_end = data.find(",", 0);
	std::string sender_id = data.substr(pos_start, pos_end-pos_start);
	
	pos_start = pos_end + 1;
	pos_end = data.find(",", pos_end+1);
	std::string receiver_id = data.substr(pos_start, pos_end-pos_start);

	pos_start = pos_end + 1;
	pos_end = data.find(",", pos_end+1);
	std::string broadcastID = data.substr(pos_start, pos_end-pos_start);
	
	pos_start = pos_end + 1;
	pos_end = data.find(",", pos_end+1);
	std::string messageID = data.substr(pos_start, pos_end-pos_start);

	pos_start = pos_end + 1;
	pos_end = data.find(",", pos_end+1);
	int type = std::stoi(data.substr(pos_start, pos_end-pos_start));

	pos_start = pos_end + 1;
	pos_end = data.find(",", pos_end+1);
	std::string ttl = data.substr(pos_start, pos_end-pos_start);

	pos_start = pos_end + 1;
	std::string data_in_msg = data.substr(pos_start);

	if (ttl == "0" && this->mode == PeerManagerETH::PUSH) {
		// no need to broadcast
		return;
	}

	Message msg = Message(broadcastID, messageID, sender_id, receiver_id);
	msg.set_TTL(std::stoi(ttl));
	msg.set_type(type);

	BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "Received from wire | [" << ip << ":" << port << "] " << " -> " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "]";
	// BOOST_LOG_TRIVIAL(debug) << data_in_msg;
	// for message logging
	Message inserted_msg = this->msg_table.insert_received(msg);
	this->append_message_record(inserted_msg);
	
	// enter control flow
	this->on_receive(msg, data_in_msg, ip, port);

	return;
}

// on receiving a message
void PeerManagerETH::on_receive(const Message &msg, const std::string &data, const std::string& sender_ip, unsigned short sender_port) {
	BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "On Receive msg | [" << sender_ip << ":" << sender_port << "] -> " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "]";

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

	if (this->mode == PeerManagerETH::PULL) {
		// pull version
		if (msg.get_type() == 1) {
			// it is an invitation
			if (this->data_map.find(data) != this->data_map.end()) {
				BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "[MSG] Received useless inv | [" << this->node->get_ip() << ":" << this->node->get_port() << "] from [" << sender_ip << ":" << sender_port << "]";
				// already received, do nothing
				return;
			} else {
				// send request
				Node receiver(msg.get_sender_id(), sender_ip, sender_port);

				BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "[MSG] Received invitation | [" << this->node->get_ip() << ":" << this->node->get_port() << "] from [" << sender_ip << ":" << sender_port << "]";

				this->send_request(std::make_shared<Node>(receiver), data, msg.get_broadcast_id());
			}
		} else if (msg.get_type() == 2) {
			// it is the data, receive it (data format: data_hash|data_content)
			BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "[MSG] Received data | [" << this->node->get_ip() << ":" << this->node->get_port() << "] from [" << sender_ip << ":" << sender_port << "]";
			this->data_map.insert({data.substr(0, data.find("|")), data.substr(data.find("|")+1)});

			// then broadcast
			this->broadcast(data.substr(data.find("|")+1), 0, msg.get_broadcast_id());
		} else if (msg.get_type() == 3) {
			// it is the request, send the data
			std::string real_data = this->data_map.find(data.substr(0, data.find("|")))->second;

			Node receiver(msg.get_sender_id(), sender_ip, sender_port);

			BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "[MSG] Received request | [" << this->node->get_ip() << ":" << this->node->get_port() << "] from [" << sender_ip << ":" << sender_port << "]";

			this->send_data(std::make_shared<Node>(receiver), data + "|" + real_data, msg.get_broadcast_id());
		} else {
			// message not recognized, drop the message
			BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "[MSG] Message not recognized | [" << this->node->get_ip() << ":" << this->node->get_port() << "] from [" << sender_ip << ":" << sender_port << "]";
			return;
		}
	} else {
		// push version
		if (std::find(this->broadcasted_msgs_all_nodes.begin(), this->broadcasted_msgs_all_nodes.end(), msg.get_broadcast_id()) != this->broadcasted_msgs_all_nodes.end()) {
			// do not need to broadcast anymore
			BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "No need to broadcast | " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "] from [" << sender_ip << ":" << sender_port << "]";
			return;
		} else {
			// continue to broadcast with ttl decreasing by 1
			broadcasted_msgs_all_nodes.push_back(msg.get_broadcast_id());
			BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "Continue to broadcast | " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "] from [" << sender_ip << ":" << sender_port << "]";
			this->broadcast(data, msg.get_TTL()-1, msg.get_broadcast_id());
		}
	}

	return;
}

// on a node join
void PeerManagerETH::on_new_connection(std::shared_ptr<Node> node) {
	return;
}

// on a node leave
void PeerManagerETHon_lost_connection(std::shared_ptr<Node> node) {
	return;
}

// start the server
void PeerManagerETH::start() {
	BOOST_LOG_TRIVIAL(trace) << "Starting the TCP server on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]";
    this->tcp_server = new AsyncUDPServer(std::static_pointer_cast<Receiver>(this->shared_from_this()), this->node->get_port());
    
	BOOST_LOG_TRIVIAL(trace) << "Running the TCP server on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]";
    this->tcp_server->run();

    return;
}

// stop the peer
void PeerManagerETH::stop() {
	BOOST_LOG_TRIVIAL(trace) << "Stopping the TCP server on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]";
    this->tcp_server->stop();

    return;
}

// random number generated uniformly from [low, high]
int PeerManagerETH::random_num_in_range(int low, int high) {
	boost::random::uniform_int_distribution<> dist(low, high);
	return dist(gen);
}

// HASH - generate a random alpha-numeric string of length len
std::string PeerManagerETH::random_string_of_length(size_t length) {
	char randchar[length];
	const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
	for (size_t i = 0; i < length; i++) {
		int random_num = this->random_num_in_range(0, max_index);
		randchar[i] = charset[random_num];
	}
	std::string result = randchar;
	return result;
    /*auto randchar = [&]() -> char {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        int random_num = this->random_num_in_range(0, max_index);
        return charset[random_num];
    };
    std::string str(length,0);
    std::generate_n(str.begin(), length, randchar);
    return str;*/
}

// write messages received and sent to the file system
void PeerManagerETH::log_message_records() {
	std::ofstream ofs;
	ofs.open("../test/log/" + this->start_time + "/" + this->node->get_id() + ".csv", std::ofstream::out | std::ofstream::app);

	ofs << this->msg_table.to_csv_string();

	ofs.close();
}

void PeerManagerETH::append_message_record(const Message& msg) {
	std::ofstream ofs;
	ofs.open("../test/log/" + this->start_time + "/" + this->node->get_id() + ".csv", std::ofstream::out | std::ofstream::app);

	ofs << msg.to_csv_string() + "\n";

	ofs.close();
}
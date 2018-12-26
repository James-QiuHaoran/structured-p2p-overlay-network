#include "peer_manager_eth.h"

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
PeerManagerETH::PeerManagerETH() {}

PeerManagerETH::PeerManagerETH(unsigned short port) {}

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
	// if the receiver is itself
	if (msg.get_receiver_id() == msg.get_sender_id()) {
		this->on_receive(msg, data);
		return;
	}

	// generate data to send
	// data format: sender_id,receiver_id,msg_id,ttl,data
	std::string data_string = this->node->get_id() + "," + 
							   node->get_id() + "," +
							   msg.get_message_id() + "," + 
							   std::to_string(msg.get_TTL()) + "," +
							   data;

	// for message logging
	Message inserted_msg = this->msg_table.insert_sent(msg);
	this->append_message_record(inserted_msg);

	BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "Send msg | " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "] -> " << "[" << node->get_ip() << ":" << node->get_port() << "]";

	// send via TCP
	this->tcp_server->send(node->get_ip(), node->get_port(), data_string);

	return;
}

// a node wants to broadcast a message
void PeerManagerETH::broadcast(const std::string &data, int ttl) {
	// wrap the data into a Message
	Message msg(random_string(MSG_HASH_LENGTH), this->node->get_id(), "");
	msg.set_TTL(ttl);

	// get all nodes in the routing table
	std::vector<std::shared_ptr<Node>> routing_table = this->node_table->get_peer_set();

	// receiver list
	std::unordered_set<std::shared_ptr<Node>> receiver_list;
	std::unordered_set<int> random_ids;

	// randomly select nodes to broadcast
    for (int i = 0; i < NUM_RECEIVERS_ETH; i++) {
        int id = this->random_num_in_range(0, TABLE_SIZE_ETH-1);
        if (random_ids.find(id) != random_ids.end()) {
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
		
		this->send(receiver, msg, data);
	}

	return;
}

// send data hash as invitation
void PeerManagerETH::send_inv(std::shared_ptr<Node> node, const std::string &data_hash) {
	Message msg(random_string(MSG_HASH_LENGTH), this->node->get_id(), node->get_id());
	msg.set_type(1);
	send(node, msg, data_hash);
}

// send real data
void PeerManagerETH::send_data(std::shared_ptr<Node> node, const std::string &data) {
	Message msg(random_string(MSG_HASH_LENGTH), this->node->get_id(), node->get_id());
	msg.set_type(2);
	send(node, msg, data);
}

// on receiving a packet
void PeerManagerETH::receive(const std::string& ip, unsigned short port, const std::string &data) {
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
	std::string ttl = data.substr(pos_start, pos_end-pos_start);

	pos_start = pos_end + 1;
	std::string data_in_msg = data.substr(pos_start);

	if (ttl == "0") {
		// no need to broadcast
		return;
	}

	Message msg = Message(messageID, sender_id, receiver_id);
	msg.set_TTL(std::stoi(ttl));

	BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "Received msg from wire | [] " << " -> " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "]";

	// for message logging
	Message inserted_msg = this->msg_table.insert_received(msg);
	this->append_message_record(inserted_msg);
	
	// enter control flow
	this->on_receive(msg, data_in_msg);

	return;
}

// on receiving a message
void PeerManagerETH::on_receive(const Message &msg, const std::string &data) {
	BOOST_LOG_TRIVIAL(trace) << this->node->get_id() << " - " << "On Receive msg | [] -> " << "[" << this->node->get_ip() << ":" << this->node->get_port() << "]";

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

	if (this->msg_table.existID(msg.get_message_id())) {
		// do not need to broadcast anymore
		return;
	} else {
		// continue to broadcast
		this->broadcast(data, msg.get_TTL()-1);
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
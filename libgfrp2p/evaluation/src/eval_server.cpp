#include "eval_server.h"

EvalServer::EvalServer(const std::string& log_dir): log_dir_(log_dir) { }

void EvalServer::run(unsigned short port) {
	std::cout << "INFO: EvalServer::run: Initializing" << std::endl;
	tcp_server_.reset(new AsyncTCPServer(std::dynamic_pointer_cast<Receiver>(shared_from_this()), port));
	db_.reset(new NodeDatabase());
	std::cout << "INFO: EvalServer::run: Initialization complete" << std::endl;

	tcp_server_->run();
}

void EvalServer::stop() {
	tcp_server_->stop();
}

void EvalServer::receive(const std::string & ip, unsigned short port, const std::string & data) {
	BootstrapMessage msg;
	if (!msg.ParseFromString(data)) {
		std::cerr << "ERROR: EvalServer::receive: Failed to parse the received data" << std::endl;
		return;
	}

	if (msg.type() == BootstrapMessage::INIT) {
		// On receive node registration
		NodeRecord record;
		record.ip = ip;
		record.port = static_cast<unsigned short>(msg.init().port());
		std::lock_guard<std::mutex> lock(mlock_);
		db_->insert({db_->size(), record});
		std::cout << "INFO: EvalServer::receive: " << record.ip << ':' << record.port << " registered";
 	} else if (msg.type() == BootstrapMessage::PUSH_LOG) {
		// On receive node message log
		std::ofstream fout;
		std::string filename = log_dir_+'/'+ msg.push_log().run_id() + '_' + msg.push_log().node_id() + ".txt";
		fout.open(filename);
		if (!fout.is_open()) {
			std::cerr << "ERROR: EvalServer::receive: Failed to open " << filename << std::endl;
			return;
		}
		fout << msg.push_log().log();
		fout.close();
		std::cout << "INFO: EvalServer::receive: Successfully received log to " << filename << std::endl;
	} else {
		std::cerr << "ERROR: EvalServer::receive: Unknown message type received" << std::endl;
	}
}


std::size_t EvalServer::handle_count() {
	std::lock_guard<std::mutex> lock(mlock_);
	if (!db_) {
		std::cerr << "ERROR: EvalServer::stat: Database not initialized" << std::endl;
		return 0;
	} 
	return db_->size();
}

NodeRecord EvalServer::handle_check(std::size_t node_id) {
	std::lock_guard<std::mutex> lock(mlock_);
	if (!db_) {
		std::cerr << "ERROR: EvalServer::handle_check: Database not initialized" << std::endl;
		return NodeRecord();
	} 
	auto iter  = db_->find(node_id);
	if (iter == db_->end()) {
		std::cerr << "ERROR: EvalServer:: Node not found" << std::endl;
		return NodeRecord();
	}
	return iter->second;
}


void EvalServer::handle_config(const EvalConfig& config) {
	
	eval_config_.reset(new BootstrapMessage());

	eval_config_->mutable_config()->set_run_id(config.run_id);

	eval_config_->mutable_config()->set_eval_type(config.eval_type ? Config::KADEMLIA : Config::HGFRR);
	
	eval_config_->mutable_config()->set_num_nodes_in_dist(config.num_nodes_in_dist);
    eval_config_->mutable_config()->set_num_cnodes_in_dist(config.num_cnodes_in_dist);
    eval_config_->mutable_config()->set_num_nodes_in_city(config.num_nodes_in_city);
    eval_config_->mutable_config()->set_num_cnodes_in_city(config.num_cnodes_in_city);
    eval_config_->mutable_config()->set_num_nodes_in_state(config.num_nodes_in_state);
    eval_config_->mutable_config()->set_num_cnodes_in_state(config.num_cnodes_in_state);
    eval_config_->mutable_config()->set_num_nodes_in_country(config.num_nodes_in_country);
    eval_config_->mutable_config()->set_num_cnodes_in_country(config.num_cnodes_in_country);
	eval_config_->mutable_config()->set_num_nodes_in_continent(config.num_nodes_in_continent);

	std::lock_guard<std::mutex> lock(mlock_);

	if (!db_) {
		std::cerr << "ERROR: EvalServer::handle_config: Database not initialized" << std::endl;
		return;
	} 

	std::cout << "DEBUG: EvalServer::handle_config: Preparing node table" << std::endl;
	eval_config_->mutable_config()->set_table_size(db_->size());
	for (const auto& kv: *db_) {
		eval_config_->mutable_config()->add_table_ids(kv.first);
		eval_config_->mutable_config()->add_table_ips(kv.second.ip);
		eval_config_->mutable_config()->add_table_ports(kv.second.port);
	}

	std::cout << "INFO:: EvalServer::handle_config: Sending config: " << std::endl;
	int counter = 0;
	for (const auto& kv : *db_) {
		send_config(kv.first, kv.second.ip, kv.second.port);

		counter ++;
		if (int(double(counter)/db_->size()*10) == int(double(counter-1)/db_->size()*10) + 1)
			std::cout << "INFO:: EvalServer::handle_config: ... " << int(double(counter)/db_->size()*100) << '%' << std::endl;
	}
}

void EvalServer::handle_broadcast(std::size_t node_id, std::uint32_t workload_size) {
	std::lock_guard<std::mutex> lock(mlock_);
	if (!db_) {
		std::cerr << "ERROR: EvalServer::handle_broadcast: Database not initialized" << std::endl;
		return;
	} 
	auto iter = db_->find(node_id);
	if (iter == db_->end()) {
		std::cerr << "ERROR: EvalServer::handle_broadcast: Failed to find the record of the node" << std::endl;
		return;
	}

	send_broadcast(workload_size, iter->second.ip, iter->second.port);
}

void EvalServer::handle_pull_log(std::size_t node_id, const std::string& run_id) {
	std::lock_guard<std::mutex> lock(mlock_);
	if (!db_) {
		std::cerr << "ERROR: EvalServer::handle_pull_log: Database not initialized" << std::endl;
		return;
	} 
	auto iter = db_->find(node_id);
	if (iter == db_->end()) {
		std::cerr << "ERROR: EvalServer::handle_pull_log: Failed to find the record of the node" << std::endl;
		return;
	}

	send_pull_log(run_id, iter->second.ip, iter->second.port );
}


void EvalServer::send_config(std::uint32_t node_id, const std::string& ip, unsigned short port) {
	if (!eval_config_) {
		std::cerr << "ERROR: EvalServer::send_config: eval_config_ not initialized" << std::endl;
		return;
	}

	eval_config_->mutable_config()->set_node_id(node_id);
	std::string buffer;
	eval_config_->SerializeToString(&buffer);
	
	if (!tcp_server_) {
		std::cerr << "ERROR: EvalServer::send_config: tcp_server_ initialized" << std::endl;
		return;
	}

	tcp_server_->send(ip, port, buffer);
}

void EvalServer::send_broadcast(std::uint32_t workload_size, const std::string& ip, unsigned short port) {
	BootstrapMessage msg;
	msg.set_type(BootstrapMessage::BROADCAST);
	// msg.mutable_broadcast()->set_node_id(node_id);
	msg.mutable_broadcast()->set_workload_size(workload_size);
	
	std::string buffer;
	msg.SerializeToString(&buffer);
	
	if (!tcp_server_) {
		std::cerr << "ERROR: EvalServer::send_broadcast: tcp_server_ initialized" << std::endl;
		return;
	}

	tcp_server_->send(ip, port, buffer);
}

void EvalServer::send_pull_log(const std::string& run_id, const std::string& ip, unsigned short port) {
	BootstrapMessage msg;
	msg.set_type(BootstrapMessage::PULL_LOG);
	msg.mutable_pull_log()->set_run_id(run_id);
	
	std::string buffer;
	msg.SerializeToString(&buffer);
	
	if (!tcp_server_) {
		std::cerr << "ERROR: EvalServer::send_pull_log: tcp_server_ initialized" << std::endl;
		return;
	}

	tcp_server_->send(ip, port, buffer);
}

int EvalServer::sendBroadcast(void *data, int argc, char **argv, char **azColName){
	uint32 workload_size = *((uint32*) data);
	unsigned short port = std::stoi(argv[1]);

	bootstrap_message::BootstrapMessage msg;
	msg.set_type(bootstrap_message::BootstrapMessage::BROADCAST);

	msg.mutable_broadcast()->set_wordload_size(wordload_size);

	std::string buffer;
	msg.SerializeToString(&buffer);

	tcp_server->send(argv[0], port, buffer);
	return 0;
}

void EvalServer::broadcast(uint32 node_id, uint32 workload_size) {
	char* errMsg;
	int rc;
   	char sql[100];
	sprintf(sql, "SELECT * FROM Node WHERE id = %d", node_id);

	rc = sqlite3_exec(db, sql, sendBroadcast, (void*) &workload_size, &errMsg);
   	if(rc != SQLITE_OK){
		std::cerr << "SQL error: " << errMsg << std::endl;
		num_node++;
   	}
	else{
		std::cout << "Operation done successfully" << std::endl;
   	}
}


int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cerr << "ERROR: main: Illegal number of arguments" << std::endl;
		std::cout << "Usage : ./eval_server port_number log_dir" << std::endl;
		return -1;
	}
	std::shared_ptr<EvalServer> eval_server = std::make_shared<EvalServer>(argv[2]);
	eval_server->run(std::stoi(argv[1]));
	
	// CLI LOOP
	std::string command;
	for (;;) {
		std::cout << "Enter a commnand (\"help\" for hints) >>> ";
		std::cin >> command;

		if (command == "count") {
			std::cout << "Current # of nodes: " << eval_server->handle_count() << std::endl;
		} else if (command == "check") {
			std::size_t node_id;
			std::cin >> node_id;

			NodeRecord r = eval_server->handle_check(node_id);

			std::cout << "Node " << node_id << ":\n\tIP: " << r.ip 
				<< "\n\tPort: " << r.port << std::endl;
		} else if (command == "config") {
			EvalConfig eval_config;
			
			std::cout << "Enter the config (run_id eval_type(0/1) n_dist cn_dist n_city cn_city n_state cn_state n_country cn_country n_continent) >>> ";
			std::cin >> eval_config.run_id >> eval_config.eval_type 
				>> eval_config.num_nodes_in_dist >> eval_config.num_cnodes_in_dist 
				>> eval_config.num_nodes_in_city >> eval_config.num_cnodes_in_city
				>> eval_config.num_nodes_in_state >> eval_config.num_cnodes_in_state
				>> eval_config.num_nodes_in_country >> eval_config.num_cnodes_in_country
				>> eval_config.num_nodes_in_continent;

			eval_server->handle_config(eval_config);
		} else if (command == "broadast") {
			std::size_t node_id;
			std::uint32_t workload_size;
			std::cout << "Enter node_id workload_size >>> ";
			std::cin >> node_id >> workload_size;

			eval_server->handle_broadcast(node_id, workload_size);
		
		} else if (command == "pull_log") {
			std::size_t node_id;
			std::string run_id;
			std::cout << "Enter node_id run_id >>> ";
			std::cin >> node_id >> run_id;
			eval_server->handle_pull_log(node_id, run_id);

		} else if (command == "help") {
			std::cout << "count\t# of registered nodes\ncheck node_id\tinfo of the node\n \
				config run_id eval_type(0/1) n_dist cn_dist n_city cn_city n_state cn_state n_country cn_country n_continent\tconfig the experiment\n \
				broadcast node_id workload_size\tmake node_id broadcast a work_size long message\n \
				pull_log node_id run_id\tmake node_id upload its log of experiment run_id" << std::endl;
		} else {
			std::cout << "Illegal command" << std::endl;
		}
	}
    return 0;
}

#include <iostream>
#include <boost/log/trivial.hpp>

std::unique_ptr<AsyncTCPServer> EvalServer::tcp_server_;


void EvalServer::start(unsigned short port) {
	BOOST_LOG_TRIVIAL(debug) << "TCPTest::start: Allocating tcp_server";
    tcp_server.reset(new AsyncTCPServer(std::static_pointer_cast<Receiver>(this->shared_from_this()), port));
	num_node = 0;

	if (sqlite3_open("test.db", &db)){
		std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
		exit(0);
	}
	else{
		std::cout << "Open database successfully" << std::endl;
	}

	char* errMsg;
   	int rc;
	char* create_sql = "CREATE TABLE Node(ip CHAR(50), port INT, id INT)";
	rc = sqlite3_exec(db, create_sql, printNode, 0, &errMsg);
	if(rc != SQLITE_OK){
		std::cerr << "SQL error: " << errMsg << std::endl;
	}
	else{
		std::cout << "Table created successfully" << std::endl;
	}	

	this->tcp_server->run();
		
	std::string command;
	while(true){
		std::cout << "Input an argument: " << std::endl;
		std::cin >> command;
	
		if(command == "config"){
			config();
		}

		else if (command == "list"){
			char* sql = "SELECT * FROM Node";
			rc = sqlite3_exec(db, sql, printNode, 0, &errMsg);
		   	if(rc != SQLITE_OK){
				std::cerr << "SQL error: " << errMsg << std::endl;
		   	}
			else{
				std::cout << "Operation done successfully" << std::endl;
		   	}
		}

		else if (command == "count"){
			std::cout << "Number of nodes: " << num_node << std::endl;
		}			
	}
}

virtual void EvalServer::receive(const std::string& ip, unsigned short port, const std::string& data) override {
	BOOST_LOG_TRIVIAL(info) << "UDPTest::receive: Packet received from " + ip + ':' + std::to_string(port) + '\n' + data;

	bootstrap_message::BootstrapMessage nodeinfo;	
	nodeinfo.ParseFromString(data);

	if (nodeinfo.type() == bootstrap_message::BootstrapMessage::INIT){
		init(ip, port, nodeinfo);
	}
}

static int EvalServer::printNode(void *NotUsed, int argc, char **argv, char **azColName)
{
	for(int i=0; i<argc; i++){
		std::cout << azColName[i] << " = " << argv[i] << std::endl;
	}
	return 0;
}

void EvalServer::init(const std::string& ip, unsigned short port, const bootstrap_message::BootstrapMessage nodeinfo){
	char* errMsg;
   	int rc;
   	char* sql;
	sprintf(sql, "INSERT INTO Node (ip, port, id) VALUES (%s, %d, %d)", ip, port, num_node+1);

	rc = sqlite3_exec(db, sql, printNode, 0, &errMsg);
   	if(rc != SQLITE_OK){
		std::cerr << "SQL error: " << errMsg << std::endl;
		num_node++;
   	}
	else{
		std::cout << "Operation done successfully" << std::endl;
   	}
}

static int EvalServer::sendConfig(void *data, int argc, char **argv, char **azColName){
	unsigned short port = std::stoi(argv[1]);

	bootstrap_message::BootstrapMessage msg;
	msg.set_type(bootstrap_message::BootstrapMessage::CONFIG);

	msg.mutable_config()->set_run_id("12345");
	msg.mutable_config()->set_eval_type(bootstrap_message::Config::HGFRR);
	msg.mutable_config()->set_node_id(std::stoi(argv[2]));
		
	msg.mutable_config()->set_table_size(2);
	msg.mutable_config()->add_table_ids(0);
	msg.mutable_config()->add_table_ips("192.168.100.2");
	msg.mutable_config()->add_table_ports(30303);
	msg.mutable_config()->add_table_ids(1);
	msg.mutable_config()->add_table_ips("192.168.100.3");
	msg.mutable_config()->add_table_ports(30303);
	msg.mutable_config()->set_num_nodes_in_dist(10);

	std::string buffer;
	msg.SerializeToString(&buffer);

	tcp_server->send(argv[0], port, buffer);
	return 0;
}

void EvalServer::config() {
	char* errMsg;
	int rc;
   	char* sql = "SELECT * FROM Node";
   	char* data = "Send config function called";

	rc = sqlite3_exec(db, sql, sendConfig, (void*)data, &errMsg);
	if(rc != SQLITE_OK){
		std::cerr << "SQL error: " << errMsg << std::endl;
   	}
	else{
		std::cout << "Operation done successfully" << std::endl;
   	}
}


int main(int argc, char* argv[]) {
    BOOST_LOG_TRIVIAL(debug) << "main: Starting with " << argc << " arguments";
    unsigned short port = std::stoi(argv[1]);
    BOOST_LOG_TRIVIAL(debug) << "main: Port number " << port;
    std::shared_ptr<TCPTest> tcp_test(new TCPTest());
    tcp_test->start(port);
    tcp_test->tcp_server->stop();
    return 0;
}

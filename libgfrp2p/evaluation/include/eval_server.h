#ifndef EVAL_SERVER_H
#define EVAL_SERVER_H

#include <string>
#include <unordered_map>
#include <memory>
#include <fstream>

#include <cstring>

#include "transport.h"
#include "bootstrap_message.pb.h"

using bootstrap_message::BootstrapMessage;
using bootstrap_message::Config;
using bootstrap_message::Init;

struct NodeRecord {
	std::string ip;
	unsigned short port;
};

using NodeDatabase = std::unordered_map<std::size_t, NodeRecord>;

struct EvalConfig {
	std::string run_id;

	std::uint32_t eval_type;

	std::uint32_t num_nodes_in_dist;
    std::uint32_t num_cnodes_in_dist;
    std::uint32_t num_nodes_in_city;
    std::uint32_t num_cnodes_in_city;
    std::uint32_t num_nodes_in_state;
    std::uint32_t num_cnodes_in_state;
    std::uint32_t num_nodes_in_country;
    std::uint32_t num_cnodes_in_country;
	std::uint32_t num_nodes_in_continent;

};

class EvalServer: public Receiver, public std::enable_shared_from_this<EvalServer> {
private:
	std::unique_ptr<AsyncTCPServer> tcp_server_;

	std::unique_ptr<BootstrapMessage> eval_config_;

	// Make database thread-safe
	std::mutex mlock_;
	std::unique_ptr<NodeDatabase> db_;

	const std::string log_dir_;

	void send_config(std::uint32_t node_id, const std::string& ip, unsigned short port);
	void send_broadcast(std::uint32_t workload_size, const std::string& ip, unsigned short port);
	void send_pull_log(const std::string& run_id, const std::string& ip, unsigned short port);

public:
	EvalServer(const std::string& log_dir);

	void run(unsigned short port);
	void stop();

	// Network input
	virtual void receive(const std::string& ip, unsigned short port, const std::string& data) override;

	// API
	std::size_t handle_count();
	NodeRecord handle_check(std::size_t node_id);
	void handle_config(const EvalConfig& config);
	void handle_broadcast(std::size_t node_id, std::uint32_t workload_size);
	void handle_pull_log(std::size_t node_id, const std::string& run_id);

};



#endif

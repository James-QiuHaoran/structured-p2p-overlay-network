#ifndef EVAL_CLIENT_H
#define EVAL_CLIENT_H

#include <memory>
#include <thread>
#include <chrono>


#include "node.h"
#include "node_table.h"
#include "peer_manager.h"
#include "transport.h"
#include "utils.h"

#include "bootstrap_message.pb.h"

class EvalClient: public Receiver, public std::enable_shared_from_this<EvalClient> {
private:
    std::shared_ptr<AsyncTCPServer> tcp_server_;
    
    // info to talk to bootstrap
    const unsigned short local_bootstrap_port; 
    const unsigned short local_broadcast_port;
    const std::string bootstrap_server_ip;
    const unsigned short bootstrap_server_port;

    // p2p layer info
    std::unique_ptr<EvalConfig> eval_config_;
    std::unique_ptr< std::unordered_map<std::string, std::pair<std::string, unsigned short>> > node_list_;

    std::shared_ptr<Node> self_;
    std::shared_ptr<NodeTable> node_table_;
    std::shared_ptr<PeerManager> peer_manager_;

    std::string generate_random_workload(unsigned int workload_size);

    void create_hgfr_table(int num_nodes_in_dist, int num_cnodes_in_dist, 
        int num_nodes_in_city, int num_cnodes_in_city, 
        int num_nodes_in_state, int num_cnodes_in_state, 
        int num_nodes_in_country, int num_cnodes_in_country, 
        int num_nodes_in_continent, 
        const std::unordered_map<std::string, std::pair<std::string, unsigned short>>& node_list);

    void send_init();
    void send_push_log();
    // void create_kad();
public:
    EvalClient(unsigned short local_bootstrap_port, unsigned short local_broadcast_port, const std::string& bootstrap_server_ip, unsigned short bootstrap_server_port);

    void receive(const std::string & ip, unsigned short port, const std::string & data) override;

    void run();

    void reset();

};

#endif
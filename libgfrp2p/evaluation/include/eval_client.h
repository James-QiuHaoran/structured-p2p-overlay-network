#ifndef EVAL_CLIENT_H
#define EVAL_CLIENT_H

#include <memory>

#include "node.h"
#include "node_table.h"
#include "peer_manager.h"
#include "transport.h"

#include "bootstrap_message.pb.h"

using bootstrap_message::BootstrapMessage;
using bootstrap_message::Config

class EvalClient: public Receiver, public std::enable_shared_from_this<EvalClient> {
private:
    std::shared_ptr<AsyncTCPServer> tcp_server_;
    
    // info to talk to bootstrap
    const unsigned short local_bootstrap_port; 
    const unsigned short local_broadcast_port;
    const std::string bootstrap_ip;
    const unsigned short bootstrap_port;

    // p2p layer info
    std::shared_ptr<Node> self_;
    std::shared_ptr<NodeTable> node_table_;
    std::shared_ptr<PeerManager> peer_manager_;

    std::string generate_random_workload(unsigned int workload_size);

    void create_hgfr_table(int num_nodes_in_dist, int num_cnodes_in_dist, 
        int num_nodes_in_city, int num_cnodes_in_city, 
        int num_nodes_in_state, int num_cnodes_in_state, 
        int num_nodes_in_country, int num_cnodes_in_country, 
        int num_nodes_in_continent, 
        std::unordered_map<std::pair<std::string, unsigned short>>& node_list);

    void send_init();
    // void create_kad();
public:
    EvalClient(unsigned short self_port, const std::string& bootstrap_ip, unsigned short bootstrap_port);

    void receive(const std::string & ip, unsigned short port, const std::string & data) override;

    void run();

    void reset();

};

#endif
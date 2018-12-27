#include "eval_client.h"


using bootstrap_message::BootstrapMessage;
using bootstrap_message::Config;

EvalClient::EvalClient(unsigned short local_bootstrap_port, unsigned short local_broadcast_port, const std::string& bootstrap_server_ip, unsigned short bootstrap_server_port):
    local_bootstrap_port(local_bootstrap_port), local_broadcast_port(local_broadcast_port), bootstrap_server_ip(bootstrap_server_ip), bootstrap_server_port(bootstrap_server_port)  { }

void EvalClient::receive(const std::string & ip, unsigned short port, const std::string & data) {
    BootstrapMessage msg;
    if (!msg.ParseFromString(data)) {
        std::cerr << "ERROR: EvalClient::receive: Failure to parse message from bootstrap" << std::endl;
        return;
    }

    if (msg.type() == BootstrapMessage::CONFIG) {
        if (msg.config().eval_type() == Config::HGFRR)  {
            if (this->self_) return; // TODO: Resetting mechanism

            // TODO: Convert id to string function
            std::string str_self_id = convert_ID_int_to_string(msg.config().node_id(),
                msg.config().num_nodes_in_dist(), msg.config().num_cnodes_in_dist(), 
                msg.config().num_nodes_in_city(), msg.config().num_cnodes_in_city(), 
                msg.config().num_nodes_in_state(), msg.config().num_cnodes_in_state(), 
                msg.config().num_nodes_in_country(), msg.config().num_cnodes_in_country(), 
                msg.config().num_nodes_in_continent());

            this->self_ = std::make_shared<Node>(str_self_id, "127.0.0.1", local_broadcast_port);
            this->node_table_ = std::make_shared<NodeTable>(str_self_id);

            // Extract the node list from msg
            std::unordered_map<std::string, std::pair<std::string, unsigned short>> node_list;
            for (int i = 0; i < msg.config().table_size(); i++) {
                std::string str_id = convert_ID_int_to_string(msg.config().table_ids(i),
                    msg.config().num_nodes_in_dist(), msg.config().num_cnodes_in_dist(), 
                    msg.config().num_nodes_in_city(), msg.config().num_cnodes_in_city(), 
                    msg.config().num_nodes_in_state(), msg.config().num_cnodes_in_state(), 
                    msg.config().num_nodes_in_country(), msg.config().num_cnodes_in_country(), 
                    msg.config().num_nodes_in_continent());
                node_list[str_id] = std::make_pair(msg.config().table_ips(i), msg.config().table_ports(i));
            }

            create_hgfr_table(msg.config().num_nodes_in_dist(), msg.config().num_cnodes_in_dist(), 
                msg.config().num_nodes_in_city(), msg.config().num_cnodes_in_city(), 
                msg.config().num_nodes_in_state(), msg.config().num_cnodes_in_state(), 
                msg.config().num_nodes_in_country(), msg.config().num_cnodes_in_country(), 
                msg.config().num_nodes_in_continent(), node_list);
            
            this->peer_manager_ = std::make_shared<PeerManager>(self_, node_table_, str_self_id);

            std::cout << "DEBUG: EvalClient::receive: HGFR evaluation configured, info:" << std::endl;
            for (const auto& table : this->node_table_->get_tables()) {
                std::cout << "DEBUG: EvalClient::receive: \tLevel " << table.ring_level << " has "  << table.peer_list.size() << " peers and " << table.contact_nodes.size() << "contact nodes" << std::endl;
            }
        } else if (false) {
            /* TODO: handle other messages*/
        }
    } else if (msg.type() == BootstrapMessage::BROADCAST) {
        if(!peer_manager_) {
            std::cerr << "ERROR: EvalClient::receive: Broadcast received before configuration" << std::endl;
            return;
        }

        std::cout << "DEBUG: EvalClient::receive: Handling requested broadcast" << std::endl;
        peer_manager_->broadcast(generate_random_workload(msg.broadcast().workload_size()));
    } else if (msg.type() == BootstrapMessage::PULL_LOG) {

        std::cout << "DEBUG: EvalClient::receive: Pushing required push" << std::endl;
        send_push_log();
    } else {
        std::cerr << "ERROR: EvalClient::receive: Unknown message type received from bootstrap" << std::endl;
        return;
    }
}

std::string EvalClient::generate_random_workload(unsigned int workload_size) {
    return std::string(workload_size, 'a');
}


void EvalClient::create_hgfr_table(
    int num_nodes_in_dist, int num_cnodes_in_dist, 
    int num_nodes_in_city, int num_cnodes_in_city, 
    int num_nodes_in_state, int num_cnodes_in_state, 
    int num_nodes_in_country, int num_cnodes_in_country, 
    int num_nodes_in_continent, 
    const std::unordered_map<std::string, std::pair<std::string, unsigned short>>& node_list) {
    
    std::string id_in_dist = this->self_->get_id().substr(ID_SINGLE_START, ID_SINGLE_LEN);
    std::string dist_id = this->self_->get_id().substr(ID_DISTRICT_START, ID_DISTRICT_LEN);
    std::string city_id = this->self_->get_id().substr(ID_CITY_START, ID_CITY_LEN);
    std::string state_id = this->self_->get_id().substr(ID_STATE_START, ID_STATE_LEN);
    std::string country_id = this->self_->get_id().substr(ID_COUNTRY_START, ID_COUNTRY_LEN);
    std::string continent_id = this->self_->get_id().substr(ID_CONTINENT_START, ID_CONTINENT_LEN);

    std::stringstream ss_node(id_in_dist);
    int node_id_in_dist = 0;
    ss_node >> node_id_in_dist;

    int num_dists_in_city = num_nodes_in_city/num_cnodes_in_dist;
    int num_cities_in_state = num_nodes_in_state/num_cnodes_in_dist;

    std::vector<Ring> tables;
    std::unordered_map<std::string, std::shared_ptr<Node>> contact_nodes;  // contact nodes of the ring
    std::shared_ptr<Node> predecessor;                                     // successor within the ring
    std::shared_ptr<Node> successor;                                       // predecessor within the ring
    std::unordered_map<std::string, std::shared_ptr<Node>> peer_set;       // peers in an unordered map
    std::vector<std::shared_ptr<Node>> peer_list;                          // used for broadcast within ring

    // used locally for next level ring's structure establishment
    std::vector<std::shared_ptr<Node>> peer_list_next;
    std::unordered_map<std::string, std::shared_ptr<Node>> contact_nodes_next;

    // contact nodes vector - help with the node assignment
    std::vector<std::shared_ptr<Node>> contact_nodes_list;

    // normal nodes - peer level
    Ring table_peer;
    table_peer.ring_level = 0;

    // add other peers
    std::stringstream ss;
    for (int i = 0; i < num_nodes_in_dist; i++) {
        ss.str("");
        ss.clear();
        ss << std::setw(9) << std::setfill('0') << i;
        std::string peer_id_in_dist = ss.str();
        std::string node_id = this->self_->get_id().substr(0, ID_SINGLE_START) + peer_id_in_dist;
        
        auto iter = node_list.find(node_id);
        if (iter == node_list.end()) {
            std::cerr << "ERROR: EvalClient::create_hgfr_table: Cannot find broadcast ip and port of " << node_id << std::endl;
            continue;
        }
        auto node = std::make_shared<Node>(node_id, iter->second.first, iter->second.second);

        // insert into contact node list
        if (i < num_cnodes_in_dist) {
            contact_nodes.insert({node_id, node});
            contact_nodes_list.push_back(node);
        }

        // check predecessor or successor
        if (node_id_in_dist == 0 && i == num_nodes_in_dist - 1) {
            predecessor = node;
        } else if (node_id_in_dist == num_nodes_in_dist - 1 && i == 0) {
            successor = node;
        } else if (i == node_id_in_dist + 1) {
            successor = node;
        } else if (i == node_id_in_dist - 1) {
            predecessor = node;
        }

        // insert into peer list
        peer_set.insert({node_id, node});
        peer_list.push_back(node);
    }

    table_peer.contact_nodes = contact_nodes;
    table_peer.predecessor = predecessor;
    table_peer.successor = successor;
    table_peer.peer_set = peer_set;
    table_peer.peer_list = peer_list;

    tables.push_back(table_peer);
    
    // contact nodes - dist level
    if (contact_nodes.find(this->self_->get_id()) != contact_nodes.end()) {
        // should be the contact node of the dist level ring
        Ring table_dist;
        table_dist.ring_level = 1;

        peer_set.clear();
        peer_list.clear();

        std::string node_id_until_city = this->self_->get_id().substr(0, ID_DISTRICT_START);
        int j = 0;
        for (int i = 0; i < num_dists_in_city; i++) {
            ss.str("");
            ss.clear();
            ss << std::setw(5) << std::setfill('0') << i;
            std::string dist_id_in_city = ss.str();
            std::string node_id_until_dist = node_id_until_city + dist_id_in_city;
            for (auto contact_node : contact_nodes_list) {
                std::string node_id = node_id_until_dist + contact_node->get_id().substr(ID_SINGLE_START, ID_SINGLE_LEN);

                auto iter = node_list.find(node_id);
                if (iter == node_list.end()) {
                    std::cerr << "ERROR: EvalClient::create_hgfr_table: Cannot find broadcast ip and port of " << node_id << std::endl;
                    continue;
                }
        
                auto node = std::make_shared<Node>(node_id, iter->second.first, iter->second.second);
        
                peer_list.push_back(node);
                peer_set.insert({node_id, node});

                if (j < num_cnodes_in_city) {
                    contact_nodes_next.insert({node_id, node});
                    j++;
                }
            }
        }

        table_dist.contact_nodes = contact_nodes_next;
        // table_dist.predecessor = NULL;
        // table_dist.successor = NULL;
        table_dist.peer_set = peer_set;
        table_dist.peer_list = peer_list;

        tables.push_back(table_dist);

        // exchange contact_nodes and contact_nodes_next
        contact_nodes.clear();
        contact_nodes_list.clear();
        for (auto contact_node : contact_nodes_next) {
            contact_nodes.insert(contact_node);
            contact_nodes_list.push_back(contact_node.second);
        }
        contact_nodes_next.clear();
    }

    // contact nodes - city level
    if (contact_nodes.find(this->self_->get_id()) != contact_nodes.end()) {
        // should be the contact node of the city level ring
        Ring table_city;
        table_city.ring_level = 2;

        // reset stringstream and ring variables
        peer_set.clear();
        peer_list.clear();

        std::string node_id_until_state = this->self_->get_id().substr(0, ID_CITY_START);
        int j = 0;
        for (int i = 0; i < num_cities_in_state; i++) {
            ss.str("");
            ss.clear();
            ss << std::setw(6) << std::setfill('0') << i;
            std::string city_id_in_state = ss.str();
            std::string node_id_until_city = node_id_until_state + city_id_in_state;
            for (auto contact_node : contact_nodes_list) {
                std::string node_id = node_id_until_city + contact_node->get_id().substr(ID_DISTRICT_START);

                auto iter = node_list.find(node_id);
                if (iter == node_list.end()) {
                    std::cerr << "ERROR: EvalClient::create_hgfr_table: Cannot find broadcast ip and port of " << node_id << std::endl;
                    continue;
                }
        
                auto node = std::make_shared<Node>(node_id, iter->second.first, iter->second.second);     

                peer_list.push_back(node);
                peer_set.insert({node_id, node});

                if (j < num_cnodes_in_state) {
                    contact_nodes_next.insert({node_id, node});
                    j++;
                }
            }
        }

        table_city.contact_nodes = contact_nodes_next;
        // table_city.predecessor = NULL;
        // table_city.successor = NULL;
        table_city.peer_set = peer_set;
        table_city.peer_list = peer_list;

        tables.push_back(table_city);
    }

    this->node_table_->set_tables(tables);
}

void EvalClient::send_init() {
    
    // Send init message
    BootstrapMessage msg;
    msg.set_type(BootstrapMessage::INIT);
    msg.mutable_init()->set_port(local_broadcast_port);
    
    std::string serialized;
    if (!msg.SerializeToString(&serialized)) {
        std::cerr << "ERROR: EvalClient::send_init: Failed to serialize init message" << std::endl;
        return;
    }

    if (!tcp_server_) {
        std::cerr << "ERROR: EvalClient::send_init: TCP Server not configured" << std::endl;
        return;
    }

    std::cout << "INFO: EvalClient::send_init: Sending INIT message to bootstrap" << std::endl;
    tcp_server_->send(bootstrap_server_ip, bootstrap_server_port, serialized);
}

void EvalClient::send_push_log() {
    BootstrapMessage msg;
    msg.set_type(BootstrapMessage::PUSH_LOG);
    msg.mutable_push_log()->set_run_id(peer_manager_->get_run_id());
    msg.mutable_push_log()->set_log(peer_manager_->get_all_records_csv());

    std::string serialized;
    if (!msg.SerializeToString(&serialized)) {
        std::cerr << "ERROR: EvalClient::send_push_log: Failed to serialize init message" << std::endl;
        return;
    }

    if (!tcp_server_) {
        std::cerr << "ERROR: EvalClient::send_push_log: TCP Server not configured" << std::endl;
        return;
    }

    tcp_server_->send(bootstrap_server_ip, bootstrap_server_port, serialized);
        
}

void EvalClient::run() {
    tcp_server_ = std::make_shared<AsyncTCPServer>(std::static_pointer_cast<Receiver>(shared_from_this()), local_bootstrap_port);
    tcp_server_->run();

    send_init();    
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Illegal number" << std::endl;
        std::cout << "Usage e.g.: ./eval_client local_bootstrap_port local_broadcast_port bootstrap_server_ip bootstrap_server_port" << std::endl;
    }
    std::shared_ptr<EvalClient> eval_client = std::make_shared<EvalClient>(std::stoi(argv[1]), std::stoi(argv[2]), argv[3], std::stoi(argv[4]));

    eval_client->run();
    
    std::this_thread::sleep_for(std::chrono::hours(24));
    return 0;

}
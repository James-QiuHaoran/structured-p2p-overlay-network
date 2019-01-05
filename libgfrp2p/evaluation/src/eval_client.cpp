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
        
        std::cout << "DEBUG: EvalClient::receive: Config received" << std::endl;
        if (this->eval_config_) {
            // Reset
            std::cout << "DEBUG: EvalClient::receive: Resetting config" << std::endl;
            if (eval_config_->eval_type) {
                peer_manager_eth_.reset();
                node_table_eth_.reset();
            } else {
                peer_manager_.reset();
                node_table_.reset();
            }
            self_.reset();
        } 
        eval_config_.reset(new EvalConfig);
        
        eval_config_->str_node_id = convert_ID_int_to_string(msg.config().node_id(),
            msg.config().num_nodes_in_dist(), msg.config().num_cnodes_in_dist(), 
            msg.config().num_nodes_in_city(), msg.config().num_cnodes_in_city(), 
            msg.config().num_nodes_in_state(), msg.config().num_cnodes_in_state(), 
            msg.config().num_nodes_in_country(), msg.config().num_cnodes_in_country(), 
            msg.config().num_nodes_in_continent());

        eval_config_->run_id = msg.config().run_id();
        
        eval_config_->eval_type = (msg.config().eval_type() == Config::HGFRR) ? 0 : 1;
        
        eval_config_->num_nodes_in_dist = msg.config().num_nodes_in_dist();
        eval_config_->num_cnodes_in_dist = msg.config().num_cnodes_in_dist();
        eval_config_->num_nodes_in_city = msg.config().num_nodes_in_city();
        eval_config_->num_cnodes_in_city = msg.config().num_cnodes_in_city();
        eval_config_->num_nodes_in_state = msg.config().num_nodes_in_state();
        eval_config_->num_cnodes_in_state = msg.config().num_cnodes_in_state();
        eval_config_->num_nodes_in_country = msg.config().num_nodes_in_country();
        eval_config_->num_cnodes_in_country = msg.config().num_cnodes_in_country();
        eval_config_->num_nodes_in_continent = msg.config().num_nodes_in_continent();
        eval_config_->num_cnodes_in_continent = msg.config().num_cnodes_in_continent();
        eval_config_->num_continents = msg.config().num_continents();
    
        
        this->self_ = std::make_shared<Node>(eval_config_->str_node_id, "127.0.0.1", local_broadcast_port);

        std::cout << "DEBUG: EvalClient::receive: Config done" << std::endl;
    } else if (msg.type() == BootstrapMessage::TABLE) {
        if (!this->eval_config_) {
            std::cerr << "ERROR: EvalClient::receive: Table received but evaluation not configured" << std::endl;
            return;
        }
        if (!this->node_list_) {        
            std::cout << "DEBUG: EvalClient::receive: Creating node table" << std::endl;
            node_list_.reset(new std::unordered_map<std::string, std::pair<std::string, unsigned short>>());    
        } else {
            std::cout << "DEBUG: EvalClient::receive: Appending node table" << std::endl;  
        }
        // Extract the node list from msg           
        for (size_t i = 0; i < msg.table().table_size(); i++) {
            std::string str_id = convert_ID_int_to_string(msg.table().table_ids(i),
                eval_config_->num_nodes_in_dist, eval_config_->num_cnodes_in_dist, 
                eval_config_->num_nodes_in_city, eval_config_->num_cnodes_in_city, 
                eval_config_->num_nodes_in_state, eval_config_->num_cnodes_in_state, 
                eval_config_->num_nodes_in_country, eval_config_->num_cnodes_in_country, 
                eval_config_->num_nodes_in_continent);
            (*node_list_)[str_id] = std::make_pair(msg.table().table_ips(i), msg.table().table_ports(i));
        }

        // Paging finished
        if (msg.table().is_end()) {
            std::cout << "DEBUG: EvalClient::receive: Last page received, creating evaluation" << std::endl;
            mkdir(("../test/log/" + eval_config_->run_id + '/').c_str(), S_IRWXU);
            if (eval_config_->eval_type) {
                this->node_table_eth_ = std::make_shared<NodeTableETH>(eval_config_->str_node_id);
                create_kad_table(eval_config_->num_nodes_in_dist, eval_config_->num_cnodes_in_dist, 
                    eval_config_->num_nodes_in_city, eval_config_->num_cnodes_in_city, 
                    eval_config_->num_nodes_in_state, eval_config_->num_cnodes_in_state, 
                    eval_config_->num_nodes_in_country, eval_config_->num_cnodes_in_country, 
                    eval_config_->num_nodes_in_continent, eval_config_->num_cnodes_in_continent,
                    eval_config_->num_continents, *node_list_);
                    
                std::cout << "DEBUG: EvalClient::receive: Kademlia table created" << std::endl;
                this->peer_manager_eth_ = std::make_shared<PeerManagerETH>(self_, node_table_eth_, eval_config_->run_id);
                this->peer_manager_eth_->set_mode(PeerManagerETH::PUSH);
                this->peer_manager_eth_->start();
                std::cout << "DEBUG: EvalClient::receive: Kademlia evaluation configured" << std::endl;  

            } else {
                this->node_table_ = std::make_shared<NodeTable>(eval_config_->str_node_id);
                create_hgfr_table(eval_config_->num_nodes_in_dist, eval_config_->num_cnodes_in_dist, 
                    eval_config_->num_nodes_in_city, eval_config_->num_cnodes_in_city, 
                    eval_config_->num_nodes_in_state, eval_config_->num_cnodes_in_state, 
                    eval_config_->num_nodes_in_country, eval_config_->num_cnodes_in_country, 
                    eval_config_->num_nodes_in_continent, eval_config_->num_cnodes_in_continent,
                    eval_config_->num_continents, *node_list_);
                    
                std::cout << "DEBUG: EvalClient::receive: HGFR table created" << std::endl;
                this->peer_manager_ = std::make_shared<PeerManager>(self_, node_table_, eval_config_->run_id);
                this->peer_manager_->start();
                              
                std::cout << "DEBUG: EvalClient::receive: HGFR evaluation configured, info:" << std::endl;
                for (const auto& table : this->node_table_->get_tables()) {
                    std::cout << "DEBUG: EvalClient::receive: \tLevel " << table.ring_level << " has "  << table.peer_list.size() << " peers and " << table.contact_nodes.size() << " contact nodes" << std::endl;
                }
            }
        }

    } else if (msg.type() == BootstrapMessage::BROADCAST) {
        if(!peer_manager_ && !peer_manager_eth_) {
            std::cerr << "ERROR: EvalClient::receive: Broadcast received before configuration" << std::endl;
            return;
        }

        std::cout << "DEBUG: EvalClient::receive: Handling requested broadcast" << std::endl;
        if (eval_config_->eval_type) {
            // eth
            peer_manager_eth_->broadcast(generate_random_workload(msg.broadcast().workload_size()), TTL_ETH, "");
        } else {
            peer_manager_->broadcast(generate_random_workload(msg.broadcast().workload_size()));
        }
    } else if (msg.type() == BootstrapMessage::PULL_LOG) {
        unsigned int ms = rand() % 30000 + 3000;
        std::cout << "DEBUG: EvalClient::receive: Pushing log after " << ms << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        send_push_log();
    } else {
        std::cerr << "ERROR: EvalClient::receive: Unknown message type received from bootstrap" << std::endl;
        return;
    }
}

std::string EvalClient::generate_random_workload(unsigned int workload_size) {
    return std::string(workload_size, 'S');
}


void EvalClient::create_hgfr_table(
    int num_nodes_in_dist, int num_cnodes_in_dist, 
    int num_nodes_in_city, int num_cnodes_in_city, 
    int num_nodes_in_state, int num_cnodes_in_state, 
    int num_nodes_in_country, int num_cnodes_in_country, 
    int num_nodes_in_continent, int num_cnodes_in_continent,
    int num_continents,
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
    int num_states_in_country = num_nodes_in_country/num_cnodes_in_state;
    int num_countries_in_continent = num_nodes_in_continent/num_cnodes_in_country;

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
    

    // contact nodes - state level
    if (num_nodes_in_country == 0) {
        this->node_table_->set_tables(tables);
        return;
    }
    if (contact_nodes.find(this->self_->get_id()) != contact_nodes.end()) {
        // should be the contact node of the state level ring
        Ring table_state;
        table_state.ring_level = 3;

        // reset stringstream and ring variables
        peer_set.clear();
        peer_list.clear();

        std::string node_id_until_country = this->self_->get_id().substr(0, ID_STATE_START);
        int j = 0;
        for (int i = 0; i < num_states_in_country; i++) {
            ss.str("");
            ss.clear();
            ss << std::setw(ID_STATE_LEN) << std::setfill('0') << i;
            std::string state_id_in_country = ss.str();
            std::string node_id_until_state = node_id_until_country + state_id_in_country;
            for (auto contact_node : contact_nodes_list) {
                std::string node_id = node_id_until_state + contact_node->get_id().substr(ID_CITY_START);

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

        table_state.contact_nodes = contact_nodes_next;
        // table_state.predecessor = NULL;
        // table_state.successor = NULL;
        table_state.peer_set = peer_set;
        table_state.peer_list = peer_list;

        tables.push_back(table_state);

        // exchange contact_nodes and contact_nodes_next
        contact_nodes.clear();
        contact_nodes_list.clear();
        for (auto contact_node : contact_nodes_next) {
            contact_nodes.insert(contact_node);
            contact_nodes_list.push_back(contact_node.second);
        }
        contact_nodes_next.clear();
    }

    // contact nodes - country level
    if (num_nodes_in_continent == 0) {
        this->node_table_->set_tables(tables);
        return;
    }
    if (contact_nodes.find(self_->get_id()) != contact_nodes.end()) {
        // should be the contact node of the country level ring
        Ring table_country;
        table_country.ring_level = 4;

        // reset stringstream and ring variables
        peer_set.clear();
        peer_list.clear();

        std::string node_id_until_continent = self_->get_id().substr(0, ID_COUNTRY_START);
        int j = 0;
        for (int i = 0; i < num_countries_in_continent; i++) {
            ss.str("");
            ss.clear();
            ss << std::setw(ID_COUNTRY_LEN) << std::setfill('0') << i;
            std::string country_id_in_continent = ss.str();
            std::string node_id_until_country = node_id_until_continent + country_id_in_continent;
            for (auto contact_node : contact_nodes_list) {
                std::string node_id = node_id_until_country + contact_node->get_id().substr(ID_STATE_START);

                auto iter = node_list.find(node_id);
                if (iter == node_list.end()) {
                    std::cerr << "ERROR: EvalClient::create_hgfr_table: Cannot find broadcast ip and port of " << node_id << std::endl;
                    continue;
                }
                auto node = std::make_shared<Node>(node_id, iter->second.first, iter->second.second);

                peer_list.push_back(node);
                peer_set.insert({node_id, node});

                if (j < num_cnodes_in_country) {
                    contact_nodes_next.insert({node_id, node});
                    j++;
                }
            }
        }

        table_country.contact_nodes = contact_nodes_next;
        // table_country.predecessor = NULL;
        // table_country.successor = NULL;
        table_country.peer_set = peer_set;
        table_country.peer_list = peer_list;

        tables.push_back(table_country);

        // exchange contact_nodes and contact_nodes_next
        contact_nodes.clear();
        contact_nodes_list.clear();
        for (auto contact_node : contact_nodes_next) {
            contact_nodes.insert(contact_node);
            contact_nodes_list.push_back(contact_node.second);
        }
        contact_nodes_next.clear();
    }

    // contact nodes - continent level
    if (contact_nodes.find(self_->get_id()) != contact_nodes.end()) {
        // should be the contact node of the country level ring
        Ring table_continent;
        table_continent.ring_level = 5;

        // reset stringstream and ring variables
        peer_set.clear();
        peer_list.clear();

        int j = 0;
        for (int i = 0; i < num_continents; i++) {
            ss.str("");
            ss.clear();
            ss << std::setw(ID_CONTINENT_LEN) << std::setfill('0') << i;
            std::string node_id_until_continent = ss.str();
            for (auto contact_node : contact_nodes_list) {
                std::string node_id = node_id_until_continent + contact_node->get_id().substr(ID_COUNTRY_START);

                auto iter = node_list.find(node_id);
                if (iter == node_list.end()) {
                    std::cerr << "ERROR: EvalClient::create_hgfr_table: Cannot find broadcast ip and port of " << node_id << std::endl;
                    continue;
                }
                auto node = std::make_shared<Node>(node_id, iter->second.first, iter->second.second);

                peer_list.push_back(node);
                peer_set.insert({node_id, node});

                if (j < num_cnodes_in_continent) {
                    contact_nodes_next.insert({node_id, node});
                    j++;
                }
            }
        }

        table_continent.contact_nodes = contact_nodes_next;
        // table_continent.predecessor = NULL;
        // table_continent.successor = NULL;
        table_continent.peer_set = peer_set;
        table_continent.peer_list = peer_list;

        tables.push_back(table_continent);
    }


    this->node_table_->set_tables(tables);
}

void EvalClient::create_kad_table(
    int num_nodes_in_dist, int num_cnodes_in_dist, 
    int num_nodes_in_city, int num_cnodes_in_city, 
    int num_nodes_in_state, int num_cnodes_in_state, 
    int num_nodes_in_country, int num_cnodes_in_country, 
    int num_nodes_in_continent, int num_cnodes_in_continent,
    int num_continents,
    const std::unordered_map<std::string, std::pair<std::string, unsigned short>>& node_list) {

    
    // form network topology based ID
    std::string id_in_dist = self_->get_id().substr(ID_SINGLE_START, ID_SINGLE_LEN);
    std::string dist_id = self_->get_id().substr(ID_DISTRICT_START, ID_DISTRICT_LEN);
    std::string city_id = self_->get_id().substr(ID_CITY_START, ID_CITY_LEN);
    std::string state_id = self_->get_id().substr(ID_STATE_START, ID_STATE_LEN);
    std::string country_id = self_->get_id().substr(ID_COUNTRY_START, ID_COUNTRY_LEN);
    std::string continent_id = self_->get_id().substr(ID_CONTINENT_START, ID_CONTINENT_LEN);

    int num_dists_in_city, num_cities_in_state, num_states_in_country, num_countries_in_continent;
    
    num_dists_in_city = num_nodes_in_city/num_cnodes_in_dist;
    if (num_dists_in_city == 0)
        num_dists_in_city = 1;
    num_cities_in_state = num_nodes_in_state/num_cnodes_in_city;
    if (num_cities_in_state == 0)
        num_cities_in_state = 1;
    num_states_in_country = num_nodes_in_country/num_cnodes_in_state;
    if (num_states_in_country == 0)
        num_states_in_country = 1;
    num_countries_in_continent = num_nodes_in_continent/num_cnodes_in_country;
    if (num_countries_in_continent == 0)
        num_countries_in_continent = 1;

    int num_nodes_total = num_continents * num_countries_in_continent * num_states_in_country * num_cities_in_state * num_dists_in_city * num_nodes_in_dist;
    
    // set node table
    std::vector<std::shared_ptr<Node>> table;

    // generate random neighbors to connect
    std::unordered_set<int> neighbor_ids;

    int self_order = convert_ID_string_to_int(this->self_->get_id(),
                                                num_nodes_in_dist, num_cnodes_in_dist, 
                                                num_nodes_in_city, num_cnodes_in_city, 
                                                num_nodes_in_state, num_cnodes_in_state, 
                                                num_nodes_in_country, num_cnodes_in_country, 
                                                num_nodes_in_continent);
    
    neighbor_ids.insert((self_order+1) % num_nodes_total);

    for (int i = 0; i < TABLE_SIZE_ETH; i++) {
        int id = rand() % num_nodes_total;
        if (neighbor_ids.find(id) == neighbor_ids.end() && id != self_order) {
            neighbor_ids.insert(id);
        } else {
            i--;
        }
    }

    // int to string
    std::stringstream ss;

    int counter = 0;
    for (int continent_counter = 0; continent_counter < num_continents; continent_counter++) {
        ss.str("");
        ss.clear();
        ss << std::setw(ID_CONTINENT_LEN) << std::setfill('0') << continent_counter;
        continent_id = ss.str();
        for (int country_counter = 0; country_counter < num_countries_in_continent; country_counter++) {
            ss.str("");
            ss.clear();
            ss << std::setw(ID_COUNTRY_LEN) << std::setfill('0') << country_counter;
            country_id = ss.str();
            for (int state_counter = 0; state_counter < num_states_in_country; state_counter++) {
                ss.str("");
                ss.clear();
                ss << std::setw(ID_STATE_LEN) << std::setfill('0') << state_counter;
                state_id = ss.str();
                for (int city_counter = 0; city_counter < num_cities_in_state; city_counter++) {
                    ss.str("");
                    ss.clear();
                    ss << std::setw(ID_CITY_LEN) << std::setfill('0') << city_counter;
                    city_id = ss.str();
                    for (int district_counter = 0; district_counter < num_dists_in_city; district_counter++) {
                        ss.str("");
                        ss.clear();
                        ss << std::setw(ID_DISTRICT_LEN) << std::setfill('0') << district_counter;
                        dist_id = ss.str();
                        for (int i = 0; i < num_nodes_in_dist; i++) {
                            ss.str("");
                            ss.clear();
                            ss << std::setw(ID_SINGLE_LEN) << std::setfill('0') << i;
                            id_in_dist = ss.str();
                            std::string node_id = continent_id + country_id + state_id + city_id + dist_id + id_in_dist;
                            int order = convert_ID_string_to_int(node_id,
                                                                    num_nodes_in_dist, num_cnodes_in_dist, 
                                                                    num_nodes_in_city, num_cnodes_in_city, 
                                                                    num_nodes_in_state, num_cnodes_in_state, 
                                                                    num_nodes_in_country, num_cnodes_in_country, 
                                                                    num_nodes_in_continent);

                            if (neighbor_ids.find(order) != neighbor_ids.end() && node_id != this->self_->get_id()) {
                                        
                                auto iter = node_list.find(node_id);
                                if (iter == node_list.end()) {
                                    std::cerr << "ERROR: EvalClient::create_lad_table: Cannot find broadcast ip and port of " << node_id << std::endl;
                                    continue;
                                }
                                auto node = std::make_shared<Node>(node_id, iter->second.first, iter->second.second);

                                // unsigned short port = starting_port_number + order;
                                // auto node = std::make_shared<Node>(node_id, "127.0.0.1", port);
                                if (order == self_order+1)
                                    table.insert(table.begin(), node);
                                else
                                    table.push_back(node);
                            }
                            counter++;
                        }
                    }
                }
            }
        }
    }

    this->node_table_eth_->set_table(table);

    return;

}


void EvalClient::send_init() {
    
    // Send init message
    BootstrapMessage msg;
    msg.set_type(BootstrapMessage::INIT);
    msg.mutable_init()->set_bootstrap_port(local_bootstrap_port);
    msg.mutable_init()->set_broadcast_port(local_broadcast_port);
    
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
    msg.mutable_push_log()->set_run_id(eval_config_->eval_type ? peer_manager_eth_->get_run_id() : peer_manager_->get_run_id());
    msg.mutable_push_log()->set_node_id(self_->get_id());

    std::stringstream ss(eval_config_->eval_type ? peer_manager_eth_->get_all_records_csv() : peer_manager_->get_all_records_csv());
    std::string line, buffer;
    std::size_t counter = 0;
    while (std::getline(ss, line)) {
        buffer += (line + '\n');
        counter++;

        if (counter % 200 == 0) {
            // Send
            msg.mutable_push_log()->set_log(buffer);
            std::string serialized;
            if (!msg.SerializeToString(&serialized)) {
                std::cerr << "ERROR: EvalClient::send_push_log: Failed to serialize init message" << std::endl;
                return;
            }

            if (!tcp_server_) {
                std::cerr << "ERROR: EvalClient::send_push_log: TCP Server not configured" << std::endl;
                return;
            }
            std::cout << "DEBUG: EvalClient::send_push_log: Sending accumulative " << counter << " records" << std::endl;
            tcp_server_->send(bootstrap_server_ip, bootstrap_server_port, serialized);
            buffer.clear();
            std::this_thread::sleep_for(std::chrono::microseconds(300));
        } 
    }

    if (counter % 200 != 0) {
        msg.mutable_push_log()->set_log(buffer);
        std::string serialized;
        if (!msg.SerializeToString(&serialized)) {
            std::cerr << "ERROR: EvalClient::send_push_log: Failed to serialize init message" << std::endl;
            return;
        }

        if (!tcp_server_) {
            std::cerr << "ERROR: EvalClient::send_push_log: TCP Server not configured" << std::endl;
            return;
        }

        std::cout << "DEBUG: EvalClient::send_push_log: Sending accumulative " << counter << " records" << std::endl;
        tcp_server_->send(bootstrap_server_ip, bootstrap_server_port, serialized);
    }
    std::cout << "DEBUG: EvalClient::send_push_log: Records sent" << std::endl;
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
        return 1;
    }
    std::shared_ptr<EvalClient> eval_client = std::make_shared<EvalClient>(std::stoi(argv[1]), std::stoi(argv[2]), argv[3], std::stoi(argv[4]));

    eval_client->run();
    
    std::this_thread::sleep_for(std::chrono::hours(24));
    return 0;

}

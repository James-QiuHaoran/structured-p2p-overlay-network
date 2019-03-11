#include "app.h"

// constructors
BaseApp::BaseApp(std::string ip, unsigned short port, std::string id) {
    this->node = std::make_shared<Node>(id, ip, port);
    this->node_table = std::make_shared<NodeTable>(id);
}

// getters
std::shared_ptr<Node> BaseApp::get_node() {
    return this->node;
}

std::shared_ptr<NodeTable> BaseApp::get_node_table() {
    return this->node_table;
}

std::shared_ptr<PeerManager> BaseApp::get_peer_manager() {
    return this->peer_manager;
}

// public functions
void BaseApp::form_structure(int num_nodes_in_dist, int num_cnodes_in_dist, 
        int num_nodes_in_city, int num_cnodes_in_city, 
        int num_nodes_in_state, int num_cnodes_in_state, 
        int num_nodes_in_country, int num_cnodes_in_country, 
        int num_nodes_in_continent, int num_continents,
        int num_cnodes_in_continent,
        unsigned short starting_port_number) {
    // form network topology based ID
    std::string id_in_dist = this->node->get_id().substr(ID_SINGLE_START, ID_SINGLE_LEN);
    std::string dist_id = this->node->get_id().substr(ID_DISTRICT_START, ID_DISTRICT_LEN);
    std::string city_id = this->node->get_id().substr(ID_CITY_START, ID_CITY_LEN);
    std::string state_id = this->node->get_id().substr(ID_STATE_START, ID_STATE_LEN);
    std::string country_id = this->node->get_id().substr(ID_COUNTRY_START, ID_COUNTRY_LEN);
    std::string continent_id = this->node->get_id().substr(ID_CONTINENT_START, ID_CONTINENT_LEN);

    std::stringstream ss_node(id_in_dist);
    int node_id_in_dist = 0;
    ss_node >> node_id_in_dist;

    int num_dists_in_city = num_nodes_in_city/num_cnodes_in_dist;
    int num_cities_in_state = num_nodes_in_state/num_cnodes_in_city;
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
        ss << std::setw(ID_SINGLE_LEN) << std::setfill('0') << i;
        std::string peer_id_in_dist = ss.str();
        std::string node_id = this->node->get_id().substr(0, ID_SINGLE_START) + peer_id_in_dist;
        unsigned short port = starting_port_number + convert_ID_string_to_int(node_id,
                                                        num_nodes_in_dist, num_cnodes_in_dist, 
                                                        num_nodes_in_city, num_cnodes_in_city, 
                                                        num_nodes_in_state, num_cnodes_in_state, 
                                                        num_nodes_in_country, num_cnodes_in_country, 
                                                        num_nodes_in_continent);
        Node node(node_id, "127.0.0.1", port);

        // insert into contact node list
        if (i < num_cnodes_in_dist) {
            contact_nodes.insert({node_id, std::make_shared<Node>(node)});
            contact_nodes_list.push_back(std::make_shared<Node>(node));
        }

        // check predecessor or successor
        if (node_id_in_dist == 0 && i == num_nodes_in_dist - 1) {
            predecessor = std::make_shared<Node>(node);
        } else if (node_id_in_dist == num_nodes_in_dist - 1 && i == 0) {
            successor = std::make_shared<Node>(node);
        } else if (i == node_id_in_dist + 1) {
            successor = std::make_shared<Node>(node);
        } else if (i == node_id_in_dist - 1) {
            predecessor = std::make_shared<Node>(node);
        }

        // insert into peer list
        peer_set.insert({node_id, std::make_shared<Node>(node)});
        peer_list.push_back(std::make_shared<Node>(node));
    }

    table_peer.contact_nodes = contact_nodes;
    table_peer.predecessor = predecessor;
    table_peer.successor = successor;
    table_peer.peer_set = peer_set;
    table_peer.peer_list = peer_list;

    tables.push_back(table_peer);
    
    // contact nodes - dist level
    if (num_nodes_in_city == 0) {
        this->node_table->set_tables(tables);
        return;
    }
    if (contact_nodes.find(this->node->get_id()) != contact_nodes.end()) {
        // should be the contact node of the dist level ring
        Ring table_dist;
        table_dist.ring_level = 1;

        peer_set.clear();
        peer_list.clear();

        std::string node_id_until_city = this->node->get_id().substr(0, ID_DISTRICT_START);
        int j = 0;
        for (int i = 0; i < num_dists_in_city; i++) {
            ss.str("");
            ss.clear();
            ss << std::setw(ID_DISTRICT_LEN) << std::setfill('0') << i;
            std::string dist_id_in_city = ss.str();
            std::string node_id_until_dist = node_id_until_city + dist_id_in_city;
            for (auto contact_node : contact_nodes_list) {
                std::string node_id = node_id_until_dist + contact_node->get_id().substr(ID_SINGLE_START, ID_SINGLE_LEN);
                unsigned short port = starting_port_number + convert_ID_string_to_int(node_id,
                                                                num_nodes_in_dist, num_cnodes_in_dist, 
                                                                num_nodes_in_city, num_cnodes_in_city, 
                                                                num_nodes_in_state, num_cnodes_in_state, 
                                                                num_nodes_in_country, num_cnodes_in_country, 
                                                                num_nodes_in_continent);
                Node node(node_id, "127.0.0.1", port);

                peer_list.push_back(std::make_shared<Node>(node));
                peer_set.insert({node_id, std::make_shared<Node>(node)});

                if (j < num_cnodes_in_city) {
                    contact_nodes_next.insert({node_id, std::make_shared<Node>(node)});
                    j++;
                }
            }
        }

        table_dist.contact_nodes = contact_nodes_next;
        table_dist.predecessor = NULL;
        table_dist.successor = NULL;
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
    if (num_nodes_in_state == 0) {
        this->node_table->set_tables(tables);
        return;
    }
    if (contact_nodes.find(this->node->get_id()) != contact_nodes.end()) {
        // should be the contact node of the city level ring
        Ring table_city;
        table_city.ring_level = 2;

        // reset stringstream and ring variables
        peer_set.clear();
        peer_list.clear();

        std::string node_id_until_state = this->node->get_id().substr(0, ID_CITY_START);
        int j = 0;
        for (int i = 0; i < num_cities_in_state; i++) {
            ss.str("");
            ss.clear();
            ss << std::setw(ID_CITY_LEN) << std::setfill('0') << i;
            std::string city_id_in_state = ss.str();
            std::string node_id_until_city = node_id_until_state + city_id_in_state;
            for (auto contact_node : contact_nodes_list) {
                std::string node_id = node_id_until_city + contact_node->get_id().substr(ID_DISTRICT_START);
                unsigned short port = starting_port_number + convert_ID_string_to_int(node_id,
                                                                num_nodes_in_dist, num_cnodes_in_dist, 
                                                                num_nodes_in_city, num_cnodes_in_city, 
                                                                num_nodes_in_state, num_cnodes_in_state, 
                                                                num_nodes_in_country, num_cnodes_in_country, 
                                                                num_nodes_in_continent);
                Node node(node_id, "127.0.0.1", port);

                peer_list.push_back(std::make_shared<Node>(node));
                peer_set.insert({node_id, std::make_shared<Node>(node)});

                if (j < num_cnodes_in_state) {
                    contact_nodes_next.insert({node_id, std::make_shared<Node>(node)});
                    j++;
                }
            }
        }

        table_city.contact_nodes = contact_nodes_next;
        table_city.predecessor = NULL;
        table_city.successor = NULL;
        table_city.peer_set = peer_set;
        table_city.peer_list = peer_list;

        tables.push_back(table_city);

        // exchange contact_nodes and contact_nodes_next
        contact_nodes.clear();
        contact_nodes_list.clear();
        for (auto contact_node : contact_nodes_next) {
            contact_nodes.insert(contact_node);
            contact_nodes_list.push_back(contact_node.second);
        }
        contact_nodes_next.clear();
    }

    // contact nodes - state level
    if (num_nodes_in_country == 0) {
        this->node_table->set_tables(tables);
        return;
    }
    if (contact_nodes.find(this->node->get_id()) != contact_nodes.end()) {
        // should be the contact node of the state level ring
        Ring table_state;
        table_state.ring_level = 3;

        // reset stringstream and ring variables
        peer_set.clear();
        peer_list.clear();

        std::string node_id_until_country = this->node->get_id().substr(0, ID_STATE_START);
        int j = 0;
        for (int i = 0; i < num_states_in_country; i++) {
            ss.str("");
            ss.clear();
            ss << std::setw(ID_STATE_LEN) << std::setfill('0') << i;
            std::string state_id_in_country = ss.str();
            std::string node_id_until_state = node_id_until_country + state_id_in_country;
            for (auto contact_node : contact_nodes_list) {
                std::string node_id = node_id_until_state + contact_node->get_id().substr(ID_CITY_START);
                unsigned short port = starting_port_number + convert_ID_string_to_int(node_id,
                                                                num_nodes_in_dist, num_cnodes_in_dist, 
                                                                num_nodes_in_city, num_cnodes_in_city, 
                                                                num_nodes_in_state, num_cnodes_in_state, 
                                                                num_nodes_in_country, num_cnodes_in_country, 
                                                                num_nodes_in_continent);
                Node node(node_id, "127.0.0.1", port);

                peer_list.push_back(std::make_shared<Node>(node));
                peer_set.insert({node_id, std::make_shared<Node>(node)});

                if (j < num_cnodes_in_state) {
                    contact_nodes_next.insert({node_id, std::make_shared<Node>(node)});
                    j++;
                }
            }
        }

        table_state.contact_nodes = contact_nodes_next;
        table_state.predecessor = NULL;
        table_state.successor = NULL;
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
        this->node_table->set_tables(tables);
        return;
    }
    if (contact_nodes.find(this->node->get_id()) != contact_nodes.end()) {
        // should be the contact node of the country level ring
        Ring table_country;
        table_country.ring_level = 4;

        // reset stringstream and ring variables
        peer_set.clear();
        peer_list.clear();

        std::string node_id_until_continent = this->node->get_id().substr(0, ID_COUNTRY_START);
        int j = 0;
        for (int i = 0; i < num_countries_in_continent; i++) {
            ss.str("");
            ss.clear();
            ss << std::setw(ID_COUNTRY_LEN) << std::setfill('0') << i;
            std::string country_id_in_continent = ss.str();
            std::string node_id_until_country = node_id_until_continent + country_id_in_continent;
            for (auto contact_node : contact_nodes_list) {
                std::string node_id = node_id_until_country + contact_node->get_id().substr(ID_STATE_START);
                unsigned short port = starting_port_number + convert_ID_string_to_int(node_id,
                                                                num_nodes_in_dist, num_cnodes_in_dist, 
                                                                num_nodes_in_city, num_cnodes_in_city, 
                                                                num_nodes_in_state, num_cnodes_in_state, 
                                                                num_nodes_in_country, num_cnodes_in_country, 
                                                                num_nodes_in_continent);
                Node node(node_id, "127.0.0.1", port);

                peer_list.push_back(std::make_shared<Node>(node));
                peer_set.insert({node_id, std::make_shared<Node>(node)});

                if (j < num_cnodes_in_country) {
                    contact_nodes_next.insert({node_id, std::make_shared<Node>(node)});
                    j++;
                }
            }
        }

        table_country.contact_nodes = contact_nodes_next;
        table_country.predecessor = NULL;
        table_country.successor = NULL;
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
    if (contact_nodes.find(this->node->get_id()) != contact_nodes.end()) {
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
                unsigned short port = starting_port_number + convert_ID_string_to_int(node_id,
                                                                num_nodes_in_dist, num_cnodes_in_dist, 
                                                                num_nodes_in_city, num_cnodes_in_city, 
                                                                num_nodes_in_state, num_cnodes_in_state, 
                                                                num_nodes_in_country, num_cnodes_in_country, 
                                                                num_nodes_in_continent);
                Node node(node_id, "127.0.0.1", port);

                peer_list.push_back(std::make_shared<Node>(node));
                peer_set.insert({node_id, std::make_shared<Node>(node)});

                if (j < num_cnodes_in_continent) {
                    contact_nodes_next.insert({node_id, std::make_shared<Node>(node)});
                    j++;
                }
            }
        }

        table_continent.contact_nodes = contact_nodes_next;
        table_continent.predecessor = NULL;
        table_continent.successor = NULL;
        table_continent.peer_set = peer_set;
        table_continent.peer_list = peer_list;

        tables.push_back(table_continent);
    }

    this->node_table->set_tables(tables);

    return;
}

void BaseApp::start(const std::string &start_time, int num_nodes_in_dist, int num_cnodes_in_dist, 
        int num_nodes_in_city, int num_cnodes_in_city, 
        int num_nodes_in_state, int num_cnodes_in_state, 
        int num_nodes_in_country, int num_cnodes_in_country, 
        int num_nodes_in_continent, int num_continents,
        int num_cnodes_in_continent,
        unsigned short starting_port_number) {
    std::cout << "Setting up NodeTable for node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]\n";
    std::cout << "Establishing structure on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]\n";
    
    this->form_structure(num_nodes_in_dist, num_cnodes_in_dist, 
        num_nodes_in_city, num_cnodes_in_city, 
        num_nodes_in_state, num_cnodes_in_state, 
        num_nodes_in_country, num_cnodes_in_country, 
        num_nodes_in_continent, num_continents,
        num_cnodes_in_continent,
        starting_port_number);
    
    std::cout << "Structure established on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]\n";
    std::cout << "Node Tables on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]\n";
    for (auto table : this->node_table->get_tables()) {
	std::cout << "Level: " << std::to_string(table.ring_level) << "\n";
        for (auto peer : table.peer_list) {
	    std::cout << "Peer - " + peer->get_id() + " " + peer->get_ip() + ":" + std::to_string(peer->get_port()) << "\n";
        }
        for (auto contact_node : table.contact_nodes) {
	    std::cout << "Contact node - " + contact_node.first + " " + contact_node.second->get_ip() + ":" + std::to_string(contact_node.second->get_port()) << "\n";
        }
    }

    this->peer_manager = std::make_shared<PeerManager>(node, node_table, start_time);

    std::cout << "Starting HGFR PeerManager on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]\n";
    this->peer_manager->start();
}

void BaseApp::stop() {
    this->peer_manager->stop();
}

void BaseApp::broadcast(const std::string &data) {
    this->peer_manager->broadcast(data);
}

int main(int argc, char** argv) {
    if (argc != 17) {
	std::cout << "Wrong arguments. Correct usage: "
                                        << "./app_eth ip_addr port_num id "
                                            << "num_nodes_in_dist num_cnodes_in_dist " 
                                            << "num_nodes_in_city num_cnodes_in_city " 
                                            << "num_nodes_in_state num_cnodes_in_state "
                                            << "num_nodes_in_country num_cnodes_in_country "
                                            << "num_nodes_in_continent num_cnodes_in_continent "
                                            << "num_continents"
                                            << "starting_port_num start_time\n";
        return 0;
    }

    std::string ip = argv[1];
    unsigned short port = (unsigned short) std::atoi(argv[2]);
    std::string id = argv[3];

    // information used for network topology establishment (only used for evaluation)
    int num_nodes_in_dist = std::atoi(argv[4]);
    int num_cnodes_in_dist = std::atoi(argv[5]);
    int num_nodes_in_city = std::atoi(argv[6]);
    int num_cnodes_in_city = std::atoi(argv[7]); 
    int num_nodes_in_state = std::atoi(argv[8]);
    int num_cnodes_in_state = std::atoi(argv[9]); 
    int num_nodes_in_country = std::atoi(argv[10]);
    int num_cnodes_in_country = std::atoi(argv[11]); 
    int num_nodes_in_continent = std::atoi(argv[12]);
    int num_cnodes_in_continent = std::atoi(argv[13]);
    int num_continents = std::atoi(argv[14]);
    int starting_port_number = std::atoi(argv[15]);
    std::string start_time = argv[16];

    /*
    int order = convert_ID_string_to_int(id, num_nodes_in_dist, num_cnodes_in_dist, 
                                            num_nodes_in_city, num_cnodes_in_city, 
                                            num_nodes_in_state, num_cnodes_in_state, 
                                            num_nodes_in_country, num_cnodes_in_country, 
                                            num_nodes_in_continent);
    std::cout << "Order: " << order << "\n";
    std::string id_string = convert_ID_int_to_string(order, num_nodes_in_dist, num_cnodes_in_dist, 
                                            num_nodes_in_city, num_cnodes_in_city, 
                                            num_nodes_in_state, num_cnodes_in_state, 
                                            num_nodes_in_country, num_cnodes_in_country, 
                                            num_nodes_in_continent);
    */

    // initialize the app
    std::cout << "Creating HGFR base application on node [ID: " + id + "] [IP: " + ip + "] [" + std::to_string(port) + "]\n";
    BaseApp app = BaseApp(ip, port, id);

    // start the app service
    std::cout << "Starting HGFR base service on node [ID: " + id + "] [IP: " + ip + "] [" + std::to_string(port) + "]\n";
    app.start(start_time, num_nodes_in_dist, num_cnodes_in_dist, 
        num_nodes_in_city, num_cnodes_in_city, 
        num_nodes_in_state, num_cnodes_in_state,
        num_nodes_in_country, num_cnodes_in_country,
        num_nodes_in_continent, num_continents,
        num_cnodes_in_continent,
        starting_port_number);

    // message record logging
    std::ofstream ofs;
    ofs.open("../test/log/" + start_time + "/" + app.get_node()->get_id() + ".csv");
    if (ofs.is_open()) {
        ofs << Message::csv_header << "\n";
        ofs.close();
    } else {
	std::cout << "Error opening file\n";
    }
    
    // broadcast a message
    int order = convert_ID_string_to_int(id, num_nodes_in_dist, num_cnodes_in_dist, 
                                            num_nodes_in_city, num_cnodes_in_city, 
                                            num_nodes_in_state, num_cnodes_in_state, 
                                            num_nodes_in_country, num_cnodes_in_country, 
                                            num_nodes_in_continent);
    if (order < 180) {
        std::this_thread::sleep_for (std::chrono::seconds(5));
	std::cout << "Slept for 5 seconds\n";
	std::cout << "Broadcasting message ...\n";
        app.broadcast(data_of_block_size);
    }

    // block
    app.stop();

    return 0;
}

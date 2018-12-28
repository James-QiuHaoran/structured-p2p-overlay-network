#include "app_eth.h"

// constructors
BaseAppETH::BaseAppETH(std::string ip, unsigned short port, std::string id) {
    this->node = std::make_shared<Node>(id, ip, port);
    this->node_table = std::make_shared<NodeTableETH>(id);
}

// getters
std::shared_ptr<Node> BaseAppETH::get_node() {
    return this->node;
}

std::shared_ptr<NodeTableETH> BaseAppETH::get_node_table() {
    return this->node_table;
}

std::shared_ptr<PeerManagerETH> BaseAppETH::get_peer_manager() {
    return this->peer_manager;
}

// public functions
void BaseAppETH::form_structure(int num_nodes_in_dist, int num_cnodes_in_dist, 
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

    int self_order = convert_ID_string_to_int(this->node->get_id(),
                                                num_nodes_in_dist, num_cnodes_in_dist, 
                                                num_nodes_in_city, num_cnodes_in_city, 
                                                num_nodes_in_state, num_cnodes_in_state, 
                                                num_nodes_in_country, num_cnodes_in_country, 
                                                num_nodes_in_continent);

    for (int i = 0; i < TABLE_SIZE_ETH; i++) {
        // int id = this->random_num_in_range(0, num_nodes_total-1);
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

                            if (neighbor_ids.find(order) != neighbor_ids.end() && node_id != this->node->get_id()) {
                                unsigned short port = starting_port_number + order;
                                Node node(node_id, "127.0.0.1", port);
                                table.push_back(std::make_shared<Node>(node));
                            }
                            counter++;
                        }
                    }
                }
            }
        }
    }

    this->node_table->set_table(table);

    return;
}

void BaseAppETH::start(const std::string &start_time, int num_nodes_in_dist, int num_cnodes_in_dist, 
        int num_nodes_in_city, int num_cnodes_in_city, 
        int num_nodes_in_state, int num_cnodes_in_state, 
        int num_nodes_in_country, int num_cnodes_in_country, 
        int num_nodes_in_continent, int num_continents,
        int num_cnodes_in_continent,
        unsigned short starting_port_number) {
    BOOST_LOG_TRIVIAL(debug) << "Setting up NodeTable for node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]";
    BOOST_LOG_TRIVIAL(debug) << "Establishing structure on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]";
    
    // form the geographical structure
    this->form_structure(num_nodes_in_dist, num_cnodes_in_dist, 
        num_nodes_in_city, num_cnodes_in_city, 
        num_nodes_in_state, num_cnodes_in_state, 
        num_nodes_in_country, num_cnodes_in_country, 
        num_nodes_in_continent, num_continents, 
        num_cnodes_in_continent,
        starting_port_number);
    
    BOOST_LOG_TRIVIAL(debug) << "Structure established on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]";
    BOOST_LOG_TRIVIAL(debug) << "Node Tables on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]";
    
    for (auto peer : this->node_table->get_table()) {
        BOOST_LOG_TRIVIAL(debug) << "Peer - " + peer->get_id() + " " + peer->get_ip() + ":" + std::to_string(peer->get_port());
    }

    this->peer_manager = std::make_shared<PeerManagerETH>(node, node_table, start_time);

    // set gossip mode
    this->peer_manager->set_mode(PeerManagerETH::PUSH);  // PUSH version
    // this->peer_manager->set_mode(PeerManagerETH::PULL);  // PULL version

    BOOST_LOG_TRIVIAL(debug) << "Starting ETH PeerManager on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]";
    this->peer_manager->start();

    return;
}

void BaseAppETH::stop() {
    // BOOST_LOG_TRIVIAL(debug) << "Stopping ETH PeerManager on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]";
    this->peer_manager->stop();

    return;
}

void BaseAppETH::broadcast(const std::string &data) {
    this->peer_manager->broadcast(data, TTL_ETH, "");

    return;
}

// random number generated uniformly from [low, high]
int BaseAppETH::random_num_in_range(int low, int high) {
    boost::random::uniform_int_distribution<> dist(low, high);
    return dist(gen_seed);
}

int main(int argc, char** argv) {
    // srand ( time(NULL) );
    srand(std::atoi(argv[2]));

    if (argc != 17) {
        BOOST_LOG_TRIVIAL(info) << "Wrong arguments. Correct usage: "
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

    // initialize the app
    BOOST_LOG_TRIVIAL(debug) << "Creating ETH base application on node [ID: " + id + "] [IP: " + ip + "] [" + std::to_string(port) + "]";
    BaseAppETH app = BaseAppETH(ip, port, id);

    // start the app service
    BOOST_LOG_TRIVIAL(debug) << "Starting ETH base service on node [ID: " + id + "] [IP: " + ip + "] [" + std::to_string(port) + "]";
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
        BOOST_LOG_TRIVIAL(trace) << "Error opening file";
    }
    
    // broadcast a message
    //if (id == "00000000000000000000000000000000") {
        std::this_thread::sleep_for (std::chrono::seconds(3));
        BOOST_LOG_TRIVIAL(trace) << "Slept for 3 seconds";
        BOOST_LOG_TRIVIAL(trace) << "Broadcasting message ...";
        // app.broadcast("MSG #1: Hello world!");
        app.broadcast(data_of_block_size);
    //}

    // stop the app service
    // BOOST_LOG_TRIVIAL(debug) << "Stopping ETH base service on node [ID: " + id + "] [IP: " + ip + "] [" + std::to_string(port) + "]";
    app.stop();

    return 0;
}
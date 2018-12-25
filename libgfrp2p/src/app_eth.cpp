#include "app_eth.h"

// constructors
BaseApp::BaseApp(std::string ip, unsigned short port, std::string id) {
    this->node = std::make_shared<Node>(id, ip, port);
    this->node_table = std::make_shared<NodeTableETH>(id);
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
        int num_nodes_in_continent, unsigned short starting_port_number) {
    
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
    int num_cities_in_state = num_nodes_in_state/num_cnodes_in_dist;

    // set node table
    std::vector<std::shared_ptr<Node>> table;
    this->node_table->set_table(table);

    return;
}

// convert ID to port
unsigned short BaseApp::convert_ID_to_port(unsigned short starting_port_number, const std::string& id,
    int num_nodes_in_dist, int num_cnodes_in_dist, 
    int num_nodes_in_city, int num_cnodes_in_city, 
    int num_nodes_in_state, int num_cnodes_in_state, 
    int num_nodes_in_country, int num_cnodes_in_country, 
    int num_nodes_in_continent) {
    // BOOST_LOG_TRIVIAL(debug) << id;
    
    std::string id_in_dist = id.substr(ID_SINGLE_START);
    std::string dist_id = id.substr(ID_DISTRICT_START, ID_DISTRICT_LEN);
    std::string city_id = id.substr(ID_CITY_START, ID_CITY_LEN);
    std::string state_id = id.substr(ID_STATE_START, ID_STATE_LEN);
    std::string country_id = id.substr(ID_COUNTRY_START, ID_COUNTRY_LEN);
    std::string continent_id = id.substr(ID_CONTINENT_START, ID_CONTINENT_LEN);

    int node_id_in_dist = 0, dist_id_int = 0, city_id_int = 0, state_id_int = 0, country_id_int = 0, continent_id_int = 0;
    
    std::stringstream ss_node(id_in_dist);
    ss_node >> node_id_in_dist;

    std::stringstream ss_dist(dist_id);
    ss_dist >> dist_id_int;
    int num_nodes_in_one_dist = num_nodes_in_dist;

    int num_dists_in_city = num_nodes_in_city/num_cnodes_in_dist;
    std::stringstream ss_city(city_id);
    ss_city >> city_id_int;
    int num_nodes_in_one_city = num_nodes_in_one_dist * num_dists_in_city;

    int num_cities_in_state = num_nodes_in_state/num_cnodes_in_dist;
    std::stringstream ss_state(state_id);
    ss_state >> state_id_int;
    int num_nodes_in_one_state = num_nodes_in_one_city * num_cities_in_state;

    int num_states_in_country = num_nodes_in_country/num_cnodes_in_state;
    std::stringstream ss_country(country_id);
    ss_country >> country_id_int;
    int num_nodes_in_one_country = num_nodes_in_one_state * num_states_in_country;

    int num_countries_in_continent = num_nodes_in_continent/num_cnodes_in_country;
    std::stringstream ss_continent(continent_id);
    ss_continent >> continent_id_int;
    int num_nodes_in_one_continent = num_nodes_in_one_country * num_countries_in_continent;

    int port_num = starting_port_number + 
                   num_nodes_in_one_continent * continent_id_int +
                   num_nodes_in_one_country * country_id_int +
                   num_nodes_in_one_state * state_id_int +
                   num_nodes_in_one_city * city_id_int +
                   num_nodes_in_one_dist * dist_id_int +
                   node_id_in_dist;

    return port_num;
}

void BaseApp::start(const std::string &start_time, int num_nodes_in_dist, int num_cnodes_in_dist, 
        int num_nodes_in_city, int num_cnodes_in_city, 
        int num_nodes_in_state, int num_cnodes_in_state, 
        int num_nodes_in_country, int num_cnodes_in_country, 
        int num_nodes_in_continent, unsigned short starting_port_number) {
    BOOST_LOG_TRIVIAL(debug) << "Setting up NodeTable for node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]";
    BOOST_LOG_TRIVIAL(debug) << "Establishing structure on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]";
    
    this->form_structure(num_nodes_in_dist, num_cnodes_in_dist, 
        num_nodes_in_city, num_cnodes_in_city, 
        num_nodes_in_state, num_cnodes_in_state, 
        num_nodes_in_country, num_cnodes_in_country, 
        num_nodes_in_continent, starting_port_number);
    
    BOOST_LOG_TRIVIAL(debug) << "Structure established on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]";
    BOOST_LOG_TRIVIAL(debug) << "Node Tables on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]";
    for (auto table : this->node_table->get_tables()) {
        BOOST_LOG_TRIVIAL(debug) << "Level: " + std::to_string(table.ring_level);
        for (auto peer : table.peer_list) {
            BOOST_LOG_TRIVIAL(debug) << "Peer - " + peer->get_id() + " " + peer->get_ip() + ":" + std::to_string(peer->get_port());
        }
        for (auto contact_node : table.contact_nodes) {
            BOOST_LOG_TRIVIAL(debug) << "Contact node - " + contact_node.first + " " + contact_node.second->get_ip() + ":" + std::to_string(contact_node.second->get_port());
        }
    }

    this->peer_manager = std::make_shared<PeerManager>(node, node_table, start_time);

    BOOST_LOG_TRIVIAL(debug) << "Starting HGFR PeerManager on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]";
    this->peer_manager->start();
}

void BaseApp::stop() {
    // BOOST_LOG_TRIVIAL(debug) << "Stopping HGFR PeerManager on node [ID: " + this->node->get_id() + "] [IP: " + this->node->get_ip() + "] [" + std::to_string(this->node->get_port()) + "]";
    this->peer_manager->stop();
}

void BaseApp::broadcast(const std::string &data) {
    this->peer_manager->broadcast(data);
}

int main(int argc, char** argv) {
    if (argc != 15) {
        BOOST_LOG_TRIVIAL(info) << "Wrong arguments. Correct usage: ./app ip_addr port_num id num_nodes_in_dist num_cnodes_in_dist num_nodes_in_city num_cnodes_in_city num_nodes_in_state num_cnodes_in_state num_nodes_in_country num_cnodes_in_country num_nodes_in_continent starting_port_num start_time\n";
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
    int starting_port_number = std::atoi(argv[13]);
    std::string start_time = argv[14];

    // initialize the app
    BOOST_LOG_TRIVIAL(debug) << "Creating HGFR base application on node [ID: " + id + "] [IP: " + ip + "] [" + std::to_string(port) + "]";
    BaseApp app = BaseApp(ip, port, id);

    // start the app service
    BOOST_LOG_TRIVIAL(debug) << "Starting HGFR base service on node [ID: " + id + "] [IP: " + ip + "] [" + std::to_string(port) + "]";
    app.start(start_time, num_nodes_in_dist, num_cnodes_in_dist, 
        num_nodes_in_city, num_cnodes_in_city, 
        num_nodes_in_state, num_cnodes_in_state,
        num_nodes_in_country, num_cnodes_in_country,
        num_nodes_in_continent, starting_port_number);

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
    if (id == "00000000000000000000000000000000") {
        std::this_thread::sleep_for (std::chrono::seconds(5));
        BOOST_LOG_TRIVIAL(debug) << "Slept for 5 seconds";
        BOOST_LOG_TRIVIAL(debug) << "Broadcasting message ... [MSG #1: Hello world!]";
        app.broadcast("MSG #1: Hello world!");
        app.broadcast("MSG #2: Hello world, again!");
    }

    /*
    if (id == "00000000000000000000000000000000") {
        std::this_thread::sleep_for (std::chrono::seconds(5));
        BOOST_LOG_TRIVIAL(debug) << "Slept for 5 seconds";
        BOOST_LOG_TRIVIAL(debug) << "Broadcasting message ... [MSG #2: Hello world, again!]";
        app.broadcast("MSG #2: Hello world, again!");
    }*/

    // stop the app service
    // BOOST_LOG_TRIVIAL(debug) << "Stopping HGFR base service on node [ID: " + id + "] [IP: " + ip + "] [" + std::to_string(port) + "]";
    app.stop();

    return 0;
}
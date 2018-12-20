#include "app.h"

// constructors
BaseApp::BaseApp(std::string ip, unsigned short port, std::string id) {
    this->node = Node(id, ip, port);
    this->node_table = NodeTable(id);
    this->peer_manager = PeerManager(node, node_table);
}

BaseApp::BaseApp(std::string ip, unsigned short port, std::string id, 
    int num_nodes_in_dist, int num_cnodes_in_dist, 
        int num_nodes_in_city, int num_cnodes_in_city, 
        int num_nodes_in_state, int num_cnodes_in_state, 
        int num_nodes_in_country, int num_cnodes_in_country, 
        int num_nodes_in_continent, unsigned short starting_port_number) {
    this->node = Node(id, ip, port);
    this->node_table = NodeTable(id);
    this->peer_manager = PeerManager(node, node_table);

    this->form_structure(num_nodes_in_dist, num_cnodes_in_dist, 
        num_nodes_in_city, num_cnodes_in_city, 
        num_nodes_in_state, num_cnodes_in_state, 
        num_nodes_in_country, num_cnodes_in_country, 
        num_nodes_in_continent, starting_port_number);
}

// public functions
void BaseApp::form_structure(int num_nodes_in_dist, int num_cnodes_in_dist, 
        int num_nodes_in_city, int num_cnodes_in_city, 
        int num_nodes_in_state, int num_cnodes_in_state, 
        int num_nodes_in_country, int num_cnodes_in_country, 
        int num_nodes_in_continent, unsigned short starting_port_number) {
    // form network topology based ID
    std::string id_in_dist = this->node.get_id().substr(ID_SINGLE_START, ID_SINGLE_START+ID_SINGLE_LEN);
    std::string dist_id = this->node.get_id().substr(ID_DISTRICT_START, ID_DISTRICT_START+ID_DISTRICT_LEN);
    std::string city_id = this->node.get_id().substr(ID_CITY_START, ID_CITY_START+ID_CITY_LEN);
    std::string state_id = this->node.get_id().substr(ID_STATE_START, ID_STATE_START+ID_STATE_LEN);
    std::string country_id = this->node.get_id().substr(ID_COUNTRY_START, ID_COUNTRY_START+ID_COUNTRY_LEN);
    std::string continent_id = this->node.get_id().substr(ID_CONTINENT_START, ID_CONTINENT_START+ID_CONTINENT_LEN);

    std::stringstream ss_node(id_in_dist);
    int node_id_in_dist = 0;
    ss_node >> node_id_in_dist;

    std::stringstream ss_dist(dist_id);
    int dist_id_int = 0;
    ss_dist >> dist_id_int;
    int num_nodes_in_one_dist = num_nodes_in_dist;

    int num_dists_in_city = num_nodes_in_city/num_cnodes_in_dist;
    std::stringstream ss_city(city_id);
    int city_id_int = 0;
    ss_city >> city_id_int;
    int num_nodes_in_one_city = num_nodes_in_one_dist * num_dists_in_city;

    int num_cities_in_state = num_nodes_in_state/num_cnodes_in_dist;
    std::stringstream ss_state(state_id);
    int state_id_int = 0;
    ss_state >> state_id_int;
    int num_nodes_in_one_state = num_nodes_in_one_city * num_cities_in_state;

    int num_states_in_country = num_nodes_in_country/num_cnodes_in_state;
    std::stringstream ss_country(country_id);
    int country_id_int = 0;
    ss_country >> country_id_int;
    int num_nodes_in_one_country = num_nodes_in_one_state * num_states_in_country;

    int num_countries_in_continent = num_nodes_in_continent/num_cnodes_in_country;
    std::stringstream ss_continent(continent_id);
    int continent_id_int = 0;
    ss_continent >> continent_id_int;
    int num_nodes_in_one_continent = num_nodes_in_one_country * num_countries_in_continent;

    std::vector<Ring> tables;
    std::unordered_map<std::string, std::shared_ptr<Node>> contact_nodes;  // contact nodes of the ring
    std::shared_ptr<Node> predecessor;                                     // successor within the ring
    std::shared_ptr<Node> successor;                                       // predecessor within the ring
    std::unordered_map<std::string, std::shared_ptr<Node>> peer_set;       // peers in an unordered map
    std::vector<std::shared_ptr<Node>> peer_list;                          // used for broadcast within ring

    // normal nodes - peer level
    Ring table_peer;
    table_peer.ring_level = 0;

    // add other peers
    std::stringstream ss;
    int starting_port_number_in_dist = starting_port_number + 
                                       num_nodes_in_one_continent * continent_id_int +
                                       num_nodes_in_one_country * country_id_int +
                                       num_nodes_in_one_state * state_id_int +
                                       num_nodes_in_one_city * city_id_int +
                                       num_nodes_in_one_dist * dist_id_int;

    for (int i = 0; i < num_nodes_in_dist; i++) {
        ss << std::setw(9) << std::setfill('0') << i;
        std::string peer_id_in_dist = ss.str();
        std::string node_id = this->node.get_id().substr(0, ID_SINGLE_START) + peer_id_in_dist;
        unsigned short port = starting_port_number_in_dist + i;
        Node node(node_id, "127.0.0.1", port);

        // insert into contact node list
        if (i < num_cnodes_in_dist) {
            contact_nodes.insert({node_id, std::make_shared<Node>(node)});
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
    if (node_id_in_dist < num_cnodes_in_dist) {
        // should be the contact node of the dist level ring
        Ring table_dist;
        table_dist.ring_level = 1;

        // reset stringstream and ring variables
        contact_nodes.clear();
        peer_set.clear();
        peer_list.clear();
        ss.str("");
        ss.clear();

        // add other peers in that level
        for (int i = 0; i < num_cnodes_in_dist; i++) {

        }

        tables.push_back(table_dist);
    }

    // contact nodes - city level
    if (node_id_in_dist < num_cnodes_in_city/num_dists_in_city) {
        // should be the contact node of the city level ring
        Ring table_city;
        table_city.ring_level = 2;

        // reset stringstream and ring variables
        contact_nodes.clear();
        peer_set.clear();
        peer_list.clear();
        ss.str("");
        ss.clear();

        // add other peers in that level
        for (int i = 0; i < num_cnodes_in_city; i++) {

        }

        tables.push_back(table_city);
    }

    // contact nodes - state level
    if (node_id_in_dist < num_cnodes_in_state/num_cities_in_state) {
        // should be the contact node of the state level ring
        Ring table_state;
        table_state.ring_level = 3;

        // reset stringstream and ring variables
        contact_nodes.clear();
        peer_set.clear();
        peer_list.clear();
        ss.str("");
        ss.clear();

        // add other peers in that level
        for (int i = 0; i < num_cnodes_in_state; i++) {

        }

        tables.push_back(table_state);
    }

    // [TODO]
    // contact nodes - country level
    // contact nodes - continent level
}

void BaseApp::start() {
    BOOST_LOG_TRIVIAL(debug) << "Setting up NodeTable for node [ID: " + this->node.get_id() + "] [IP: " + this->node.get_ip() + "] [" + std::to_string(this->node.get_port()) + "]";
    std::vector<Ring> tables;
    this->node_table.set_tables(tables);

    BOOST_LOG_TRIVIAL(debug) << "Starting HGFR PeerManager on node [ID: " + this->node.get_id() + "] [IP: " + this->node.get_ip() + "] [" + std::to_string(this->node.get_port()) + "]";
    this->peer_manager.start();
}

void BaseApp::stop() {
    BOOST_LOG_TRIVIAL(debug) << "Stopping HGFR PeerManager on node [ID: " + this->node.get_id() + "] [IP: " + this->node.get_ip() + "] [" + std::to_string(this->node.get_port()) + "]";
    this->peer_manager.stop();
}

int main(int argc, char** argv) {
    if (argc != 4) {
        BOOST_LOG_TRIVIAL(info) << "Wrong arguments. Correct usage: ./app ip_addr port_num id\n";
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

    // initialize the app
    BOOST_LOG_TRIVIAL(debug) << "Creating HGFR base application on node [ID: " + id + "] [IP: " + ip + "] [" + std::to_string(port) + "]";
    BaseApp app = BaseApp(ip, port, id, 
        num_nodes_in_dist, num_cnodes_in_dist, 
        num_nodes_in_city, num_cnodes_in_city, 
        num_nodes_in_state, num_cnodes_in_state,
        num_nodes_in_country, num_cnodes_in_country,
        num_nodes_in_continent, starting_port_number);

    // start the app service
    BOOST_LOG_TRIVIAL(debug) << "Starting HGFR base service on node [ID: " + id + "] [IP: " + ip + "] [" + std::to_string(port) + "]";
    app.start();

    // stop the app service
    BOOST_LOG_TRIVIAL(debug) << "Stopping HGFR base service on node [ID: " + id + "] [IP: " + ip + "] [" + std::to_string(port) + "]";
    app.stop();

    return 0;
}
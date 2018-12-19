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
    std::stringstream str_stream(id_in_dist);
    std::stringstream ss;

    int node_id_in_dist = 0;
    str_stream >> node_id_in_dist;

    std::vector<Ring> tables;

    // normal nodes - peer level
    Ring table_peer;
    table_peer.ring_level = 0;
    std::unordered_map<std::string, std::shared_ptr<Node>> contact_nodes;  // contact nodes of the ring
    std::shared_ptr<Node> predecessor;                                     // successor within the ring
    std::shared_ptr<Node> successor;                                       // predecessor within the ring
    std::unordered_map<std::string, std::shared_ptr<Node>> peer_set;       // peers in an unordered map
    std::vector<std::shared_ptr<Node>> peer_list;                          // used for broadcast within ring

    // add other peers
    for (int i = 0; i < num_nodes_in_dist; i++) {
        ss << std::setw(9) << std::setfill('0') << i;
        std::string peer_id_in_dist = ss.str();
        std::string node_id = this->node.get_id().substr(0, ID_SINGLE_START) + peer_id_in_dist;
        unsigned short port = starting_port_number;
        Node node(node_id, "127.0.0.1", port);

        if (node_id_in_dist < num_cnodes_in_dist) {

        }
    }

    tables.push_back(table_peer);
    
    // contact nodes - dist level
    if (node_id_in_dist < num_cnodes_in_dist) {
        // should be the contact node of the dist level ring
        Ring table_dist;
        table_dist.ring_level = 1;

        // add other peers in that level
        for (int i = 0; i < num_cnodes_in_dist; i++) {

        }

        tables.push_back(table_dist);
    }

    // contact nodes - city level
    int num_dists_in_city = num_nodes_in_city/num_cnodes_in_dist;
    if (node_id_in_dist < num_cnodes_in_city/num_dists_in_city) {
        // should be the contact node of the city level ring
        Ring table_city;
        table_city.ring_level = 2;

        // add other peers in that level
        for (int i = 0; i < num_cnodes_in_city; i++) {

        }

        tables.push_back(table_city);
    }

    // contact nodes - state level
    int num_cities_in_state = num_nodes_in_state/num_cnodes_in_dist;
    if (node_id_in_dist < num_cnodes_in_state/num_cities_in_state) {
        // should be the contact node of the state level ring
        Ring table_state;
        table_state.ring_level = 3;

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
#ifndef APP_ETH_H
#define APP_ETH_H

#include <string>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>

#include "peer_manager_eth.h"
#include "node.h"
#include "node_table_eth.h"
#include "message.h"
#include "param_constants.h"
#include "utils.h"

boost::random::mt19937 gen_seed;

// string of data of length 1024 bytes
static const std::string one_kb_data(1024, 'S');
static const std::string data_of_block_size(1024*17, 'S');

class BaseAppETH {
private:
    std::shared_ptr<Node> node;
    std::shared_ptr<NodeTableETH> node_table;
    std::shared_ptr<PeerManagerETH> peer_manager;

public:
    // constructor
    BaseAppETH(std::string ip, unsigned short port, std::string id);

    // getters
    std::shared_ptr<Node> get_node();
    std::shared_ptr<NodeTableETH> get_node_table();
    std::shared_ptr<PeerManagerETH> get_peer_manager();

    // form the network topology
    void form_structure(int num_nodes_in_dist, int num_cnodes_in_dist, 
    	int num_nodes_in_city, int num_cnodes_in_city, 
    	int num_nodes_in_state, int num_cnodes_in_state, 
    	int num_nodes_in_country, int num_cnodes_in_country, 
    	int num_nodes_in_continent, int num_continents,
        int num_cnodes_in_continent,
        unsigned short starting_port_number);

    // generate random number in range [low, high]
    int random_num_in_range(int low, int high);

    // start the application
    void start(const std::string &start_time, int num_nodes_in_dist, int num_cnodes_in_dist, 
        int num_nodes_in_city, int num_cnodes_in_city, 
        int num_nodes_in_state, int num_cnodes_in_state, 
        int num_nodes_in_country, int num_cnodes_in_country, 
        int num_nodes_in_continent, int num_continents,
        int num_cnodes_in_continent,
        unsigned short starting_port_number);

    // stop the application
    void stop();

    // broadcast
    void broadcast(const std::string &data);
};

#endif
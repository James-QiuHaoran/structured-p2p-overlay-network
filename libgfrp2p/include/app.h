#ifndef APP_H
#define APP_H

#include <string>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>

#include "peer_manager.h"
#include "node.h"
#include "node_table.h"
#include "param_constants.h"

class BaseApp {
private:
    Node node;
    NodeTable node_table;
    PeerManager peer_manager;

public:
	// constructor
    BaseApp(std::string ip, unsigned short port, std::string id);
    BaseApp(std::string ip, unsigned short port, std::string id, 
    	int num_nodes_in_dist, int num_cnodes_in_dist, 
    	int num_nodes_in_city, int num_cnodes_in_city, 
    	int num_nodes_in_state, int num_cnodes_in_state, 
    	int num_nodes_in_country, int num_cnodes_in_country, 
    	int num_nodes_in_continent, unsigned short starting_port_number);

    // form the network topology
    void form_structure(int num_nodes_in_dist, int num_cnodes_in_dist, 
    	int num_nodes_in_city, int num_cnodes_in_city, 
    	int num_nodes_in_state, int num_cnodes_in_state, 
    	int num_nodes_in_country, int num_cnodes_in_country, 
    	int num_nodes_in_continent, unsigned short starting_port_number);

    // convert ID to port
    unsigned short convert_ID_to_port(unsigned short starting_port_number, const std::string& id,
    	int num_nodes_in_dist, int num_cnodes_in_dist, 
    	int num_nodes_in_city, int num_cnodes_in_city, 
    	int num_nodes_in_state, int num_cnodes_in_state, 
    	int num_nodes_in_country, int num_cnodes_in_country, 
    	int num_nodes_in_continent);

    // start the application
    void start();

    // stop the application
    void stop();
};

#endif
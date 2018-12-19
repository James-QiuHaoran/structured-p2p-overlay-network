#ifndef APP_H
#define APP_H

#include <string>
#include <cstdlib>
#include <iostream>

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
    BaseApp(std::string ip, unsigned short port, std::string id, int num_nodes, int num_dists, int num_cities, int num_states, int num_countries, int num_continents);

    // form the network topology
    void BaseApp::form_structure(int num_nodes, int num_dists, int num_cities, int num_states, int num_countries, int num_continents);

    // start the application
    void start();

    // stop the application
    void stop();
};

#endif
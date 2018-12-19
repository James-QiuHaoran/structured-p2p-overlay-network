#ifndef APP_H
#define APP_H

#include <string>
#include <cstdlib>
#include <iostream>

#include "peer_manager.h"
#include "node.h"
#include "node_table.h"

#define DEBUG_APP 1

class BaseApp {
private:
    Node node;
    NodeTable node_table;
    PeerManager peer_manager;

public:
	// constructor
    BaseApp(std::string ip, unsigned short port, std::string id);

    // start the application
    void start();

    // stop the application
    void stop();
};

#endif
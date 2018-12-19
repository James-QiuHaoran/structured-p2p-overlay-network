#ifndef APP_H
#define APP_H

#include <string>
#include <cstdlib>
#include <iostream>

#include "peer_manager.h"
#include "node.h"
#include "node_table.h"

class BaseApp {
private:
    Node node;
    NodeTable node_table;
    PeerManager peer_manager;

public:
    BaseApp(std::string ip, unsigned short port, std::string id);

    void start();
    void stop();
};

#endif

#ifndef APP_H
#define APP_H

#define RING_UPPER_LIMIT 20
#define RING_LOWER_LIMIT 10
#define HEARTBEAT_TIMEOUT 5
#define NUM_CONTACT_NODES 3
#define MSG_HASH_LENGTH  50
#define DEBUG             0

#include <string>
#include <cstdlib>
#include "peer_manager.h"

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

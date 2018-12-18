#ifndef APP_H
#define APP_H

#define RING_UPPER_LIMIT  20
#define RING_LOWER_LIMIT  10
#define HEARTBEAT_TIMEOUT  5
#define NUM_CONTACT_NODES  3
#define MSG_HASH_LENGTH   50
#define ID_CONTINENT_LEN   3
#define ID_COUNTRY_LEN     4
#define ID_STATE_LEN       5
#define ID_CITY_LEN        5
#define ID_SINGLE_LEN     10
#define ID_CONTINENT_START 0
#define ID_COUNTRY_START   3
#define ID_STATE_START     7
#define ID_CITY_START     12
#define ID_SINGLE_START   17
#define DEBUG              0

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

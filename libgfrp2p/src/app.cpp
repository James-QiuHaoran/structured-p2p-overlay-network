#include "app.h"

BaseApp::BaseApp(std::string ip, unsigned short port, std::string id) {
    this->node = Node(id, ip, port);
    this->node_table = NodeTable(id);
    this->peer_manager = PeerManager(node, node_table);
}

void BaseApp::start() {
    this->peer_manager.start();
}

void BaseApp::stop() {
    this->peer_manager.stop();
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cout << "Wrong arguments. Correct usage: ./app ip_addr port_num id\n";
        return 0;
    }

    std::string ip = argv[1];
    unsigned short port = (unsigned short) std::atoi(argv[2]);
    std::string id = argv[3];

    // initialize the app
    BaseApp app = BaseApp(ip, port, id);

    // start the app service
    app.start();

    // stop the app service
    app.stop();

    return 0;
}

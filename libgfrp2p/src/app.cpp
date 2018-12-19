#include "app.h"

BaseApp::BaseApp(std::string ip, unsigned short port, std::string id) {
    this->node = Node(id, ip, port);
    this->node_table = NodeTable(id);
    this->peer_manager = PeerManager(node, node_table);
}

void BaseApp::start() {
    if (DEBUG_APP)
        std::cout << "Starting HGFR PeerManager on node [ID: " + this->node.get_id() + "] [IP: " + this->node.get_ip() + "] [" + std::to_string(this->node.get_port()) + "]\n";
    this->peer_manager.start();
}

void BaseApp::stop() {
    if (DEBUG_APP)
        std::cout << "Stopping HGFR PeerManager on node [ID: " + this->node.get_id() + "] [IP: " + this->node.get_ip() + "] [" + std::to_string(this->node.get_port()) + "]\n";
    this->peer_manager.stop();
}

int main(int argc, char** argv) {
    if (argc != 4) {
        if (DEBUG_APP)
            std::cout << "Wrong arguments. Correct usage: ./app ip_addr port_num id\n";
        return 0;
    }

    std::string ip = argv[1];
    unsigned short port = (unsigned short) std::atoi(argv[2]);
    std::string id = argv[3];

    // initialize the app
    if (DEBUG_APP)
        std::cout << "Creating HGFR base application on node [ID: " + id + "] [IP: " + ip + "] [" + std::to_string(port) + "]\n";
    BaseApp app = BaseApp(ip, port, id);

    // start the app service
    if (DEBUG_APP)
        std::cout << "Starting HGFR base service on node [ID: " + id + "] [IP: " + ip + "] [" + std::to_string(port) + "]\n";
    app.start();

    // stop the app service
    if (DEBUG_APP)
        std::cout << "Stopping HGFR base service on node [ID: " + id + "] [IP: " + ip + "] [" + std::to_string(port) + "]\n";
    app.stop();

    return 0;
}
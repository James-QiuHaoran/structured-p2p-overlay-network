#include "app.h"

BaseApp::BaseApp(std::string ip, unsigned short port, std::string id) {
    this->node = Node(id, ip, port);
    this->node_table = NodeTable(id);
    this->peer_manager = PeerManager(node, node_table);
}

void BaseApp::start() {
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

    // initialize the app
    BOOST_LOG_TRIVIAL(debug) << "Creating HGFR base application on node [ID: " + id + "] [IP: " + ip + "] [" + std::to_string(port) + "]";
    BaseApp app = BaseApp(ip, port, id);

    // start the app service
    BOOST_LOG_TRIVIAL(debug) << "Starting HGFR base service on node [ID: " + id + "] [IP: " + ip + "] [" + std::to_string(port) + "]";
    app.start();

    // stop the app service
    BOOST_LOG_TRIVIAL(debug) << "Stopping HGFR base service on node [ID: " + id + "] [IP: " + ip + "] [" + std::to_string(port) + "]";
    app.stop();

    return 0;
}
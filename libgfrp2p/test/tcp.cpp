#include <iostream>

#include <boost/log/trivial.hpp>

#include "transport.h"

class TCPTest:  public Receiver, public enable_shared_from_this<TCPTest> {
public:

    std::unique_ptr<AsyncTCPServer> udp_server;

    void start(unsigned short port) {
        BOOST_LOG_TRIVIAL(debug) << "TCPTest::start: Allocating udp_server";
        udp_server.reset(new AsyncTCPServer(static_pointer_cast<Receiver>(this->shared_from_this()), port));
        this->udp_server->run();
    }

    virtual void receive(const std::string& ip, unsigned short port, const std::string& data) override {
        BOOST_LOG_TRIVIAL(info) << "TCPTest::receive: Packet of length " + std::to_string(data.length()) + " received from " + ip + ':' + std::to_string(port) + '\n' + data;
        
        unsigned short remote_port = 63304;

        
        BOOST_LOG_TRIVIAL(info) << "TCPTest::receive: Responding to " + ip + ':' + std::to_string(remote_port) + '\n';
        std::string msg = "Packet of length " + std::to_string(data.length()) + " received from " + ip + ':' + std::to_string(remote_port);

        this->udp_server->send(ip, remote_port, msg);
    }

};


int main(int argc, char* argv[]) {
    BOOST_LOG_TRIVIAL(debug) << "main: Starting with " << argc << " arguments";
    unsigned short port = std::stoi(argv[1]);
    BOOST_LOG_TRIVIAL(debug) << "main: Port number " << port;
    std::shared_ptr<TCPTest> udp_test(new TCPTest());
    udp_test->start(port);
    udp_test->udp_server->stop();
    return 0;
}
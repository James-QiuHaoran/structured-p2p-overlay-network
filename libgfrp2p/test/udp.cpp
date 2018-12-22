#include <iostream>

#include <boost/log/trivial.hpp>

#include "transport.h"

class UDPTest: public Receiver, public enable_shared_from_this<TCPTest> {
public:

    std::unique_ptr<AsyncUDPServer> udp_server;

    void start(unsigned short port) {
        BOOST_LOG_TRIVIAL(debug) << "UDPTest::start: Allocating udp_server";
        udp_server.reset(new AsyncUDPServer(std::static_pointer_cast<Receiver>(this->shared_from_this()), port));
        this->udp_server->run();
    }

    virtual void receive(const std::string& ip, unsigned short port, const std::string& data) override {
        BOOST_LOG_TRIVIAL(info) << "UDPTest::receive: Packet received from " + ip + ':' + std::to_string(port) + '\n' + data;
        std::string msg = "Packet of length " + std::to_string(data.length()) + " received from " + ip + ':' + std::to_string(port);

        this->udp_server->send(ip, port, msg);
    }

};


int main(int argc, char* argv[]) {
    BOOST_LOG_TRIVIAL(debug) << "main: Starting with " << argc << " arguments";
    unsigned short port = std::stoi(argv[1]);
    BOOST_LOG_TRIVIAL(debug) << "main: Port number " << port;
    std::shared_ptr<UDPTest> udp_test(new UDPTest());
    udp_test->start(port);
    udp_test->udp_server->stop();
    return 0;
}
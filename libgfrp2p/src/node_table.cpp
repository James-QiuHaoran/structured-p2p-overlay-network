#include "node_table.h"

std::string Packet::pack() const {
    return this->data;
}

// No need to disassemble in this particular case
void Packet::unpack(std::string datagram) {
    this->data = datagram;
}

void std::string Packet::get_data() const {
    return this->data
}


// Initialzie the udp server, but not run receiving
WireProtocol::WireProtocol(unsigned short port):
    udp_server(static_cast<Receiver*>(this), port) { }

void WireProtocol::run() {
    udp_server.run();
}

void WireProtocol::receive(const std::string& ip, unsigned short port, const std::string& data) override {
    // Unpack the UDP datagram
    std::unique_ptr<Packet> packet(new Packet());
    packet->unpack(data);
    std::string unpacked_data = packet->get_data();

    // Verify signature, etc.

    // Get corresponding node, create one if not exist

    // TODO: get command id and call corresponding function

}

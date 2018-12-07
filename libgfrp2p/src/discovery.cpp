#include "discovery.h"

std::string Packet::pack() const {
    return this->payload;
}
// No need to disassemble in this particular case
void Packet::unpack(std::string datagram) {
    this->data = datagram;
}

std::string Packet::get_payload() const {
    return this->payload;
}

void Packet::set_payload(std::string new_payload) {
    this->payload = new_payload;
}

/* Constructor */
Discovery::Discovery(const std::shared_ptr<NodeTable>& node_table, unsigned short port):
    udp_server(new AsyncUDPServer(static_cast<Receiver*>(this), port)), node_table(node_table) { }

/* Private functions */
void Discovery::receive(const std::string& ip, unsigned short port, const std::string& datagram) override {
    // Unpack the UDP datagram
    std::unique_ptr<Packet> packet(new Packet());
    packet->unpack(datagram);
    std::string unpacked_data = packet->get_payload();

    // Verify signature, etc.

    // Disassemble data
    std::string id = data.substr(0, 8);
    char cmd = data.at(8);
    std::string cmd_field = data.substr(9);

    // Call the corresponding Node Table logic
    if (cmd & CMD_PING) {
        receive_ping(ip, port);
    } else if (cmd & CMD_PONG) {
        receive_pong(id, ip, port);
    }

}

void Discovery::send(const std::string& ip, unsigned short port, std::string payload) {
    Packet packet;
    packet.set_data(payload);
    this->udp_server->send(ip, port, packet.pack());
}

void Discovery::send_ping(const std::string& id, const std::string& ip, unsigned short port) {
    std::string payload = this->node_table->get_self()->id + CMD_PING;
    this->send(ip, port, payload)
    if (this->node_table->has_node(id)) {
        this->node_table->set_node_last_ping_now(id);
    }
}

void Discovery::send_pong(const std::string& ip, unsigned short port) {
    std::string payload = this->node_table->get_self()->id + CMD_PONG;
    this->send(ip, port, payload)
}

void Discovery::receive_ping(const std::string& ip, unsigned short port) {
    send_pong(ip, port);
}

void Discovery::receive_pong(const std::string& id, const std::string& ip, unsigned short port) {
    if (this->node_table->has_node(id)) {
        this->node_table->set_node_last_pong_now(id);
    }
}



void Discovery::start() {
    this->udp_server.run();
}

void Discovery::stop() {
    // Disconnect to be implemented
}

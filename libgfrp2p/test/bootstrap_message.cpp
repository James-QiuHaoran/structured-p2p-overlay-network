#include <iostream>
#include <string>

#include "bootstrap_message.pb.h"

using namespace bootstrap_message;

int main() {

    BootstrapMessage msg, msg_;
    msg.set_type(BootstrapMessage::CONFIG);

    msg.config().set_run_id(10086);
    msg.config().set_eval_type(Config::HGFRR);
    msg.config().set_node_id("00001000010000100001000010000100");
    
    msg.config().set_table_size(2);
    msg.config().add_table_ids("00001000010000100001000010000101");
    msg.config().add_table_ips("192.168.100.2");
    msg.config().add_table_ports(30303)
    msg.config().add_table_ids("00001000010000100001000010000110");
    msg.config().add_table_ips("192.168.100.3");
    msg.config().add_table_ports(30303);

    msg.config().set_num_nodes_in_dist(10);
    

    std::string buffer;
    std::cout << "Serialization: " << msg.SerializeToString(&buffer) << std::endl;
    std::cout << "Parse: " << msg_.ParseFromString(buffer) << std::endl;

    std::cout << "Parsed object debug strig: " << msg.DebugString() << std::endl;
    return 0;
}
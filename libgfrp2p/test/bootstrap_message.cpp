#include <iostream>
#include <string>

#include "bootstrap_message.pb.h"

using namespace bootstrap_message;

int main() {

    BootstrapMessage msg, msg_;
    msg.set_type(BootstrapMessage::CONFIG);

    {    
        Config cfg;
        cfg.set_run_id(10086);
        cfg.set_eval_type(Config::HGFRR);
        cfg.set_node_id("00001000010000100001000010000100");
        
        cfg.set_table_size(2);
        cfg.add_table_ids("00001000010000100001000010000101")
        cfg.add_table_ips("192.168.100.2");
        cfg.add_table_ports(30303)
        cfg.add_table_ids("00001000010000100001000010000110")
        cfg.add_table_ips("192.168.100.3");
        cfg.add_table_ports(30303);

        cfg.set_num_nodes_in_dist(10);
        
        msg.set_config(cfg);
    }
    std::string buffer;
    std::cout << "Serialization: " << msg.SerializeToString(&buffer) << std::endl;
    std::cout << "Parse: " << msg_.ParseFromString(buffer) << std::endl;

    std::cout << "Parsed object debug strig: " << msg.DebugString() << std::endl;
    return 0;
}
#include <iostream>

#include "format.h"


int main() {
    ConfigFormatter fmt1, fmt2;
    fmt1.run_id = 19903;
    fmt1.node_id = "00001000010000100001000010000110";

    fmt1.num_nodes_in_dist = 1;
    fmt1.num_cnodes_in_dist = 2;
    fmt1.num_nodes_in_city = 3;
    fmt1.num_cnodes_in_city = 4;
    fmt1.num_nodes_in_state = 5;
    fmt1.num_cnodes_in_state = 6;
    fmt1.num_nodes_in_country = 7;
    fmt1.num_cnodes_in_country = 8;
    fmt1.num_nodes_in_continent = 9;

    fmt1.num_table_entries = 2;
    fmt1.table_entries["10001000010000100001000010000110"] = "192.168.100.100";
    fmt1.table_entries["11001000010000100001000010000110"] = "192.168.100.101";

    fmt2.from_string(fmt1.to_string());
    
    std::cout << (fmt1.to_string() == fmt2.to_string()) << std::endl;
    std::cout << fmt2.command_id + '0' << ' ' << fmt2.run_id << ' ' << fmt2.num_nodes_in_continent << ' ' << fmt2.num_table_entries << std::endl;
    
    for (const auto & kv:fmt2.table_entries){
        std::cout << kv.first << " -> " << kv.second;
    } 
    return 0;
}
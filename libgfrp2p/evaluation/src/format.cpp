#include "format.h"

std::string BaseFormatter::to_string() {
    return std::string() + command_id;
}

std::string BaseFormatter::from_string(const std::string& data) {
    if (!data.empty()) {
        command_id = data[0];
    }
    return data.substr(1);
}

InitFormatter::InitFormatter() { 
    command_id = 0x00;
}

std::string InitFormatter::to_string() {
    char c_port[sizeof(port)+1]; std::memcpy(c_port, &port, sizeof(port)); c_port[sizeof(port)] = '\0';
    
    return BaseFormatter::to_string() + c_port;
}

std::string InitFormatter::from_string(const std::string& data) {
    std::string specific = BaseFormatter::from_string(data);
    std::size_t offset = 0;
    if (specific.size() >= sizeof(port)) {
        std::memcpy(&port, specific.data(), sizeof(port));
        offset += sizeof(port);
    }
    return specific.substr(offset);
}

ConfigFormatter::ConfigFormatter() { 
    command_id = 0x01;
}

std::string ConfigFormatter::to_string() {
    char c_run_id[sizeof(run_id)+1]; std::memcpy(c_run_id, &run_id, sizeof(run_id)); c_run_id[sizeof(run_id)] = '\0'; 

    std::bitset<32> b_node_id(node_id);
    unsigned long l_node_id = b_node_id.to_ulong();
    char c_node_id[sizeof(l_node_id)+1]; std::memcpy(c_node_id, &l_node_id, sizeof(l_node_id)); c_node_id[sizeof(l_node_id)] = '\0'; 
    

    char c_num_nodes_in_dist[sizeof(num_nodes_in_dist)+1]; std::memcpy(c_num_nodes_in_dist, &num_nodes_in_dist, sizeof(num_nodes_in_dist)); c_num_nodes_in_dist[sizeof(num_nodes_in_dist)] = '\0';
    char c_num_cnodes_in_dist[sizeof(num_cnodes_in_dist)+1]; std::memcpy(c_num_cnodes_in_dist, &num_cnodes_in_dist, sizeof(num_cnodes_in_dist)); c_num_cnodes_in_dist[sizeof(num_cnodes_in_dist)] = '\0';
    char c_num_nodes_in_city[sizeof(num_nodes_in_city)+1]; std::memcpy(c_num_nodes_in_city, &num_nodes_in_city, sizeof(num_nodes_in_city)); c_num_nodes_in_city[sizeof(num_nodes_in_city)] = '\0';
    char c_num_cnodes_in_city[sizeof(num_cnodes_in_city)+1]; std::memcpy(c_num_cnodes_in_city, &num_cnodes_in_city, sizeof(num_cnodes_in_city)); c_num_cnodes_in_city[sizeof(num_cnodes_in_city)] = '\0';
    char c_num_nodes_in_state[sizeof(num_nodes_in_state)+1]; std::memcpy(c_num_nodes_in_state, &num_nodes_in_state, sizeof(num_nodes_in_state)); c_num_nodes_in_state[sizeof(num_nodes_in_state)] = '\0';
    char c_num_cnodes_in_state[sizeof(num_cnodes_in_state)+1]; std::memcpy(c_num_cnodes_in_state, &num_cnodes_in_state, sizeof(num_cnodes_in_state)); c_num_cnodes_in_state[sizeof(num_cnodes_in_state)] = '\0';
    char c_num_nodes_in_country[sizeof(num_nodes_in_country)+1]; std::memcpy(c_num_nodes_in_country, &num_nodes_in_country, sizeof(num_nodes_in_country)); c_num_nodes_in_country[sizeof(num_nodes_in_country)] = '\0';
    char c_num_cnodes_in_country[sizeof(num_cnodes_in_country)+1]; std::memcpy(c_num_cnodes_in_country, &num_cnodes_in_country, sizeof(num_cnodes_in_country)); c_num_cnodes_in_country[sizeof(num_cnodes_in_country)] = '\0';
    char c_num_nodes_in_continent[sizeof(num_nodes_in_continent)+1]; std::memcpy(c_num_nodes_in_continent, &num_nodes_in_continent, sizeof(num_nodes_in_continent)); c_num_nodes_in_continent[sizeof(num_nodes_in_continent)] = '\0';

    char c_num_table_entries[sizeof(num_table_entries)+1]; std::memcpy(c_num_table_entries, &num_table_entries, sizeof(num_table_entries)); c_num_table_entries[sizeof(num_table_entries)] = '\0';

    std::string result = c_run_id;
    result = result + c_node_id + c_num_nodes_in_dist + c_num_cnodes_in_dist 
        + c_num_nodes_in_city + c_num_cnodes_in_city
        + c_num_nodes_in_state + c_num_cnodes_in_state
        + c_num_nodes_in_country + c_num_cnodes_in_country
        + c_num_nodes_in_continent;
    
    for (const auto& v:table_entries) {
        std::bitset<32> b_id(v.first);
        unsigned long l_id = b_id.to_ulong();
        char c_id[sizeof(l_id)+1]; std::memcpy(c_id, &l_id, sizeof(l_id)); c_id[sizeof(l_id)] = '\0'; 
        result += c_id;

        std::uint32_t ip;
        inet_pton(AF_INET, v.second.c_str(), &ip);
        char c_ip[sizeof(ip)+1]; std::memcpy(c_ip, &ip, sizeof(ip)); c_ip[5] = '\0';
        result += c_ip;
    }


    return BaseFormatter::to_string() + result;
}

std::string ConfigFormatter::from_string(const std::string& data) {
    std::string specific = BaseFormatter::from_string(data);
    if (specific.size() < 11*sizeof(uint32_t) + sizeof(unsigned long)) {
        return "ERROR";
    }
    std::size_t offset = 0;

    std::memcpy(&run_id, specific.data(), sizeof(run_id));
    offset += sizeof(run_id);

    unsigned long l_id;
    std::memcpy(&l_id, specific.data() + offset, sizeof(unsigned long));
    offset += sizeof(unsigned long);
    std::bitset<32> b_id(l_id);
    node_id = b_id.to_string();
    
    std::memcpy(&num_nodes_in_dist, specific.data() + offset, sizeof(std::uint32_t)); offset += sizeof(std::uint32_t);
    std::memcpy(&num_cnodes_in_dist, specific.data() + offset, sizeof(std::uint32_t)); offset += sizeof(std::uint32_t);
    std::memcpy(&num_nodes_in_city, specific.data() + offset, sizeof(std::uint32_t)); offset += sizeof(std::uint32_t);
    std::memcpy(&num_cnodes_in_city, specific.data() + offset, sizeof(std::uint32_t)); offset += sizeof(std::uint32_t);
    std::memcpy(&num_nodes_in_state, specific.data() + offset, sizeof(std::uint32_t)); offset += sizeof(std::uint32_t);
    std::memcpy(&num_cnodes_in_state, specific.data() + offset, sizeof(std::uint32_t)); offset += sizeof(std::uint32_t);
    std::memcpy(&num_nodes_in_country, specific.data() + offset, sizeof(std::uint32_t)); offset += sizeof(std::uint32_t);
    std::memcpy(&num_cnodes_in_country, specific.data() + offset, sizeof(std::uint32_t)); offset += sizeof(std::uint32_t);
    std::memcpy(&num_nodes_in_continent, specific.data() + offset, sizeof(std::uint32_t)); offset += sizeof(std::uint32_t);

    std::memcpy(&num_nodes_in_continent, specific.data() + offset, sizeof(std::uint32_t)); offset += sizeof(std::uint32_t);

    while (specific.size() - offset > sizeof(unsigned long) + sizeof(std::uint32_t)) {
        std::string node_id_;
        unsigned long l_id_;
        std::memcpy(&l_id_, specific.data() + offset, sizeof(unsigned long));
        offset += sizeof(unsigned long);
        std::bitset<32> b_id_(l_id_);
        node_id_ = b_id_.to_string();
        
        std::uint32_t ip_;
        std::memcpy(&ip_, specific.data() + offset, sizeof(std::uint32_t));
        offset +=  sizeof(std::uint32_t);
        char s_ip_[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ip_, s_ip_);
        std::string node_ip_(s_ip_);

        table_entries.insert(std::make_pair(node_id_, node_ip));

    }

    return specific.substr(offset);
}

ConfigAckFormatter::ConfigAckFormatter() { 
    command_id = 0x02;
}

std::string ConfigAckFormatter::to_string() {
    return BaseFormatter::to_string() + status;
}

std::string ConfigAckFormatter::from_string(const std::string& data) {
    std::string specific = BaseFormatter::from_string(data);

    if (!data.empty()) {
        status = data[0];
    }
    return data.substr(1);

}

BroadcastFormatter::BroadcastFormatter() { 
    command_id = 0x03;
}

std::string BroadcastFormatter::to_string() {
    char c_workload_size[sizeof(workload_size)+1];
    std::memcpy(c_workload_size, &workload_size, sizeof(workload_size));
    c_workload_size[sizeof(workload_size)] = '\0';

    return BaseFormatter::to_string() + c_workload_size;
}

std::string BroadcastFormatter::from_string(const std::string& data) {
    std::string specific = BaseFormatter::from_string(data);
    std::size_t offset = 0;
    if (specific.size() >= sizeof(workload_size)) {
        std::memcpy(&workload_size, specific.data(), sizeof(workload_size));
        offset += sizeof(workload_size);
    }
    return specific.substr(offset);
}

PullLogFormatter::PullLogFormatter() { 
    command_id = 0x05;
}

std::string PullLogFormatter::to_string() {
    return BaseFormatter::to_string();
}

std::string PullLogFormatter::from_string(const std::string& data) {
    std::string specific = BaseFormatter::from_string(data);
    return specific;
}

PushLogFormatter::PushLogFormatter() { 
    command_id = 0x06;
}

std::string PushLogFormatter::to_string() {
    return BaseFormatter::to_string() + log ;
}

std::string PullLogFormatter::from_string(const std::string& data) {
    log = BaseFormatter::from_string(data);
    return std::string();
}
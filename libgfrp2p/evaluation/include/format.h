#ifndef FORMAT_H
#define FORMAT_H

#include <string>
#include <list>

#include <cstdint>

class BaseFormatter {
public: 
    char command_id;

    virtual std::string to_string(); 
    virtual void from_string(const std::string& datagram);
    
};

class InitiateFormatter {
public: 
    std::uint16_t port;
    
};

class ConfigFormatter: public BaseFormatter {
public:
    std::uint32_t run_id;

    char id[4];

    std::uint32_t num_nodes_in_dist;
    std::uint32_t num_cnodes_in_dist;
    std::uint32_t num_nodes_in_city;
    std::uint32_t num_cnodes_in_city;
    std::uint32_t num_nodes_in_state;
    std::uint32_t num_cnodes_in_state;
    std::uint32_t num_nodes_in_country;
    std::uint32_t num_cnodes_in_country;
    std::uint32_t num_nodes_in_continent;

    std::uint32_t num_table_entries;
    std::list<std::pair<std::string, std::string>> table_entries;
    
};

class ConfigAckFormatter: public BaseFormatter {
public:

    char status;
};


class BroadcastFormatter: public BaseFormatter {
public:
    uint32_t workload_size;
};

class PullLogFormatter: public BaseFormatter {
public:
    uint32_t workload_size;
};


class PullLogFormatter: public BaseFormatter {
    // No commnad-specific fields
};

class PushLogFormatter: public BaseFormatter {
public:
    std::string log;
};

#endif
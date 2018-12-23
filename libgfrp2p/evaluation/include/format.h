#ifndef FORMAT_H
#define FORMAT_H

#include <string>
#include <bitset>
#include <list>

#include <cstdio>
#include <cstdint>
#include <cstring>

#include <arpa/inet.h>

class BaseFormatter {
public: 
    char command_id;

    virtual std::string to_string(); 
    virtual std::string from_string(const std::string& data);
    
};

class InitFormatter: public BaseFormatter {
public: 
    std::uint16_t port;
    
    virtual std::string to_string() override; 
    virtual std::string from_string(const std::string& data) override;
};

class ConfigFormatter: public BaseFormatter {
public:
    std::uint32_t run_id;

    std::string node_id;

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

    virtual std::string to_string() override; 
    virtual std::string from_string(const std::string& data) override;
    
};

class ConfigAckFormatter: public BaseFormatter {
public:

    char status;

    virtual std::string to_string() override; 
    virtual std::string from_string(const std::string& data) override;
};


class BroadcastFormatter: public BaseFormatter {
public:
    uint32_t workload_size;

    virtual std::string to_string() override; 
    virtual std::string from_string(const std::string& data) override;
};

class PullLogFormatter: public BaseFormatter {
public:
    uint32_t workload_size;

    virtual std::string to_string() override; 
    virtual std::string from_string(const std::string& data) override;
};


class PullLogFormatter: public BaseFormatter {
    // No commnad-specific fields

    virtual std::string to_string() override; 
    virtual std::string from_string(const std::string& data) override;
};

class PushLogFormatter: public BaseFormatter {
public:
    std::string log;

    virtual std::string to_string() override; 
    virtual std::string from_string(const std::string& data) override;
};

#endif
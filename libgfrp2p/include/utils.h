#ifndef UTILS_H
#define UTILS_H

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <time.h>
#include <iostream>

#include "param_constants.h"

std::string random_string(size_t length);

std::string convert_ID_int_to_string(int id,
    int num_nodes_in_dist, int num_cnodes_in_dist, 
    int num_nodes_in_city, int num_cnodes_in_city, 
    int num_nodes_in_state, int num_cnodes_in_state, 
    int num_nodes_in_country, int num_cnodes_in_country, 
    int num_nodes_in_continent);

int convert_ID_string_to_int(const std::string& id,
    int num_nodes_in_dist, int num_cnodes_in_dist, 
    int num_nodes_in_city, int num_cnodes_in_city, 
    int num_nodes_in_state, int num_cnodes_in_state, 
    int num_nodes_in_country, int num_cnodes_in_country, 
    int num_nodes_in_continent);


struct EvalConfig {
	std::string run_id;

    std::string str_node_id;

	std::uint32_t eval_type;

	std::uint32_t num_nodes_in_dist;
    std::uint32_t num_cnodes_in_dist;
    std::uint32_t num_nodes_in_city;
    std::uint32_t num_cnodes_in_city;
    std::uint32_t num_nodes_in_state;
    std::uint32_t num_cnodes_in_state;
    std::uint32_t num_nodes_in_country;
    std::uint32_t num_cnodes_in_country;
	std::uint32_t num_nodes_in_continent;
	std::uint32_t num_cnodes_in_continent;
	std::uint32_t num_continents;
};

#endif
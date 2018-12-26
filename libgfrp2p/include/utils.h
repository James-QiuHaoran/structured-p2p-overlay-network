#include <sstream>
#include <iomanip>

#include "param_constants.h"

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
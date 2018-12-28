#include "utils.h"

// HASH - generate a random alpha-numeric string of length len
std::string random_string(size_t length) {
    srand ( time(NULL) );
    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}

// convert ID int to string
std::string convert_ID_int_to_string(int id,
    int num_nodes_in_dist, int num_cnodes_in_dist, 
    int num_nodes_in_city, int num_cnodes_in_city, 
    int num_nodes_in_state, int num_cnodes_in_state, 
    int num_nodes_in_country, int num_cnodes_in_country, 
    int num_nodes_in_continent) {

    int num_nodes_in_one_dist = num_nodes_in_dist;
    int num_dists_in_city = num_nodes_in_city/num_cnodes_in_dist;
    int num_nodes_in_one_city = num_nodes_in_one_dist * num_dists_in_city;
    int num_cities_in_state = num_nodes_in_state/num_cnodes_in_dist;
    int num_nodes_in_one_state = num_nodes_in_one_city * num_cities_in_state;
    int num_states_in_country = num_nodes_in_country/num_cnodes_in_state;
    int num_nodes_in_one_country = num_nodes_in_one_state * num_states_in_country;
    int num_countries_in_continent = num_nodes_in_continent/num_cnodes_in_country;
    int num_nodes_in_one_continent = num_nodes_in_one_country * num_countries_in_continent;

    std::stringstream ss;
    std::string id_string = "";
    int i = 0;

    i = id % num_nodes_in_one_dist;
    ss << std::setw(ID_SINGLE_LEN) << std::setfill('0') << i;
    std::string peer_id = ss.str();
    id_string += peer_id;

    ss.str("");
    ss.clear();
    i = id / num_nodes_in_one_dist % num_dists_in_city;
    ss << std::setw(ID_DISTRICT_LEN) << std::setfill('0') << i;
    std::string dist_id = ss.str();
    id_string = dist_id + id_string;

    ss.str("");
    ss.clear();
    i = id / num_nodes_in_one_city % num_cities_in_state;
    ss << std::setw(ID_CITY_LEN) << std::setfill('0') << i;
    std::string city_id = ss.str();
    id_string = city_id + id_string;

    ss.str("");
    ss.clear();
    i = id / num_nodes_in_one_state % num_states_in_country;
    ss << std::setw(ID_STATE_LEN) << std::setfill('0') << i;
    std::string state_id = ss.str();
    id_string = state_id + id_string;

    ss.str("");
    ss.clear();
    i = id / num_nodes_in_one_country % num_countries_in_continent;
    ss << std::setw(ID_COUNTRY_LEN) << std::setfill('0') << i;
    std::string country_id = ss.str();
    id_string = country_id + id_string;

    ss.str("");
    ss.clear();
    i = id / num_nodes_in_one_continent;
    ss << std::setw(ID_CONTINENT_LEN) << std::setfill('0') << i;
    std::string continent_id = ss.str();
    id_string = continent_id + id_string;

    return id_string;
}

// convert ID string to int
int convert_ID_string_to_int(const std::string& id,
    int num_nodes_in_dist, int num_cnodes_in_dist, 
    int num_nodes_in_city, int num_cnodes_in_city, 
    int num_nodes_in_state, int num_cnodes_in_state, 
    int num_nodes_in_country, int num_cnodes_in_country, 
    int num_nodes_in_continent) {
    
    std::string id_in_dist = id.substr(ID_SINGLE_START);
    std::string dist_id = id.substr(ID_DISTRICT_START, ID_DISTRICT_LEN);
    std::string city_id = id.substr(ID_CITY_START, ID_CITY_LEN);
    std::string state_id = id.substr(ID_STATE_START, ID_STATE_LEN);
    std::string country_id = id.substr(ID_COUNTRY_START, ID_COUNTRY_LEN);
    std::string continent_id = id.substr(ID_CONTINENT_START, ID_CONTINENT_LEN);

    int node_id_in_dist = 0, dist_id_int = 0, city_id_int = 0, state_id_int = 0, country_id_int = 0, continent_id_int = 0;
    
    std::stringstream ss_node(id_in_dist);
    ss_node >> node_id_in_dist;

    std::stringstream ss_dist(dist_id);
    ss_dist >> dist_id_int;
    int num_nodes_in_one_dist = num_nodes_in_dist;

    int num_dists_in_city = num_nodes_in_city/num_cnodes_in_dist;
    std::stringstream ss_city(city_id);
    ss_city >> city_id_int;
    int num_nodes_in_one_city = num_nodes_in_one_dist * num_dists_in_city;

    int num_cities_in_state = num_nodes_in_state/num_cnodes_in_dist;
    std::stringstream ss_state(state_id);
    ss_state >> state_id_int;
    int num_nodes_in_one_state = num_nodes_in_one_city * num_cities_in_state;

    int num_states_in_country = num_nodes_in_country/num_cnodes_in_state;
    std::stringstream ss_country(country_id);
    ss_country >> country_id_int;
    int num_nodes_in_one_country = num_nodes_in_one_state * num_states_in_country;

    int num_countries_in_continent = num_nodes_in_continent/num_cnodes_in_country;
    std::stringstream ss_continent(continent_id);
    ss_continent >> continent_id_int;
    int num_nodes_in_one_continent = num_nodes_in_one_country * num_countries_in_continent;

    int num = num_nodes_in_one_continent * continent_id_int +
                   num_nodes_in_one_country * country_id_int +
                   num_nodes_in_one_state * state_id_int +
                   num_nodes_in_one_city * city_id_int +
                   num_nodes_in_one_dist * dist_id_int +
                   node_id_in_dist;

    return num;
}
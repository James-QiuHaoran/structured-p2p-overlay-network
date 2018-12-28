#!/bin/bash

echo $'== ETH Evaluation ==\n'

# structure setting
# |xxx|xxxx|xxxxx|xxxxxx|xxxxx|xxxxxxxxx| - 32 bits in total
continent0="000"
country0="0000"
state0="00000"
city0="000000"
district0="00000"
node0="000000000"

num_countries_in_a_continent=1
num_states_in_a_country=1
num_cities_in_a_state=3
num_districts_in_a_city=6
num_nodes_in_a_district=10

num_continents=1
num_countries=$num_continents*$num_countries_in_a_continent
num_states=$num_countries*$num_states_in_a_country
num_cities=$num_states*$num_cities_in_a_state
num_districts=$num_cities*num_districts_in_a_city
num_nodes=$num_districts*num_nodes_in_a_district

echo "Topology stats:"

printf "# of continents: $num_continents;\n"
printf "# of countries: $num_countries = "
eval "num_countries_total=\$(( $num_countries ))"
echo "$num_countries_total;"
printf "# of states: $num_states = "
eval "num_states_total=\$(( $num_states ))"
echo "$num_states_total;"
printf "# of cities: $num_cities = "
eval "num_cities_total=\$(( $num_cities ))"
echo "$num_cities_total;"
printf "# of districts: $num_districts = "
eval "num_districts_total=\$(( $num_districts ))"
echo "$num_districts_total;"
printf "# of nodes: $num_nodes = "
eval "num_nodes_total=\$(( $num_nodes ))"
printf "$num_nodes_total;\n\n"

eval "num_nodes_in_a_continent=\$(( ($num_nodes_total/$num_continents) ))"
eval "num_nodes_in_a_country=\$(( ($num_nodes_total/$num_countries_total) ))"
eval "num_nodes_in_a_state=\$(( ($num_nodes_total/$num_states_total) ))"
eval "num_nodes_in_a_city=\$(( ($num_nodes_total/$num_cities_total) ))"
eval "num_nodes_in_a_district=\$(( ($num_nodes_total/$num_districts_total) ))"

echo "# of nodes in a continent: $num_nodes_in_a_continent"
echo "# of nodes in a country: $num_nodes_in_a_country"
echo "# of nodes in a state: $num_nodes_in_a_state"
echo "# of nodes in a city: $num_nodes_in_a_city"
echo "# of nodes in a district: $num_nodes_in_a_district"
echo ""

echo "== Boostrap Stage =="

start_time=$(date +"%Y-%m-%d-%H-%M-%S")
mkdir -p log/$start_time

starting_port_num="2180"

for (( i=0; i<$num_nodes_total; i++ ))
do
	printf "\nBoostrapping node #$i ... \n"

	# set id
	eval "continent_id=\$(( $continent0 + $i/$num_nodes_in_a_continent ))"
	continent_id=$(printf "%03d" $continent_id)

	eval "country_id=\$(( $country0 + ($i/$num_nodes_in_a_country)%$num_countries_in_a_continent ))"
	if [ "$num_countries_in_a_continent" -eq "1" ]
	then
		eval "country_id=\$(( $country0 + $i/$num_nodes_in_a_country ))"
	fi
	country_id=$(printf "%04d" $country_id)

	eval "state_id=\$(( $state0 + ($i/$num_nodes_in_a_state)%$num_states_in_a_country ))"
	if [ "$num_states_in_a_country" -eq "1" ]
	then
		eval "state_id=\$(( $state0 + $i/$num_nodes_in_a_state ))"
	fi
	state_id=$(printf "%05d" $state_id)

	eval "city_id=\$(( $city0 + ($i/$num_nodes_in_a_city)%$num_cities_in_a_state ))"
	if [ "$num_cities_in_a_state" -eq "1" ]
	then
		eval "city_id=\$(( $city0 + $i/$num_nodes_in_a_city ))"
	fi
	city_id=$(printf "%06d" $city_id)

	eval "district_id=\$(( $district0 + ($i/$num_nodes_in_a_district)%$num_districts_in_a_city ))"
	if [ "$num_districts_in_a_city" -eq "1" ]
	then
		eval "district_id=\$(( $district0 + $i/$num_nodes_in_a_district ))"
	fi
	district_id=$(printf "%05d" $district_id)

	eval "node_id=\$(( $node0 + $i % $num_nodes_in_a_district ))"
	node_id=$(printf "%09d" $node_id)

	id_to_display="$continent_id,$country_id,$state_id,$city_id,$district_id,$node_id"
	id="$continent_id$country_id$state_id$city_id$district_id$node_id"
	printf "Node ID = $id_to_display\n"

	# set ip
	ip=127.0.0.1
	printf "IP: $ip\n"

	# set port
	eval "port=\$(( $starting_port_num + $i ))"
	printf "Port: $port\n"

	num_nodes_in_dist=$num_nodes_in_a_district
	num_cnodes_in_dist=2
	num_nodes_in_city=12
	num_cnodes_in_city=2
	num_nodes_in_state=6
	num_cnodes_in_state=2
	num_nodes_in_country=1
	num_cnodes_in_country=1
	num_nodes_in_continent=1
	num_cnodes_in_continent=1
	num_continents=1

	../bin/app_eth $ip $port $id \
		$num_nodes_in_dist $num_cnodes_in_dist \
		$num_nodes_in_city $num_cnodes_in_city \
		$num_nodes_in_state $num_cnodes_in_state \
		$num_nodes_in_country $num_cnodes_in_country \
		$num_nodes_in_continent $num_cnodes_in_continent \
		$num_continents \
		$starting_port_num $start_time &
done

sleep 2

printf "\n== Successfully boostrapped for $num_nodes_total nodes! ==\n"
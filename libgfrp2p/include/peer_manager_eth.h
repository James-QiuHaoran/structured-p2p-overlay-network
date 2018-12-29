#ifndef PEER_MANAGER_ETH_H
#define PEER_MANAGER_ETH_H

#include <string>
#include <unordered_set>
#include <cstdlib>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include <functional>
#include <ctime>

#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>

#include "node.h"
#include "node_table_eth.h"
#include "transport.h"
#include "message.h"
#include "param_constants.h"
#include "utils.h"

/* Error Class
 * define error types and error messages
 */
class PeerError {
private:
    std::string errorType;
    std::string errorMessage;

public:
    // constructor
    PeerError();
    PeerError(std::string errorType, std::string errorMessage);
    
    // getters
    std::string get_errorType() const;
    std::string get_errorMessage() const;

    // setters
    void set_errorType(std::string errorType);
    void set_errorMessage(std::string errorMessage);
};

/* PeerManager class -- ethereum version
 * responsible for broadcasting messages, Kademlia is used to form the routing table of each node
 */
class PeerManagerETH: public Receiver, public std::enable_shared_from_this<PeerManagerETH> {
private:
    // contains node information
    std::shared_ptr<Node> node;

    // contains peers and related information
    std::shared_ptr<NodeTableETH> node_table;

    // TCP server
    // AsyncTCPServer* tcp_server;

    // UDP server
    AsyncUDPServer* tcp_server;

    // seed for random number generator
    boost::random::mt19937 gen;

    // for message logging, contains all sent and received messages
    MessageTable msg_table;
    std::vector<std::string> broadcasted_msgs;
    std::vector<std::string> broadcasted_msgs_all_nodes; // store broadcast_id
    std::string start_time;

    // broadcast mode
    unsigned short mode;

    // messaged received <data_hash, data_content>
    std::unordered_map<std::string, std::string> data_map;

public:
    // two modes of gossip broadcast
    static const unsigned short PUSH = 0;
    static const unsigned short PULL = 1;

    // constructors
    PeerManagerETH();
    PeerManagerETH(unsigned short port);
    PeerManagerETH(const std::shared_ptr<Node>& node, const std::shared_ptr<NodeTableETH>& node_table, const std::string &start_time);

    // getters
    std::shared_ptr<Node> get_node();
    std::shared_ptr<NodeTableETH> get_node_table();

    // setters
    void set_node(std::shared_ptr<Node> node);
    void set_node_table(std::shared_ptr<NodeTableETH> node_table);
    void set_mode(unsigned short mode);
    
    // start the server
    void start();

    // broadcast a message
    void broadcast(const std::string &data, int ttl, std::string broadcast_id);
    void send(std::shared_ptr<Node> node, const Message &msg, const std::string &data);
    void send_inv(std::shared_ptr<Node> node, const std::string &data_hash, const std::string &broadcast_id);
    void send_data(std::shared_ptr<Node> node, const std::string &data, const std::string &broadcast_id);
    void send_request(std::shared_ptr<Node> node, const std::string &data_hash, const std::string &broadcast_id);

    // override the receive() func inherited from Receiver
    virtual void receive(const std::string& ip, unsigned short port, const std::string &data) override;

    // on receive a message
    void on_receive(const Message &msg, const std::string &data, const std::string& ip, unsigned short port);

    // on a node join
    void on_new_connection(std::shared_ptr<Node> node);

    // on a node leave
    void on_lost_connection(std::shared_ptr<Node> node);

    // stop the peer
    void stop();

    /* helper classes (if any) */

    // generate random number uniformly from range [low, high]
    int random_num_in_range(int low, int high);

    // generate random string of length
    std::string random_string_of_length(size_t length);

    // write messages to file system
    void log_message_records();
    void append_message_record(const Message& msg);
};

#endif
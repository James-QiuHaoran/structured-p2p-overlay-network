#ifndef PEER_MANAGER_H
#define PEER_MANAGER_H

#include <string>
#include <unordered_set>
#include <cstdlib>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>

#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>

#include "node.h"
#include "node_table_eth.h"
#include "transport.h"
#include "message.h"
#include "param_constants.h"

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

/* PeerManager class
 * responsible for broadcasting messages
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
    std::string start_time;

public:
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
    
    // start the server
    void start();

    // broadcast/multicast a message
    void broadcast(const std::string &data);
    void send(std::shared_ptr<Node> node, const Message &msg, const std::string &data);

    // override the receive() func inherited from Receiver
    virtual void receive(const std::string& ip, unsigned short port, const std::string &data) override;

    // on receive a message
    void on_receive(const Message &msg, const std::string &data);

    // on a node join
    void on_new_connection(std::shared_ptr<Node> node);

    // on a node leave
    void on_lost_connection(std::shared_ptr<Node> node);

    // stop the peer
    void stop();

    /* helper classes (if any) */

    // generate random number uniformly from range [low, high]
    int random_num_in_range(int low, int high);

    // write messages to file system
    void log_message_records();
    void append_message_record(const Message& msg);
};

#endif
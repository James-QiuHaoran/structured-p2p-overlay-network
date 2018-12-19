#ifndef PEER_MANAGER_H
#define PEER_MANAGER_H

#include <string>
#include <unordered_set>
#include <cstdlib>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "node.h"
#include "node_table.h"
#include "transport.h"
#include "param_constants.h"

#define DEBUG 0

boost::random::mt19937 gen;  // seed for random number generator

/* Message class
 * definition of messages transmitted among peers
 */
class Message {
private:
    std::string message_id;
    int type, node_order;
    unsigned long from_level;
    std::string sender_id;
    std::string receiver_id;

public:
    // constructor
    Message();
    Message(std::string messageID, int type, unsigned long from_level, std::string sender_id, std::string receiver_id);

    // getters
    std::string get_sender_id() const;
    std::string get_receiver_id() const;
    unsigned long get_from_level() const;
    std::string get_message_id() const;
    int get_type() const;
    int get_node_order() const;

    // setters
    void set_sender_id(const std::string &sender_id);
    void set_receiver_id(const std::string &receiver_id);
    void set_from_level(unsigned long level);
    void set_message_id(std::string message_id);
    void set_type(int type);
    void set_node_order(int order);
};

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
class PeerManager: public Receiver {
private:
    // contains node information
    std::shared_ptr<Node> node;

    // contains peers and related information
    std::shared_ptr<NodeTable> node_table;

    // TCP server
    AsyncTCPServer* tcp_server;

public:
    // constructors
    PeerManager();
    PeerManager(unsigned short port);
    PeerManager(Node node, NodeTable node_table);

    // getters
    std::shared_ptr<Node> get_node();
    std::shared_ptr<NodeTable> get_node_table();

    // setters
    void set_node(std::shared_ptr<Node> node);
    void set_node_table(std::shared_ptr<NodeTable> node_table);
    
    // start the server
    void start();

    // broadcast/multicast a message
    void broadcast(Message msg, unsigned long current_level);
    void broadcast_up(Message msg, unsigned long current_level);
    void broadcast_within_ring(Message msg, unsigned long current_level, int k);
    void broadcast_down(Message msg, unsigned long current_level);
    void multicast_to_contact_nodes(Message msg, unsigned long current_level);
    void send(std::shared_ptr<Node> node, const Message &msg);

    void receive(const std::string& ip, unsigned short port, const std::string &data);

    void on_receive(const Message &msg);

    // on a node join
    void on_new_connection(std::shared_ptr<Node> node);

    // on a node leave
    void on_lost_connection(std::shared_ptr<Node> node);

    // contact nodes election
    void contact_node_election(unsigned long level);

    // stop the peer
    void stop();
};

#endif

/*
    // To be put in node_table
    // detect node leave
    void on_node_lost_connection();
    void on_node_leave();
    void detect_node_left();
    bool liveness_check_predecessor();
    bool liveness_check_successor();
    bool check_your_predecessor();
    bool check_your_sucessor();
*/

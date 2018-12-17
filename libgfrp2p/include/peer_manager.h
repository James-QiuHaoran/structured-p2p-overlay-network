#ifndef PEER_MANAGER_H
#define PEER_MANAGER_H

#include <string>
#include <unordered_set>
#include <cstdlib>
#include <iostream>
#include <math.h>

#include "node.h"
#include "node_table.h"
#include "app.h"

/* Message class
 * definition of messages transmitted among peers
 */
class Message {
private:
    std::string messageID;
    int type, node_id;
    unsigned long from_level;
    std::shared_ptr<Node> sender;
    std::shared_ptr<Node> receiver;

public:
    // constructor
    Message();
    Message(std::string messageID, int type, unsigned long from_level, std::shared_ptr<Node> sender, std::shared_ptr<Node> receiver);

    // getters
    std::shared_ptr<Node> get_sender() const;
    std::shared_ptr<Node> get_receiver() const;
    unsigned long get_from_level() const;
    std::string get_messageID() const;
    int get_type() const;
    int get_node_id() const;

    // setters
    void set_sender(const std::shared_ptr<Node> &sender);
    void set_receiver(const std::shared_ptr<Node> &receiver);
    void set_from_level(unsigned long level);
    void set_messageID(std::string messageId);
    void set_type(int type);
    void set_node_id(int id);
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
class PeerManager {
private:
    // contains node information
    std::shared_ptr<Node> node;

    // contains peers and related information
    NodeTable node_table;

public:
    // constructor
    PeerManager(const std::string& self_id);

    // getters
    std::shared_ptr<Node> get_node();
    NodeTable get_node_table();

    // setters
    void set_node(std::shared_ptr<Node> node);
    void set_node_table(NodeTable node_table);
    
    // create and initialize a peer
    void create_peer();

    // connect to the network
    void connect();

    // start the server
    void start();

    // broadcast/multicast a message
    void broadcast(Message msg, unsigned long current_level);
    void broadcast_up(Message msg, unsigned long current_level);
    void broadcast_within_ring(Message msg, unsigned long current_level, int k);
    void broadcast_down(Message msg, unsigned long current_level);
    void multicast_to_contact_nodes(Message msg, unsigned long current_level);
    void send(std::shared_ptr<Node> node, const Message &msg);
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

#include <string>
#include <unordered_set>

#include "node.h"
#include "node_table.h"

/* Message class
 * definition of messages transmitted among peers
 */
class Message {
private:
    std::string sender;
    std::string receiver;
    std::string messageID;
    std::string messageHash;
    std::string type;

public:
    // constructor
    Message();

    // getters
    std::string get_sender() const;
    std::string get_receiver() const;
    std::string get_messageID() const;
    std::string get_messageHash() const;
    std::string get_type() const;

    // setters
    void set_sender(string sender);
    void set_receiver(string receiver);
    void set_messageID(string messageId);
    void set_messageHash(string messageHash);
    void set_type(string type);
};

/* Error Class
 * define error types and error messages
 *
 */
class PeerError {
private:
    std::string errorType;
    std::string errorMessage;

public:
    // constructor
    PeerError();
    
    // getters
    std::string get_errorType();
    std::string get_errorMessage();

    // setters
    void set_errorType();
    void set_errorMessage();
};

/* PeerManager class
 * responsible for broadcasting messages
 */
class PeerManager {
private:
    // contains node information
    Node node;

    // contains peers and related information
    NodeTable node_table;

public:
    // constructor
    void PeerManager();

    // getters
    Node get_node();
    NodeTable get_node_table();
    
    // create and initialize a peer
    void create_peer();

    // connect to the network
    void connect();

    // start the server
    void start();

    // broadcast a message
    void broadcast(const Message &msg);
    void broadcast_up(const Message &msg, unsigned long current_level);
    void broadcast_down(const Message &msg, unsigned long current_level);
    void send(const Node &node, const Message &msg);
    void on_receive(const Message &msg);

    // stop the peer
    void stop();
};

// To be put in node_table
/*
    // on joining a node
    void on_new_connection();

    // detect node leave
    void on_node_lost_connection();
    void on_node_leave();
    void detect_node_left();
    bool liveness_check_predecessor();
    bool liveness_check_successor();
    bool check_your_predecessor();
    bool check_your_sucessor();

    // contact node(s) election
    std::unordered_set<Node> contact_node_election();
    void election_result_broadcast();
*/
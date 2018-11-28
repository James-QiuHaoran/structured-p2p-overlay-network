#include <string>
#include "node.h"
#include "app.h"

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

/* PeerManager class
 * responsible for broadcasting messages
 */
class PeerManager {
private:
    // peers[]
    // listen address
    // server

public:
    // create and initialize a peer
    void create_peer();

    // connect to the network
    void connect();

    // start the server
    void start();

    // broadcast a message
    void broadcast(Message msg);

    // on joining a node
    void on_new_connection();

    // stop the peer
    void stop();
};
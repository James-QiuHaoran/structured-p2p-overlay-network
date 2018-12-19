#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>

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

#endif
#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <utility>
#include <unordered_map>
#include <chrono>
#include <cstdio>


using message_key_t = unsigned long;

/* Message class
 * definition of messages transmitted among peers
 */
class Message {
    friend class MessageTable;
private:
    unsigned long io_timestamp;
    unsigned short io_type;
    
    std::string message_id;
    int type, node_order;
    unsigned long from_level;
    std::string sender_id;
    std::string receiver_id;
    // std::string data;


public:
    static constexpr const char* csv_header = "io_timestamp,io_type,sender_id,message_id,receiver_id,type,from_level,node_order";

    static const unsigned short IO_TYPE_RECEIVED = 0;
    static const unsigned short IO_TYPE_SENT = 1;

    // constructor
    Message();
    Message(std::string messageID, int type, unsigned long from_level, std::string sender_id, std::string receiver_id);
    Message(unsigned short io_type, std::string messageID, int type, unsigned long from_level, std::string sender_id, std::string receiver_id);

    // DB semantic
    message_key_t get_key() const;
    std::string to_csv_string() const;

    // getters
    std::string get_sender_id() const;
    std::string get_receiver_id() const;
    unsigned long get_from_level() const;
    std::string get_message_id() const;
    int get_type() const;
    int get_node_order() const;
    std::string get_data() const;

    // setters
    void set_sender_id(const std::string &sender_id);
    void set_receiver_id(const std::string &receiver_id);
    void set_from_level(unsigned long level);
    void set_message_id(std::string message_id);
    void set_type(int type);
    void set_node_order(int order);
    void set_data(const std::string &data);
};

// Define hash for message key
// namespace std {
//     template <>
//     struct hash<message_key_t> {
//         size_t operator()(const message_key_t& msg) const {
//             // Compute individual hash values for two data members and combine them using XOR and bit shifting
//             return std::hash<std::string>{}(msg.first) ^ (std::hash<std::string>{}(msg.second) << 1) ;
//         }
//     };

//     // template <>
//     // struct hash<Message> {
//     //     size_t operator()(const Message& msg) const {
//     //         // Compute individual hash values for two data members and combine them using XOR and bit shifting
//     //         return std::hash<std::string>{}(msg.get_sender_id()) ^ (std::hash<std::string>{}(msg.get_message_id()) << 1) ;
//     //     }
//     // };
// }

class MessageTable {
private:
    std::unordered_map<message_key_t, Message> table;

    // std::unordered_map<message_key_t, std::size_t> sender_id_counter;

    // std::size_t receive_counter;

    // std::size_t send_counter;


public:
    
    bool exist(const message_key_t& msg_key) const;

    void insert_received(const Message& msg);
    void insert_sent(const Message& msg);

    std::string to_csv_string() const;
};

#endif
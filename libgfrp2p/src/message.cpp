#include "message.h"

unsigned long get_milliseconds_since_epoch() {
    return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
}

// Message member 
// const char* Message::csv_header = ;
// Constructors
Message::Message() {}

Message::Message(std::string messageID, std::string sender_id, std::string receiver_id):
    message_id(messageID),
    sender_id(sender_id),
    receiver_id(receiver_id) {
        this->node_order = -1;
        this->type = 0;
        this->from_level = 0;
        this->io_type = 0;
        this->io_timestamp = 0;
}

Message::Message(std::string messageID, int type, unsigned long from_level, std::string sender_id, std::string receiver_id):
	message_id(messageID),
	type(type),
	from_level(from_level),
	sender_id(sender_id),
	receiver_id(receiver_id) {
		this->node_order = -1;
        this->io_type = 0;
        this->io_timestamp = 0;
	}
    
Message::Message(unsigned short io_type, std::string messageID, int type, unsigned long from_level, std::string sender_id, std::string receiver_id):
	io_type(io_type),
    message_id(messageID),
	type(type),
	from_level(from_level),
	sender_id(sender_id),
	receiver_id(receiver_id) {
		this->node_order = -1;
        this->io_timestamp = 0;
	}

message_key_t Message::get_key() const { return this->io_timestamp; }

std::string Message::to_csv_string() const { 
    return std::to_string(io_timestamp) + ',' +
        std::to_string(io_type) + ',' +
        sender_id + ','+ 
        message_id + ',' + 
        receiver_id + ',' +
        std::to_string(type) + ',' + 
        std::to_string(from_level) + ',' +         
        std::to_string(node_order);        
}

// Getters and setters
std::string Message::get_sender_id() const { return this->sender_id; }

std::string Message::get_receiver_id() const { return this->receiver_id; }

unsigned long Message::get_from_level() const { return this->from_level; }

std::string Message::get_message_id() const { return this->message_id; }

int Message::get_node_order() const { return this->node_order; }

int Message::get_type() const { return this->type; }

// std::string Message::get_data() const { return this->data; }

void Message::set_sender_id(const std::string &sender_id) { this->sender_id = sender_id; }

void Message::set_receiver_id(const std::string &receiver_id) { this->receiver_id = receiver_id; }

void Message::set_from_level(unsigned long level) { this->from_level = level; }

void Message::set_message_id(std::string msgID) { this->message_id = msgID; }

void Message::set_node_order(int order) { this->node_order = order; }

void Message::set_type(int type) { this->type = type; }

/* MessageTable member functions */
bool MessageTable::exist(const message_key_t& msg_key) const {
    return this->table.find(msg_key) != this->table.end();
}

bool MessageTable::existID(const std::string &msgID) const {
    for (auto msg : this->table) {
        if (msg.second.get_message_id() == msgID) {
            return true;
        }
    }

    return false;
}

// insert records for receiving messages
Message MessageTable::insert_received(const Message& msg) {
    Message to_insert = msg;
    to_insert.io_timestamp = get_milliseconds_since_epoch();
    to_insert.io_type = Message::IO_TYPE_RECEIVED;

    this->table[msg.get_key()] = to_insert;

    return to_insert;
}

// insert records for sending messages
Message MessageTable::insert_sent(const Message& msg) {
    Message to_insert = msg;
    to_insert.io_timestamp = get_milliseconds_since_epoch();
    to_insert.io_type = Message::IO_TYPE_SENT;

    this->table[msg.get_key()] = to_insert;

    return to_insert;
}

std::string MessageTable::to_csv_string() const {
    std::string result = Message::csv_header;

    for(const auto& kv: this->table) {
        result += (kv.second.to_csv_string() + '\n');
    }

    return result;
}

// void Message::set_data(const std::string &data) { this->data = data; }
#include "message.h"

// Message member functions

std::string Message::get_sender_id() const { return this->sender_id; }

std::string Message::get_receiver_id() const { return this->receiver_id; }

unsigned long Message::get_from_level() const { return this->from_level; }

std::string Message::get_message_id() const { return this->message_id; }

int Message::get_node_order() const { return this->node_order; }

int Message::get_type() const { return this->type; }

std::string Message::get_data() const { return this->data; }

void Message::set_sender_id(const std::string &sender_id) { this->sender_id = sender_id; }

void Message::set_receiver_id(const std::string &receiver_id) { this->receiver_id = receiver_id; }

void Message::set_from_level(unsigned long level) { this->from_level = level; }

void Message::set_message_id(std::string msgID) { this->message_id = msgID; }

void Message::set_node_order(int order) { this->node_order = order; }

void Message::set_type(int type) { this->type = type; }

void Message::set_data(const std::string &data) { this->data = data; }
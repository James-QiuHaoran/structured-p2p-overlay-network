#include "peer_manager.h"

// constructor
PeerManager::PeerManager() {

}

Message::Message() {

}

PeerError::PeerError() {

}


// getters
std::string Message::get_sender() const {
	return this->sender;
}

std::string Message::get_receiver() const {
	return this->receiver;
}

std::string Message::get_messageID() const {
	return this->messageID;
}

std::string Message::get_messageHash() const {
	return this->messageHash;
}

std::string Message::get_type() const {
	return this->type;
}

std::string PeerError::get_errorType() const {
	return this->errorType;
}

std::string PeerError::get_errorMessage() const {
	return this->errorMessage;
}

// setters
void Message::set_sender(string sender) {
	this->sender = sender;
}

void Message::set_receiver(string receiver) {
	this->receiver = receiver;
}

void Message::set_messageID(string messageId) {
	this->messageId = messageId;
}

void Message::set_messageHash(string messageHash) {
	this->messageHash = messageHash;
}

void Message::set_type(string type) {
	this->type = type;
}

void PeerError::set_errorType(string type) {
	this->errorType = type;
}

void PeerError::set_errorMessage(string message) {
	this->errorMessage = message;
}
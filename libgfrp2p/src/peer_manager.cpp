#include "peer_manager.h"

// constructor
PeerManager::PeerManager() {

}

// getters
std::string PeerManager::get_sender() const {
	return this->sender;
}

std::string PeerManager::get_receiver() const {
	return this->receiver;
}

std::string PeerManager::get_messageID() const {
	return this->messageID;
}

std::string PeerManager::get_messageHash() const {
	return this->messageHash;
}

std::string PeerManager::get_type() const {
	return this->type;
}

// setters
void PeerManagerset_sender(string sender) {
	this->sender = sender;
}

void PeerManagerset_receiver(string receiver) {
	this->receiver = receiver;
}

void PeerManagerset_messageID(string messageId) {
	this->messageId = messageId;
}

void PeerManagerset_messageHash(string messageHash) {
	this->messageHash = messageHash;
}

void PeerManagerset_type(string type) {
	this->type = type;
}
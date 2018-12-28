#ifndef EVAL_SERVER_H
#define EVAL_SERVER_H

#include <sqlite3.h>
#include "transport.h"
#include "bootstrap_message.pb.h"

class EvalServer: public Receiver, public std::enable_shared_from_this<EvalServer> {
private:
	sqlite3 *db;
	int num_node;    

public:
	void start(unsigned short port);
	virtual void receive(const std::string& ip, unsigned short port, const std::string& data) override;
	static int printNode(void *NotUsed, int argc, char **argv, char **azColName);
	void init(const std::string& ip, unsigned short port, const bootstrap_message::BootstrapMessage nodeinfo);
	static int sendConfig(void *data, int argc, char **argv, char **azColName);
	void config();

};

#endif

#include <cstdio>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include "websocketdef.h"
#include "task.h"
#include "files.h"

void receiveWSMessage(WSserver* server, websocketpp::connection_hdl hdl, WSserver::message_ptr msg) {
	if (msg->get_payload() == "finish") {
		server->stop();
		return;
	}
	RunTask(server, hdl, msg);
}

void startAsioServer() {
	WSserver server;
	server.init_asio();
	server.set_message_handler(websocketpp::lib::bind(&receiveWSMessage, &server, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
	server.listen(9002);
	server.start_accept();

	std::vector<std::thread> threads;
	auto ioservice = std::bind(&WSserver::run, &server);
	for (int i = 0; i < 4; ++i) {
		threads.push_back(std::thread(ioservice));
	}

	for (auto it = threads.begin(); it < threads.end(); ++it) {
		it->join(); // Wait for stuff to be done
	}
}

int main(int argc, char** argv) {
	startAsioServer();
	return 0;
}
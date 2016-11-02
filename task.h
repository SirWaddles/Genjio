#pragma once

#include <string>
#include "websocketdef.h"

class Task {
public:
	template<typename T>
	static void RunTask(WSserver* server, websocketpp::connection_hdl hdl, std::string message) {
		T task(server, hdl, message);
		task.Run();
	}

	Task(WSserver* server, websocketpp::connection_hdl hdl, std::string message);

	virtual void Run();
protected:
	WSserver* mServer;
	websocketpp::connection_hdl mHdl;
	std::string contents;
};

void RunTask(WSserver* server, websocketpp::connection_hdl hdl, WSserver::message_ptr msg);
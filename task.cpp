#include <functional>
#include <cstdint>
#include "task.h"
#include "files.h"
#include "videocheck.h"

Task::Task(WSserver* server, websocketpp::connection_hdl hdl, std::string message) : mServer(server), mHdl(hdl), contents(message) {

}

void Task::Run() {

}

void RunTask(WSserver* server, websocketpp::connection_hdl hdl, WSserver::message_ptr msg) {
	int messageID = *reinterpret_cast<const uint32_t*>(msg->get_payload().c_str());
	switch (messageID) {
	case 1:
		server->get_io_service().post(std::bind(&Task::RunTask<FileAssemblerTask>, server, hdl, msg->get_payload()));
	}
}
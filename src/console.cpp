/*
 * console.cpp
 *
 *  Created on: Aug 12, 2017
 *      Author: nullifiedcat
 */

#include <iostream>
#include <functional>
#include <unordered_map>
#include <string>
#include <vector>
#include <ctime>

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#include "ipcb.hpp"
#include "cathookipc.hpp"

#include "json.hpp"

using json = nlohmann::json;
using peer_t = cat_ipc::Peer<server_data_s, user_data_s>;

std::unordered_map<std::string, std::function<json(const json&)>> commands {};
peer_t* peer { nullptr };

bool has_key(const json& o, const std::string& key) {
	return o.find(key) != o.end();
}

void ReplaceString(std::string& input, const std::string& what, const std::string& with_what) {
	size_t index;
	index = input.find(what);
	while (index != std::string::npos) {
		input.replace(index, what.size(), with_what);
		index = input.find(what, index + with_what.size());
	}
}

json query_peer(unsigned id) {
	if (not peer or not peer->connected) {
		throw std::runtime_error("not connected to ipc server");
	}

	if (id >= cat_ipc::max_peers) {
		throw std::out_of_range("peer out of range");
	}

	json result {};

	const auto& sdata = peer->memory->peer_data[id];
	const auto& udata = peer->memory->peer_user_data[id];

	if (peer->IsPeerDead(id)) {
		result["dead"] = true;
		return result;
	}

	result["pid"] = sdata.pid;
	result["starttime"] = sdata.starttime;

	result["class"] = udata.clazz;
	result["connected"] = udata.connected;
	result["friendid"] = udata.friendid;
	result["good"] = udata.good;
	result["health"] = udata.health;
	result["health_max"] = udata.health_max;
	result["heartbeat"] = udata.heartbeat;
	result["life_state"] = udata.life_state;
	result["name"] = std::string(udata.name);
	result["score"] = udata.score;
	result["server"] = udata.server;
	result["team"] = udata.team;
	result["total_score"] = udata.total_score;

	return result;
}

namespace cmd {

json exec(const json& args) {
	if (not peer or not peer->connected) {
		throw std::runtime_error("not connected to ipc server");
	}
	if (not has_key(args, "target")) {
		throw std::runtime_error("undefined pid");
	}
	if (not has_key(args, "cmd")) {
		throw std::runtime_error("undefined command");
	}
	unsigned uid = args["target"].get<unsigned>();
	if (uid < 0 || uid >= cat_ipc::max_peers) {
		throw std::out_of_range("peer out of range");
	}
	if (peer->IsPeerDead(uid)) {
		throw std::runtime_error("peer is not connected");
	}
	std::string cmd = args["cmd"];
	ReplaceString(cmd, " && ", " ; ");
	if (cmd.length() >= 63) {
		peer->SendMessage(0, (1 << uid), ipc_commands::execute_client_cmd_long, cmd.c_str(), cmd.length() + 1);
	} else {
		peer->SendMessage(cmd.c_str(), (1 << uid), ipc_commands::execute_client_cmd, 0, 0);
	}
	return json {};
}

json exec_all(const json& args) {
	if (not peer or not peer->connected) {
		throw std::runtime_error("not connected to ipc server");
	}
	if (not has_key(args, "cmd")) {
		throw std::runtime_error("undefined command");
	}
	std::string cmd = args["cmd"];
	ReplaceString(cmd, " && ", " ; ");
	if (cmd.length() >= 63) {
		peer->SendMessage(0, 0, ipc_commands::execute_client_cmd_long, cmd.c_str(), cmd.length() + 1);
	} else {
		peer->SendMessage(cmd.c_str(), 0, ipc_commands::execute_client_cmd, 0, 0);
	}
	return json {};
}

json query(const json& args) {
	if (not peer or not peer->connected) {
		throw std::runtime_error("not connected to ipc server");
	}
	json result {};

	bool skipEmpty = (has_key(args, "skipEmpty") and args["skipEmpty"].get<bool>());

	if (has_key(args, "ids")) {
		for (const auto& i : args["ids"]) {
			unsigned uid = i;
			if (skipEmpty and peer->IsPeerDead(uid)) {
				continue;
			}
			result[i.get<std::string>()] = query_peer(uid);
		}
	} else {
		for (unsigned i = 1; i < cat_ipc::max_peers; i++) {
			if (skipEmpty and peer->IsPeerDead(i)) {
				continue;
			}
			result[std::to_string(i)] = query_peer(i);
		}
	}
	return result;
}

// Server Query
json squery(const json& args) {
	if (not peer or not peer->connected) {
		throw std::runtime_error("not connected to ipc server");
	}
	json result {};
	const auto& mem = peer->memory;
	result["count"] = mem->peer_count;
	result["command_count"] = mem->command_count;
	return result;
}

json kill(const json& args) {
	if (getuid() != 0) {
		throw std::runtime_error("kill can only be used as root");
	}
	if (not peer or not peer->connected) {
		throw std::runtime_error("not connected to ipc server");
	}
	if (not has_key(args, "pid")) {
		throw std::runtime_error("undefined pid");
	}
	pid_t uid = args["pid"].get<pid_t>();
	if (uid < 0 || uid >= cat_ipc::max_peers) {
		throw std::out_of_range("peer out of range");
	}
	if (peer->IsPeerDead(uid)) {
		throw std::runtime_error("already dead");
	}
	::kill(pid_t(peer->memory->peer_data[uid].pid), SIGKILL);
	return json {};
}

json echo(const json& args) {
	json result {};
	result["args"] = args;
	return result;
}

json connect(const json& args) {
	if (peer) {
		throw std::runtime_error("already connected");
	}
	try {
		peer = new peer_t(has_key(args, "server") ? args["server"].get<std::string>() : "cathook_followbot_server", false, false, true);
		peer->Connect();
	} catch (std::exception& ex) {
		peer = nullptr;
		throw ex;
	}
	return json {};
}

json disconnect(const json& args) {
	if (not peer or not peer->connected) {
		throw std::runtime_error("not connected to ipc server");
	}
	delete peer;
	peer = nullptr;
	return json {};
}

}

std::vector<std::string> split(const std::string& input) {
	std::vector<std::string> result {};
	std::string current;
	bool quote = false,
		 escape = false;

	for (const auto& c : input) {
		if (escape) {
			current.push_back(c);
			escape = false;
		} else {
			if (c == '\\') {
				escape = true;
			} else if (c == '"') {
				quote = !quote;
			} else if (not quote and (c == ' ' || c == '\n' || c == '\t')) {
				if (not current.empty()) {
					result.push_back(current);
					current.clear();
				}
			} else {
				current.push_back(c);
			}
		}
	}

	if (not current.empty()) {
		result.push_back(current);
	}

	return result;
}


int main(int argc, const char** argv) {
	commands["exec"] = &cmd::exec;
	commands["exec_all"] = &cmd::exec_all;
	commands["query"] = &cmd::query;
	commands["kill"] = &cmd::kill;
	commands["echo"] = &cmd::echo;
	commands["connect"] = &cmd::connect;
	commands["disconnect"] = &cmd::disconnect;
	commands["squery"] = &cmd::squery;

	std::cout << json { { "init", time(nullptr) } } << "\n";

	while (true) {
		std::string input { "" };
		std::string cmdid { "undefined" };
		std::getline(std::cin, input);
		try {
			auto args = json::parse(input);
			if (not has_key(args, "command")) {
				throw std::runtime_error("empty command");
			}
			if (has_key(args, "cmdid")) {
				cmdid = args["cmdid"];
			}
			if (args["command"] == "exit" or args["command"] == "quit") {
				std::cout << json { { "exit", time(nullptr) } } << "\n";
				break;
			}
			if (commands.find(args["command"]) == commands.end()) {
				throw std::runtime_error("command not found");
			} else {
				std::cout << json { { "status", "success" }, { "cmdid", cmdid }, { "result", commands[args["command"]](args) } } << "\n";
			}
		} catch (std::exception& ex) {
			std::cout << json { { "status", "error" }, { "cmdid", cmdid }, { "error", std::string(ex.what()) } } << "\n";
		}
	}
}

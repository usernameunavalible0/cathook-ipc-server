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


std::unordered_map<std::string, std::function<json(const std::vector<std::string>&)>> commands {};
peer_t* peer { nullptr };

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

json exec(const std::vector<std::string>& args) {
	if (not peer or not peer->connected) {
		throw std::runtime_error("not connected to ipc server");
	}
	json result {};
	result["success"] = true;
	return result;
}

json exec_all(const std::vector<std::string>& args) {
	if (not peer or not peer->connected) {
		throw std::runtime_error("not connected to ipc server");
	}
	json result {};
	result["success"] = true;
	return result;
}

json query(const std::vector<std::string>& args) {
	if (not peer or not peer->connected) {
		throw std::runtime_error("not connected to ipc server");
	}
	json result { { "data", json {} } };
	if (args.size() > 1) {
		for (int i = 1; i < args.size(); i++) {
			unsigned uid = std::stoul(args[1]);
			result[args[1]] = query_peer(uid);
		}
	} else {
		for (unsigned i = 0; i < cat_ipc::max_peers; i++) {
			result[std::to_string(i)] = query_peer(i);
		}
	}
	result["success"] = true;
	return result;
}

// Server Query
json squery(const std::vector<std::string>& args) {
	if (not peer or not peer->connected) {
		throw std::runtime_error("not connected to ipc server");
	}
	json result { { "data", json::object() } };
	const auto& mem = peer->memory;
	result["data"]["count"] = mem->peer_count;
	result["data"]["command_count"] = mem->command_count;
	result["success"] = true;
	return result;
}

json kill(const std::vector<std::string>& args) {
	if (getuid() != 0) {
		throw std::runtime_error("kill can only be used as root");
	}
	if (not peer or not peer->connected) {
		throw std::runtime_error("not connected to ipc server");
	}
	int uid = std::stoi(args[1]);
	if (uid < 0 || uid >= cat_ipc::max_peers) {
		throw std::out_of_range("peer out of range");
	}
	if (peer->IsPeerDead(uid)) {
		throw std::runtime_error("already dead");
	}
	::kill(pid_t(peer->memory->peer_data[uid].pid), SIGKILL);
	json result {};
	result["success"] = true;
	return result;
}

json echo(const std::vector<std::string>& args) {
	json result {};
	result["success"] = true;
	result["args"] = json::array();
	for (const auto& a : args) {
		result["args"].push_back(a);
	}
	return result;
}

json connect(const std::vector<std::string>& args) {
	if (peer) {
		throw std::runtime_error("already connected");
	}
	peer = new peer_t(args.size() > 1 ? args[1] : "cathook_followbot_server", false, false, true);
	peer->Connect();
	return json {
		{ "success", true }
	};
}

json disconnect(const std::vector<std::string>& args) {
	if (not peer or not peer->connected) {
		throw std::runtime_error("not connected to ipc server");
	}
	delete peer;
	peer = nullptr;
	return json {
		{ "success", true }
	};
}

// Does literally nothing but emit success
json test(const std::vector<std::string>& args) {
	return json {
		{ "success", true }
	};
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
	commands["test"] = &cmd::test;
	commands["echo"] = &cmd::echo;
	commands["connect"] = &cmd::connect;
	commands["disconnect"] = &cmd::disconnect;
	commands["squery"] = &cmd::squery;

	std::cout << json { { "init", time(nullptr) } } << std::endl;

	while (true) {
		std::string input;
		std::getline(std::cin, input);
		try {
			auto args = split(input);
			if (args.empty()) {
				throw std::runtime_error("empty command");
			}
			if (args[0] == "exit" or args[0] == "quit") {
				std::cout << json { { "exit", time(nullptr) } } << std::endl;
				break;
			}
			if (commands.find(args[0]) == commands.end()) {
				throw std::runtime_error("command not found");
			} else {
				std::cout << commands[args[0]](args) << std::endl;
			}
		} catch (std::exception& ex) {
			std::cout << json { { "error", std::string(ex.what()) } } << std::endl;
		}
	}
}

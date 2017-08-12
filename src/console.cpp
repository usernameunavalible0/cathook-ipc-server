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

#include "json.hpp"

using json = nlohmann::json;

std::unordered_map<std::string, std::function<json(const std::vector<std::string>&)>> commands {};

namespace cmd {

json exec(const std::vector<std::string>& args) {
	return json {
		{ "success", true }
	};
}

json exec_all(const std::vector<std::string>& args) {
	return json {
		{ "success", true }
	};
}

json query(const std::vector<std::string>& args) {
	return json {
		{ "success", true }
	};
}

json kill(const std::vector<std::string>& args) {
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

json echo(const std::vector<std::string>& args) {
	json result {};
	result["args"] = json::array();
	for (const auto& a : args) {
		result["args"].push_back(a);
	}
	return result;
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
			std::cout << json { "error", std::string(ex.what()) } << std::endl;
		}
	}
}

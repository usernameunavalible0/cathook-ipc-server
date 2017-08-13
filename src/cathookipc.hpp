/*
 * cathookipc.hpp
 *
 *  Created on: Mar 19, 2017
 *      Author: nullifiedcat
 */

#ifndef CATHOOKIPC_HPP_
#define CATHOOKIPC_HPP_

#include <time.h>

struct server_data_s {
	unsigned magic_number;
};

struct user_data_s {
	char name[32];
	char server[22];
	unsigned friendid;
	bool connected;
	bool good;
	int health;
	int health_max;
	char life_state;
	int team;
	int clazz;
	int score;
	int last_score;
	int total_score;
	time_t heartbeat;
};

namespace ipc_commands {

constexpr unsigned execute_client_cmd = 1;
constexpr unsigned set_follow_steamid = 2;
constexpr unsigned execute_client_cmd_long = 3;
constexpr unsigned move_to_vector = 4;
constexpr unsigned stop_moving = 5;
constexpr unsigned start_moving = 6;

}

#endif /* CATHOOKIPC_HPP_ */

/*
 * cathookipc.hpp
 *
 *  Created on: Mar 19, 2017
 *      Author: nullifiedcat
 */

#ifndef CATHOOKIPC_HPP_
#define CATHOOKIPC_HPP_

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
};

#endif /* CATHOOKIPC_HPP_ */

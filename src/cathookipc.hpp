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
	unsigned friendid;
};

#endif /* CATHOOKIPC_HPP_ */

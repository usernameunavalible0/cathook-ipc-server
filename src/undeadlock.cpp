/*
 * undeadlock.cpp
 *
 *  Created on: Jul 29, 2017
 *      Author: nullifiedcat
 */

#include "ipcb.hpp"
#include "cathookipc.hpp"

int main() {
	cat_ipc::Peer<server_data_s, user_data_s> object("cathook_followbot_server", false, false, true);
	object.Connect();
	object.memory->mutex.unlock();
}

/*
 * undeadlock.cpp
 *
 *  Created on: Jul 29, 2017
 *      Author: nullifiedcat
 */

#include "ipcb.hpp"
#include "cathookipc.hpp"

int main() {
	cat_ipc::Peer<server_data_s, user_data_s> object("cathook_followbot_server", false, false);

	int old_mask = umask(0);
	int flags = O_RDWR;
	int fd = shm_open("cathook_followbot_server", flags, S_IRWXU | S_IRWXG | S_IRWXO);
	if (fd == -1) {
		throw std::runtime_error("server isn't running");
	}
	ftruncate(fd, sizeof(cat_ipc::Peer<server_data_s, user_data_s>::memory_t));
	umask(old_mask);
	cat_ipc::Peer<server_data_s, user_data_s>::memory_t* memory = (cat_ipc::Peer<server_data_s, user_data_s>::memory_t*)mmap(0, sizeof(cat_ipc::Peer<server_data_s, user_data_s>::memory_t), PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED, fd, 0);
	close(fd);
	pthread_mutex_unlock(&object.memory->mutex);
}

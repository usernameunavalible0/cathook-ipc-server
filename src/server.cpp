/*
 * main.cpp
 *
 *  Created on: Mar 19, 2017
 *      Author: nullifiedcat
 */

#include "ipcb.hpp"
#include "cathookipc.hpp"

cat_ipc::Peer<server_data_s, user_data_s>& peer() {
	static cat_ipc::Peer<server_data_s, user_data_s> object("cathook_followbot_server", false, true);
	return object;
}

#define ESC_PRINTF(s, ...) printf("\e[" s, __VA_ARGS__)
#define ESC_CUP(x, y) ESC_PRINTF("%d;%dH", (y), (x))
#define ESC_EL(n) ESC_PRINTF("%dK", (n))
#define ESC_SGR(n) ESC_PRINTF("%dm", (n))
#define ESC_ED(n) ESC_PRINTF("%dJ", (n));

#define TEXT_BOLD ESC_SGR(1)
#define TEXT_NORMAL ESC_SGR(0)

void print_status() {
	ESC_CUP(1, 1);
	ESC_ED(2);
	fflush(stdout);
	ESC_CUP(2, 2);
	TEXT_BOLD;
	printf("cathook followbot server");
	ESC_CUP(3, 4);
	printf("connected: ");
	TEXT_NORMAL; printf("%d", peer().memory->peer_count); TEXT_BOLD;
	ESC_CUP(3, 5);
	printf("command count: ");
	TEXT_NORMAL; printf("%lu", peer().memory->command_count); TEXT_BOLD;
	ESC_CUP(3, 6);
	printf("command memory pool stats: ");
	CatMemoryPool::pool_info_s info;
	peer().pool->statistics(info);
	ESC_CUP(4, 8); ESC_EL(2);  printf("total:     ");
	ESC_CUP(4, 9); ESC_EL(2);  printf("free:      ");
	ESC_CUP(4, 10); ESC_EL(2); printf("allocated: ");
	TEXT_NORMAL;
	ESC_CUP(16, 8);  printf("%lu (%u blocks)", info.free + info.alloc, info.blkcnt);
	ESC_CUP(16, 9); printf("%lu (%u blocks)", info.free, info.freeblk);
	ESC_CUP(16, 10); printf("%lu (%u blocks)", info.alloc, info.allocblk);
	ESC_CUP(3, 12);
	TEXT_BOLD;
	printf("peer list: ");
	int ypos = 13;
	ESC_CUP(2, ypos);
	printf("ID PID   STEAMID   SERVER                NAME");
	ypos++;
	TEXT_NORMAL;
	// Zeroth peer is the server.
	for (unsigned i = 1; i < cat_ipc::max_peers; i++) {
		if (!peer().memory->peer_data[i].free) {
			ESC_CUP(2, ypos);
			printf("%-2u %-5d %-9ld %-22s %s", i, peer().memory->peer_data[i].pid, peer().memory->peer_user_data[i].friendid, peer().memory->peer_user_data[i].server, peer().memory->peer_user_data[i].name);
			ypos++;
		}
	}
	ESC_CUP(1, ypos + 1);
	fflush(stdout);
}

int main(int argc, char** argv) {
	unsigned long tick = 0;
	peer().Connect();
	peer().memory->global_data.magic_number = 0x0DEADCA7;
	//printf("magic number offset: 0x%08x\n", (uintptr_t)&peer().memory->global_data.magic_number - (uintptr_t)peer().memory);
	while (true) {
		tick++;
		if (!(tick % 10)) { // Sweep/Process once every 10 seconds
			peer().SweepDead();
			peer().ProcessCommands();
		}
		print_status();
		sleep(2);
	}
	return 0;
}

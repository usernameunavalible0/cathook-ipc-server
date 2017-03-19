/*
 * main.cpp
 *
 *  Created on: Mar 19, 2017
 *      Author: nullifiedcat
 */

#include "ipcb.hpp"
#include "cathookipc.hpp"

static bool is_server = false;

cat_ipc::Peer<global_data_s, peer_data_s>& peer() {
	static cat_ipc::Peer<global_data_s, peer_data_s> object("cathook_followbot_server", false, is_server);
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
	TEXT_NORMAL;
	for (unsigned i = 0; i < peer().memory->peer_count; i++) {
		if (!peer().memory->peer_data[i].free) {
			printf("%u (%d) ", i, peer().memory->peer_data[i].pid);
		}
	}
	ESC_CUP(1, 14);
	fflush(stdout);
}

int main(int argc, char** argv) {
	is_server = true;
	unsigned long tick = 0;
	while (true) {
		tick++;
		if (!(tick % 100)) { // Sweep/Process once every 10 seconds
			peer().SweepDead();
			peer().ProcessCommands();
		}
		print_status();
		usleep(100000);
	}
	return 0;
}

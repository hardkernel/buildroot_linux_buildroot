/*
 * Copyright (C) Amlogic
 *
 * auther: peipeng.zhao@amlogic.com
 */

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/klog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "events_process.h"
static const char *STATE_FILE = "/sys/power/state";
EventsProcess* ep = new EventsProcess();

int main(int argc, char **argv) {
	int key = -1;
	int mem_wrote_flag = 0;

	int state_fd = open(STATE_FILE,O_RDWR);

	if (state_fd == -1) {
		printf("open sys/power/state failed (%s)\n",
				strerror(errno));
	}

	ep->Init();

	while (1) {
		key = ep->WaitKey();
		if (key == KEY_POWER) {
			lseek(state_fd, 0, SEEK_SET);
			if (mem_wrote_flag == 0) {
				write(state_fd,"mem", 3);
				mem_wrote_flag = 1;
			} else if (mem_wrote_flag == 1) {
				write(state_fd,"on", 2);
				mem_wrote_flag = 0;
			}

		} else if (key == KEY_VOLUMEUP) {
		} else if (key == KEY_VOLUMEDOWN) {
		}
	}

	close(state_fd);
}


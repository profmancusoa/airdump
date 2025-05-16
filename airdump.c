/*
 * airdump - a lightweight yet powerful tool for monitoring WiFi networks
 *
 * Copyright (c) 2025 Antonio Mancuso <profmancusoa@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *
 * This tools is eavily inspired by wavemon (https://github.com/uoaerg/wavemon) and reuse part of its code
 *
 */

// #include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>
#include <err.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "iw_if.h"
#include "error.h"

extern void collect_dump_init();
extern void collect_dump_loop();


// GLOBALS CONFIG
char *wireless_iface = NULL; 
const char *dump_file = NULL;
uint32_t period = 100;
uint32_t count = UINT32_MAX;

void usage() {
	printf("\nUsage: %s [-hv] [-d ms] [-c count] <-i ifname> <-o dumpfile>\n", TOOL_NAME);
	printf("  -h            This help screen\n");
	printf("  -v            Print version details\n");
	printf("  -d [ms]       Scan period in milliseconds - default 100\n");
	printf("  -c [count]    Exact number of measure to dump - default 2^32\n");
	printf("  -i <ifname>   Use specified network interface\n");
	printf("  -o <dumpfile> File to dump air information\n\n");
	exit(EXIT_SUCCESS);
}

void get_cli_config(int argc, char *argv[]) {
	int arg;
	bool help = false, version = false, iface = false, out = false;
	
	while ((arg = getopt(argc, argv, "hvd:c:i:o:")) >= 0) {
		switch (arg) {
		case 'h':
			help = true;;
			break;
		case 'v':
			version = true;
			break;	
		case 'd':
			period = atoi(optarg);
			break;
		case 'c':
			count = atol(optarg);
			break;
		case 'i':
			wireless_iface = optarg;
			iface = true;
			break;
		case'o':
			dump_file = optarg;
			out = true;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	}

	if(version) {
		printf("%s v%s\n", TOOL_NAME, TOOL_VERSION);
		exit(EXIT_SUCCESS);
	}
	if(help) usage();
	if(!iface || !out) usage();
}

const char *conf_ifname(void) {
	return wireless_iface;
}

int main(int argc, char *argv[]) {
	get_cli_config(argc, argv);
	iface_exists_up(wireless_iface);
	collect_dump_init();
	collect_dump_loop(dump_file, period, count);
	
	return EXIT_SUCCESS;
}
/* Copyright (c) 2017 Kewin Rausch
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Empower Agent simulator main application.
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "emsim.h"

#define LOG_MAIN(x, ...)	LOG_TRACE(x, ##__VA_ARGS__)

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

/* Id of the agent associated with the simulator. */
u32 sim_ID = 1;
/* 1 second of default interval time */
u32 sim_loop_int = 1000;
/* Headless start? */
u32 sim_hl = 0;

/* Address of the controller */
char sim_ctrl_addr[64] = "127.0.0.1";
/* Port used to connect to the controller */
u16    sim_ctrl_port = 2210;

void util_mask_all_signals()
{
	sigset_t set;

	/* Don't let us get disturbed by any signal */
	sigfillset(&set);
	pthread_sigmask(SIG_SETMASK, &set, NULL);
}

/******************************************************************************
 * Arguments handling:                                                        *
 ******************************************************************************/

void help(void)
{
	printf(
"EmPower Base Station simulator.\n"
"Simulates a LTE eNB which connects to an Empower-protocols compatible "
"controller.\n"
"\n"
"Supported arguments:\n"
"\n"
"--id <num>\n"
"    Agent id for this instance.\n"
"--cell <pci:DL_earfcn:UL_earfcn:DL_prbs:UL_prbs>\n"
"    Define a new cell in the simulator.\n"
"--ctrl_addr <IP addr>\n"
"    Connect with EmPOWER controller on this address.\n"
"--ctrl_port <num>\n"
"    Connect with EmPOWER controller using custom port.\n"
"--x2p <num>\n"
"    Use the specified port for X2 interface connection\n"
"--scenario <path>\n"
"    Load a scenario (known UE and neighbors) at startup\n"
"--hl\n"
"    Headless, run without UI\n");
}

void parse_cell(char * args)
{
	char * pci;
	char * dl_earfcn;
	char * ul_earfcn;
	char * dl_prb;
	char * ul_prb;

	if(sim_phy.nof_cells == PHY_CELL_MAX) {
		LOG_MAIN("No slots left for additional cells!\n");
		return;
	}

	pci       = strtok(args, ":");
	dl_earfcn = strtok(0, ":");
	ul_earfcn = strtok(0, ":");
	dl_prb    = strtok(0, ":");
	ul_prb    = strtok(0, ":");

	if(stack_add_cell(
		(unsigned short)atoi(pci),
		atoi(dl_earfcn),
		atoi(ul_earfcn),
		(unsigned char)atoi(dl_prb),
		(unsigned char)atoi(ul_prb)))
	{
		LOG_MAIN("Cell configuration failed!\n");
		exit(0);
	}
}

void parse_args(int argc, char ** argv)
{
	int i;

	for(i = 1; i < argc; i++) {
		if(strcmp(argv[i], "--id") == 0) {
			sim_ID = atoi(argv[i + 1]);
			i = i + 1;

			LOG_MAIN("Agent ID set to %d.\n", sim_ID);

			continue;
		}

		if(strcmp(argv[i], "--cell") == 0) {
			parse_cell(argv[i + 1]);
			i = i + 1;

			continue;
		}

		if(strcmp(argv[i], "--ctrl_addr") == 0) {
			if(i + 1 >= argc) {
				LOG_MAIN("--ctrl_addr miss a value\n");
				continue;
			}

			strncpy(sim_ctrl_addr, argv[i + 1], 64);
			i++;

			LOG_MAIN("Will connect to controller %s\n",
				sim_ctrl_addr);

			continue;
		}

		if(strcmp(argv[i], "--x2p") == 0) {
			if(i + 1 >= argc) {
				LOG_MAIN("--x2p miss a value\n");
				continue;
			}

			sim_x2_port = (u16)atoi(argv[i + 1]);
			i++;

			LOG_MAIN("Will start X2 interface on port %d\n",
				sim_x2_port);

			continue;
		}

		if(strcmp(argv[i], "--hl") == 0) {
			sim_hl = 1;

			LOG_MAIN("Will not start the UI\n");

			continue;
		}

		if(strcmp(argv[i], "--scenario") == 0) {
			if(i + 1 >= argc) {
				LOG_MAIN("Scenario is missing a path\n");
				continue;
			}

			LOG_MAIN("Loading scenario %s\n", argv[i + 1]);

			sce_load(argv[i + 1]);
			i++;

			continue;
		}
	}
}

/******************************************************************************
 * Entry point:                                                               *
 ******************************************************************************/

static volatile int ctrl_c = 0;

/* Simple signal handler */
void signal_handler(int sig)
{
	ctrl_c = 1;
}
#if 1
int sim_switch = 1;
int sim_peak = 5;
#endif
int main(int argc, char ** argv) {
	char logp[256] = {0};
	//util_mask_all_signals();

	/* No arguments means show the help. */
	if(argc <= 1) {
		help();
		return 0;
	}

	snprintf(logp, 256, "embase.%d.log", getpid());

	/* Initialize the logging subsystem. */
	if(log_init(logp) < 0) {
		return 0;
	}

	/* Salt the random mechanism... will be used later. */
	srand((int)time(NULL));

	/* Initialize the LTE stack simulation subsystem. */
	if(stack_init()) {
		return 0;
	}

	/* Examine arguments. */
	parse_args(argc, argv);

	/* Nobody has set up a cell in the simulator yet... */
	if(sim_phy.nof_cells == 0) {
		/* Add the default cell */
		stack_add_cell(1, 1750, 19750, 25, 25);
	}

	/* Start the agent. */
	em_start(sim_ID, &sim_ops, sim_ctrl_addr, sim_ctrl_port);

	/* Start the X2 interface. */
	if(x2_init()) {
		goto out;
	}

	if(!sim_hl) {
		/* Start the UI mechanisms. */
		iface_init();
	} else {
		signal(SIGINT, signal_handler);
		iface_alive = 1;
	}

	/* Wait for the interface to come down... */
	do {
#if 1
		if(sim_ues[0].rnti == RAN_USER_INVALID_ID) {
			sim_peak = 5;
			goto skip;
		}

		/* sim_switch != 0
		 * Carrier quality increses and neighbor cell quality decreses.
		 */
		if(sim_switch) {
			sim_ues[0].meas[0].rs.rsrq = sim_ues[0].meas[0].rs.rsrq - 1.0f;

			if(sim_ues[0].meas[0].rs.rsrq < PHY_RSRQ_LOWER) {
				sim_ues[0].meas[0].rs.rsrq = PHY_RSRQ_LOWER;
				sim_peak--;

				if(sim_peak <= 0) {
					sim_peak   = 5;
					sim_switch = !sim_switch;
				}
			}

			/* Neighbour quality increase */
			sim_neighs[0].rs[0].rsrq = sim_neighs[0].rs[0].rsrq + 1.0f;

			if(sim_neighs[0].rs[0].rsrq > PHY_RSRQ_HIGHER) {
				sim_neighs[0].rs[0].rsrq = PHY_RSRQ_HIGHER;
			}

			sim_ues[0].meas[0].dirty = 1;
		} 
		/* sim_switch == 0
		 * Carrier quality decreses and neighbor cell quality increses.
		 */		
		else {
			/* Carrier quality increase */
			sim_ues[0].meas[0].rs.rsrq = sim_ues[0].meas[0].rs.rsrq + 1.0f;

			if(sim_ues[0].meas[0].rs.rsrq > PHY_RSRQ_HIGHER) {
				sim_ues[0].meas[0].rs.rsrq = PHY_RSRQ_HIGHER;
				sim_peak--;

				if(sim_peak <= 0) {
					sim_peak   = 5;
					sim_switch = !sim_switch;
				}
			}

			/* Neighbour quality increase */
			sim_neighs[0].rs[0].rsrq = sim_neighs[0].rs[0].rsrq - 1.0f;

			if(sim_neighs[0].rs[0].rsrq < PHY_RSRQ_LOWER) {
				sim_neighs[0].rs[0].rsrq = PHY_RSRQ_LOWER;
			}

			sim_ues[0].meas[0].dirty = 1;
		}
skip:
#endif
		/*
		 * Perform UE simulation.
		 * NOTE: this can generate network feedback.
		 */
		ue_compute();

		/*
		 * Perform LTE stack simulation.
		 */
		stack_compute();

		/*
		 * X2 channel for eNB-to-eNB communication.
		 */
		x2_compute();

		/* Sleep for configurable usec */
		usleep(sim_loop_int * 1000);
	} while(iface_alive && !ctrl_c);

out:
	em_terminate_agent(sim_ID);
	log_release();

	printf("Logging session saved in %s\n", logp);

	return 0;
}

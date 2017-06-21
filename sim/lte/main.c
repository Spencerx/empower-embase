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
#include <time.h>
#include <unistd.h>

#include "emsim.h"

#define LOG_MAIN(x, ...)	LOG_TRACE(x, ##__VA_ARGS__)

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

/* Id of the agent associated with the simulator. */
u32 sim_ID = 0;

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
"--cell <num>\n"
"    Physical Cell Id for this instance.\n"
"--freq <earfcn>\n"
"    Frequency used by the cell.\n"
"--dl_prb <num>\n"
"    Number of PRB used in the Downlink.\n"
"--ul_prb <num>\n"
"    Number of PRB used in the Uplink.\n");
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

		if(strcmp(argv[i], "--freq") == 0) {
			sim_phy.earfcn = atoi(argv[i + 1]);
			i = i + 1;

			LOG_MAIN("Cell EARFCN frequency set to %d.\n",
				sim_phy.earfcn);

			continue;
		}

		if(strcmp(argv[i], "--dl_prb") == 0) {
			sim_phy.DL_prb = atoi(argv[i + 1]);
			i = i + 1;

			LOG_MAIN("DL PRB set to %d.\n", sim_phy.DL_prb);

			continue;
		}

		if(strcmp(argv[i], "--ul_prb") == 0) {
			sim_phy.UL_prb = atoi(argv[i + 1]);
			i = i + 1;

			LOG_MAIN("UL PRB set to %d.\n", sim_phy.UL_prb);

			continue;
		}

		if(strcmp(argv[i], "--cell") == 0) {
			sim_phy.pci = atoi(argv[i + 1]);
			i = i + 1;

			LOG_MAIN("Physical cell ID set to %d.\n", sim_phy.pci);

			continue;
		}
	}
}

/******************************************************************************
 * Entry point:                                                               *
 ******************************************************************************/

int main(int argc, char ** argv) {
	/* No arguments means show the help. */
	if(argc <= 1) {
		help();
		return 0;
	}

	/* Initialize the logging subsystem. */
	if(log_init("emlog.log")) {
		return 0;
	}

	/* Salt the random mechanism... will be used later. */
	srand((int)time(NULL));

	/* Initialize the PHY subsystem. */
	if(phy_init()) {
		return 0;
	}

	/* Initialize the RAN subsystem. */
	if(ran_init()) {
		return 0;
	}

	/* Examine arguments. */
	parse_args(argc, argv);

	/* Start the agent. */
	em_start(&sim_ops, sim_ID);

	/* Start the X2 interface. */
	if(x2_init()) {
		goto out;
	}

	/* Start the UI mechanisms. */
	iface_init();

	/* Wait for the interface to come down... */
	do {
		/*
		 * UE related stuff:
		 */

		/*
		 * Perform UE simulation.
		 * NOTE: this can generate network feedback.
		 */
		ue_compute();


		/*
		 * PHY related:
		 */

		/*
		 * Perform PHY layer emulation.
		 * NOTE: this can generate network feedback.
		 */
		phy_compute();

		/*
		 * X2 channel for eNB-to-eNB communication.
		 * NOTE: this can generate network feedback.
		 */

		x2_compute();

		/* Sleep for one second. */
		sleep(1);
	} while(iface_alive);

out:
	em_terminate_agent(sim_ID);
	log_release();

	return 0;
}

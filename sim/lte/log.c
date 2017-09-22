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
 * Empower Agent simulator log module.
 */

#include "emsim.h"

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

/* File descriptor used for logging errors/debug messages. */
FILE * sim_logFD = 0;

/******************************************************************************
 * Public procedures implementation:                                          *
 ******************************************************************************/

int log_init(char * log_path)
{
	sim_logFD = fopen(log_path, "w");

	if(!sim_logFD) {
		printf("%-"LOG_PROC_SPACE"s: Could not open the log file!\n",
			__func__);

		return ERR_LOG_INIT_IO;
	}

	LOG_TRACE("Log module started.\n");

	return 0;
}

int log_release(void)
{
	LOG_TRACE("Log module is shutting down.\n");

	fflush(sim_logFD);
	fclose(sim_logFD);

	return 0;
}

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

#ifndef __EM_SIM_LOG_H
#define __EM_SIM_LOG_H

#include <stdio.h>

#include <emtypes.h>

#define LOG_PROC_SPACE		"25"

#define LOG_TRAW(x, ...)	fprintf(sim_logFD, x, ##__VA_ARGS__)
#define LOG_TRACE(x, ...)	fprintf(                             \
	sim_logFD, "%-"LOG_PROC_SPACE"s: "x, __func__, ##__VA_ARGS__)

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

/* Log file descriptor. */
extern FILE * sim_logFD;

/******************************************************************************
 * Public accessible procedures:                                              *
 ******************************************************************************/

/* Initializes the log subsystems.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
int log_init(char * log_path);

/* Releases the log subsystem, causing log to become ineffective.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
int log_release(void);

#endif

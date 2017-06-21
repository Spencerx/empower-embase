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
 * Empower Agent RAN sharing module.
 */

#ifndef __EM_SIM_RAN_H
#define __EM_SIM_RAN_H

#include <emtypes.h>

#include "phy.h"

#define RAN_MAX_TENANTS		16
#define RAN_INVALID_TENANT	0

#define RAN_MAX_SCHEDULERS	8
#define RAN_SCHEDULER_MAX_NAME	64

/* Provides a description of how RAN sharing info are organized. */
typedef struct __em_sim_ran_sharing_info {
	/* Tenants currently managed. */
	u32 tenants[RAN_MAX_TENANTS];

	/* Number of tenant schedulers. */
	u32 nof_ts;
	/* Schedulers at Tenant level; these are ASCII names. */
	char t_sched[RAN_MAX_SCHEDULERS][RAN_SCHEDULER_MAX_NAME];

	/* Number of UE schedulers. */
	u32 nof_us;
	/* Schedulers at UE level; these are ASCII names. */
	char u_sched[RAN_MAX_SCHEDULERS][RAN_SCHEDULER_MAX_NAME];

	/* DL timed window in sub-frames. */
	u32 DL_win;
	/* Tenant distribution over the DL.
	 * This area is accessible as: [DL_win][DL_prb].
	 */
	u32 * DL;

	/* UL timed window in sub-frames. */
	u32 UL_win;
	/* Tenant distribution over the UL.
	 * This area is accessible as: [UL_win][UL_prb].
	 */
	u32 * UL;
} em_ran_info;

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

/* RAN sharing information of the simulated eNB. */
extern em_ran_info sim_ran;

/******************************************************************************
 * Public accessible procedures:                                              *
 ******************************************************************************/

/* Adds a tenant in the managed ones.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
u32 ran_add_tenant(u32 plmn);

/* Changes the actual window (in sub-frames) to be taken in account by the RAN
 * algorithms.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
u32 ran_change_window(u32 win);

/* Initializes the RAN layer subsystems.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
u32 ran_init();

/* Register a tenant scheduler in the known ones.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
u32 ran_register_tsched(char * name);

/* Register an UE scheduler in the known ones.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
u32 ran_register_usched(char * name);

/* Removes a tenant from the managed ones.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
u32 ran_rem_tenant(u32 plmn);

/******************************************************************************
 * PHY simulation logic:                                                      *
 ******************************************************************************/

u32 ran_compute();

#endif

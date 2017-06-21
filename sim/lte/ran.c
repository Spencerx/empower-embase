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

#include <stdlib.h>
#include <string.h>

#include "emsim.h"

#define LOG_RAN(x, ...)			LOG_TRACE(x, ##__VA_ARGS__)

em_ran_info sim_ran = {{0}};

/******************************************************************************
 * Private procedures:                                                        *
 ******************************************************************************/

u32 ran_adjust_win(void * buf, u32 old_win, u32 new_win, u32 prbs)
{
	void * tmp = 0;	/* Old data. */

	if(buf) {
		/* Save the previous buffer. */
		tmp = buf;

		free(buf);
		buf = 0;
	}

	buf = malloc(sizeof(u32) * new_win * prbs);

	if(!buf) {
		LOG_RAN("No more memory on new RAN buf allocation.\n");
		return ERR_RAN_CWIN_MEMORY;
	}

	/* Preserve old info in case of already populated data. */
	if(tmp) {
		memcpy(
			buf,
			tmp,
			sizeof(u32) * prbs *
			/* NOTE:
			 * Avoid to do buffer overflow if the new window
			 * is smaller than old data.
			 */
			(new_win > old_win ? old_win : new_win));

		/* Finally free the old buffer. */
		free(tmp);
	} else {
		memset(buf, 0, sizeof(u32) * new_win * prbs);
	}

	return SUCCESS;
}

/******************************************************************************
 * Public accessible procedures:                                              *
 ******************************************************************************/

u32 ran_add_tenant(u32 plmn)
{
	int i;
	int f = -1;

	if(plmn == RAN_INVALID_TENANT) {
		LOG_RAN("Tenant id %06d is not valid!\n", plmn);
		return ERR_RAN_ADD_INVALID;
	}

	for(i = 0; i < RAN_MAX_TENANTS; i++) {
		if(f < 0 && sim_ran.tenants[i] == RAN_INVALID_TENANT) {
			f = i;
		}

		if(plmn == sim_ran.tenants[i]) {
			LOG_RAN("Tenant %06d already exists...\n", plmn);
			return ERR_RAN_ADD_EXISTS;
		}
	}

	if(f < 0) {
		LOG_RAN("Not possible to handle more tenants.\n");
		return ERR_RAN_ADD_FULL;
	}

	sim_ran.tenants[f] = plmn;

	LOG_RAN("Tenant %06d added in slot %d.\n", plmn, f);

	return SUCCESS;
}

u32 ran_change_window(u32 win)
{
	u32 op;

	if(win != sim_ran.DL_win) {
		op = ran_adjust_win(
			sim_ran.DL, sim_ran.DL_win, win, sim_phy.DL_prb);

		if(!op) {
			return op;
		}

		sim_ran.DL_win = win;
		LOG_RAN("RAN DL window changed to %d.\n", sim_ran.DL_win);
	}

	if(win != sim_ran.UL_win) {
		op = ran_adjust_win(
			sim_ran.UL, sim_ran.UL_win, win, sim_phy.UL_prb);

		if(!op) {
			return op;
		}

		sim_ran.UL_win = win;
		LOG_RAN("RAN UL window changed to %d.\n", sim_ran.UL_win);
	}

	return SUCCESS;
}

u32 ran_init()
{
	sim_ran.DL_win = 10;
	sim_ran.UL_win = 10;

	sim_ran.DL = malloc(sizeof(u32) * sim_ran.DL_win * sim_phy.DL_prb);
	sim_ran.UL = malloc(sizeof(u32) * sim_ran.UL_win * sim_phy.UL_prb);

	if(!sim_ran.DL || !sim_ran.UL) {
		LOG_RAN("No more memory during initialization.\n");
		return ERR_RAN_INIT_MEMORY;
	}

	memset(sim_ran.DL, 0, sizeof(u32) * sim_ran.DL_win * sim_phy.DL_prb);
	memset(sim_ran.UL, 0, sizeof(u32) * sim_ran.UL_win * sim_phy.UL_prb);

	sim_ran.nof_ts = 1;
	strcpy(sim_ran.t_sched[0], "NopeSched");

	sim_ran.nof_us = 1;
	strcpy(sim_ran.u_sched[0], "NopeSched");

	return SUCCESS;
}

u32 ran_register_tsched(char * name)
{
	int i;
	int f = -1;

	if(!name || name[0] == 0 || strlen(name) >= RAN_SCHEDULER_MAX_NAME) {
		LOG_RAN("Invalid scheduler name!\n");
		return ERR_RAN_TSCH_INVALID;
	}

	for(i = 0; i < RAN_MAX_SCHEDULERS; i++) {
		if(f < 0 && sim_ran.t_sched[i][0] == 0) {
			f = i;
		}

		if(strcmp(name, sim_ran.t_sched[i]) == 0) {
			return ERR_RAN_TSCH_EXISTS;
		}
	}

	if(f < 0) {
		LOG_RAN("No more slots for Tenant schedulers!\n");
		return ERR_RAN_TSCH_FULL;
	}

	strcpy(name, sim_ran.t_sched[f]);
	LOG_RAN("Tenant scheduler %s added to Simulator.\n",
		sim_ran.t_sched[f]);

	return SUCCESS;
}

u32 ran_register_usched(char * name)
{
	int i;
	int f = -1;

	if(!name || name[0] == 0 || strlen(name) >= RAN_SCHEDULER_MAX_NAME) {
		LOG_RAN("Invalid scheduler name!\n");
		return ERR_RAN_USCH_INVALID;
	}

	for(i = 0; i < RAN_MAX_SCHEDULERS; i++) {
		if(f < 0 && sim_ran.u_sched[i][0] == 0) {
			f = i;
		}

		if(strcmp(name, sim_ran.u_sched[i]) == 0) {
			return ERR_RAN_USCH_EXISTS;
		}
	}

	if(f < 0) {
		LOG_RAN("No more slots for UE schedulers!\n");
		return ERR_RAN_USCH_FULL;
	}

	strcpy(name, sim_ran.u_sched[f]);
	LOG_RAN("UE scheduler %s added to Simulator.\n",
		sim_ran.u_sched[f]);

	return SUCCESS;
}

/******************************************************************************
 * PHY simulation logic:                                                      *
 ******************************************************************************/

u32 ran_compute()
{
	return SUCCESS;
}

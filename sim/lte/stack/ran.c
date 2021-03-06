/* Copyright (c) 2018 Kewin Rausch
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
 * Empower Agent simulator RAN module.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <emtypes.h>

#include "../emsim.h"

/* Dif "b-a" two timespec structs and return such value in ms.*/
#define ts_diff_to_ms(a, b) 			\
	(((b.tv_sec - a.tv_sec) * 1000) +	\
	 ((b.tv_nsec - a.tv_nsec) / 1000000))

#define LOG_RAN(x, ...) 	LOG_TRACE(x, ##__VA_ARGS__)

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

em_ran sim_ran = {0};

/******************************************************************************
 * RAN utilities:                                                             *
 ******************************************************************************/

/* User Round-Robin scheduler, ID 1
 *
 * 'validPRGB' is an array of integers with values 0/1, which identifies which 
 * groups can be written by 
 */
u32 ran_user_rr_sched(
	em_mac *        mac,
	em_ran *        ran,
	em_ran_slice * slice,
	int             validPRGB[MAC_DL_PRBG_MAX])
{
	int i;
	int j    = 0;
	int k    = 0;
	int t    = mac->DL.tti % 10;
	int u;
	int * last;

	/* Initialization */
	if (!slice->sched_init) {
		slice->sched_priv = (int *) malloc(sizeof(int));

		if (!slice->sched_priv) {
			return ERR_RAN_INIT_MEMORY;
		}

		slice->sched_init           = 1;
		*((int *)slice->sched_priv) = 0;
	}

	last = (int *)slice->sched_priv;
	u    = (*last + 1) % RAN_USER_MAX;

	/* Perform ONE cycle of every possible UE */
	while (j < RAN_USER_MAX) {
		/* Not a valid User/UE id */
		if (ran->users[u].rnti == RAN_USER_INVALID_ID) {
			goto next;
		}

		for (k = 0; k < RAN_SLICE_MAX; k++) {
			if (ran->users[u].slice[k] == slice->id) {
				break;
			}
		}

		/* The UE does not belong to this Tenant */
		if (k == RAN_SLICE_MAX) {
			goto next;
		}

		/* We got the next valid UE belonging to a certain slice */
		break;

next:
		u = (u + 1) % RAN_USER_MAX;
		j++;
	}

	/* No next UE has been selected */
	if (j >= RAN_USER_MAX) {
		return SUCCESS;
	}

	/* FINAL STEP:
	 * Assign the DL spectrum resources to the selected RNTI 
	 */

	for (i = 0; i < MAC_DL_PRBG_MAX; i++) {
		/* If we are not authorized to use this group, skip it */
		if (!validPRGB[i]) {
			continue;
		}

		for (j = 0; j < MAC_DL_RGS_20; j++) {
			mac->DL.PRBG[t][i].PRBG[j] = ran->users[u].rnti;
		}

		*last = u;

		mac->DL.PRBG[t][i].rnti = ran->users[u].rnti;
		mac->DL.prb_in_use++;
	}

	/* Update the accumulators of active reports */
	for(i = 0; i < MAC_REPORT_MAX; i++) {
		if(!mac->mac_rep[i].mod) {
			continue;
		}

		/* PRB used must be casted per sub-frame */
		mac->mac_rep[i].DL_acc += (mac->DL.prb_in_use * sim_loop_int);
	}

	return SUCCESS;
}

/* Map for Tenant Static Scheduler (TSS) */
u64 ran_tss_map[PHY_SUBFRAME_X_FRAME][32];

/* Tenant static-assignment scheduler, ID 1 */
u32 ran_slice_static_sched(em_mac * mac, em_ran * ran)
{
	int d;
	int i;
	int j;
	int t = mac->DL.tti % 10;
	int validPRGB[MAC_DL_PRBG_MAX] = {0};

	/* Invalidate the subframe at the start */
	for (i = 0; i < MAC_DL_PRBG_MAX; i++) {
		for (j = 0; j < MAC_DL_RGS_20; j++) {
			mac->DL.PRBG[t][i].PRBG[j] = RAN_USER_INVALID_ID;
		}

		mac->DL.PRBG[t][i].rnti = RAN_USER_INVALID_ID;
	}

	/* Loop over all the Tenants */
	for (i = 0; i < RAN_SLICE_MAX; i++) {
		d = 0;

		/* Skip invalid slices */
		if (ran->slices[i].id == RAN_SLICE_INVALID_ID) {
			continue;
		}

		/* For this subframe, select the group which belong to this 
		 * slice.
		 */
		for (j = 0; j < MAC_DL_PRBG_MAX; j++) {
			if (ran_tss_map[t][j] == ran->slices[i].id) {
				d = 1;
				validPRGB[j] = 1;
			}
		}

		/* Detected some areas for this slice? */
		if (d) {
			ran_user_rr_sched(
				mac,
				ran,
				&ran->slices[i],
				validPRGB);
		}
	}

	return SUCCESS;
}

 /* Perform RAN sharing simulation on the DL */
u32 ran_DL_scheduler(em_mac * mac, em_ue * ues, u32 nof_ues)
{
	return ran_slice_static_sched(mac, &sim_ran);
}

/******************************************************************************
 * RAN simulation logic:                                                      *
 ******************************************************************************/

u32 ran_bootstrap() {
	int i;
	int j;

	LOG_RAN("Resetting Radio Access Network(RAN) Sharing\n");

	/* Reset all the UE informations */
	for (i = 0; i < RAN_USER_MAX; i++) {
		memset(&sim_ran.users[i], 0, sizeof(em_ran_user));
	}

	/* Reset all the Tenant informations; skip slice 1 */
	for (i = 1; i < RAN_SLICE_MAX; i++) {
		sim_ran.slices[i].id = RAN_SLICE_INVALID_ID;
		/* Hard-coded RR scheduler for slices */
		sim_ran.slices[i].sched_id   = 1;
		sim_ran.slices[i].sched_init = 0;

		if (sim_ran.slices[i].sched_priv) {
			free(sim_ran.slices[i].sched_priv);
			sim_ran.slices[i].sched_priv = 0;
		}
	}

	LOG_RAN("RAN Sharing turned ON\n");

	for (i = 0, j = 0; i < RAN_USER_MAX; i++) {
		/* A valid UE detected */
		if (sim_ues[i].rnti != UE_RNTI_INVALID) {
			sim_ran.users[j].rnti      = sim_ues[i].rnti;
			sim_ran.users[j].slice[0] = RAN_SLICE_DEFAULT;

			LOG_RAN("Existing user %d added to Tenant 1\n", 
				sim_ues[i].rnti);

			j++;
		}
	}

	return SUCCESS;
}


u32 ran_init()
{
	/* Hardcoded limitation of using the static scheduler for slices */
	sim_ran.sched_id = 1;

	/* Initial UE connection default slice */
	sim_ran.slices[0].id         = RAN_SLICE_DEFAULT;
	sim_ran.slices[0].sched_id   = 1;
	sim_ran.slices[0].sched_init = 0;

	/* All the UEs belongs to the Default Tenant, to allow them completing
	 * their connection procedures.
	 */

	ran_tss_map[0][0]  = RAN_SLICE_DEFAULT;
	ran_tss_map[0][1]  = RAN_SLICE_DEFAULT;
	ran_tss_map[0][2]  = RAN_SLICE_DEFAULT;
	ran_tss_map[0][3]  = RAN_SLICE_DEFAULT;
	ran_tss_map[0][4]  = RAN_SLICE_DEFAULT;
	ran_tss_map[0][5]  = RAN_SLICE_DEFAULT;
	ran_tss_map[0][6]  = RAN_SLICE_DEFAULT;
	ran_tss_map[0][7]  = RAN_SLICE_DEFAULT;
	ran_tss_map[0][8]  = RAN_SLICE_DEFAULT;
	ran_tss_map[0][9]  = RAN_SLICE_DEFAULT;

	ran_tss_map[0][10] = RAN_SLICE_DEFAULT;
	ran_tss_map[0][11] = RAN_SLICE_DEFAULT;
	ran_tss_map[0][12] = RAN_SLICE_DEFAULT;
	ran_tss_map[0][13] = RAN_SLICE_DEFAULT;
	ran_tss_map[0][14] = RAN_SLICE_DEFAULT;
	ran_tss_map[0][15] = RAN_SLICE_DEFAULT;
	ran_tss_map[0][16] = RAN_SLICE_DEFAULT;
	ran_tss_map[0][17] = RAN_SLICE_DEFAULT;
	ran_tss_map[0][18] = RAN_SLICE_DEFAULT;
	ran_tss_map[0][19] = RAN_SLICE_DEFAULT;

	ran_tss_map[0][20] = RAN_SLICE_DEFAULT;
	ran_tss_map[0][21] = RAN_SLICE_DEFAULT;
	ran_tss_map[0][22] = RAN_SLICE_DEFAULT;
	ran_tss_map[0][23] = RAN_SLICE_DEFAULT;
	ran_tss_map[0][24] = RAN_SLICE_DEFAULT;
	ran_tss_map[0][25] = RAN_SLICE_DEFAULT;
	ran_tss_map[0][26] = RAN_SLICE_DEFAULT;
	ran_tss_map[0][27] = RAN_SLICE_DEFAULT;
	ran_tss_map[0][28] = RAN_SLICE_DEFAULT;
	ran_tss_map[0][29] = RAN_SLICE_DEFAULT;

	ran_tss_map[0][30] = RAN_SLICE_DEFAULT;
	ran_tss_map[0][31] = RAN_SLICE_DEFAULT;

	return SUCCESS;
}

u32 ran_add_user(u16 user, u64 slice) {
	int i;
	int j;
	int f = -1;

	/* Check if the user exists of if there are any free slots */
	for (i = 0; i < RAN_USER_MAX; i++) {
		if (f < 0 && sim_ran.users[i].rnti == RAN_USER_INVALID_ID) {
			f = i;
		}

		if (sim_ran.users[i].rnti == user) {
			break;
		}
	}

	/* Select the right index */
	if (i >= RAN_USER_MAX) {
		if (f < 0) {
			LOG_RAN("No more free user slots\n");
			return ERR_RAN_ADD_FULL;
		}

		i = f;
	}

	sim_ran.users[i].rnti = user;

	/* Check if the user is already part or such slice or get a free slot
	 */
	for (j = 0, f = -1; j < RAN_SLICE_MAX; j++) {
		if (f < 0 && sim_ran.users[i].slice[j] ==
			RAN_SLICE_INVALID_ID) {

			f = j;
		}

		if (sim_ran.users[i].slice[j] == slice) {
			break;
		}
	}

	if (j >= RAN_SLICE_MAX) {
		if (f < 0) {
			LOG_RAN("No slice slots free for user %d\n", user);
			return ERR_RAN_ADD_FULL;
		}

		j = f;
	}
	
	sim_ran.users[i].slice[j] = slice;

	LOG_RAN("RAN added new association, %d --> %ld\n",
		sim_ran.users[i].rnti,
		sim_ran.users[i].slice[j]);

	return SUCCESS;
}

u32 ran_rem_user(u16 user, u64 slice) {
	int i;
	int j;
	int f = -1;

	/* Check if the user exists of if there are any free slots */
	for (i = 0; i < RAN_USER_MAX; i++) {
		if (sim_ran.users[i].rnti == user) {
			break;
		}
	}

	/* Select the right index */
	if (i >= RAN_USER_MAX) {
		if (f < 0) {
			LOG_RAN("RAN user %d don't exists\n", user);
			return ERR_RAN_REM_INVALID;
		}

		i = f;
	}

	/* No slice specified; remove the entire user */
	if (!slice) {
		memset(&sim_ran.users[i], 0, sizeof(em_ran_user));
		LOG_RAN("RAN user %d removed\n", user);
		return SUCCESS;
	}

	/* Do not remove the default slice */
	if (slice == RAN_SLICE_DEFAULT) {
		LOG_RAN("RAN user association with default slice unchanged\n");
		return SUCCESS;
	}

	/* Find and remove the exact slice association */
	for (j = 0; j < RAN_SLICE_MAX; j++) {
		if (sim_ran.users[i].slice[j] == slice) {
			sim_ran.users[i].slice[j] = RAN_SLICE_INVALID_ID;
			break;
		}
	}

	LOG_RAN("RAN user %d association to slice %ld removed\n", 
		user, slice);

	return SUCCESS;
}

u32 ran_add_slice(u64 slice, u32 sched)
{
	int i;
	int f = -1;

	/* Check if the user exists of if there are any free slots */
	for (i = 0; i < RAN_SLICE_MAX; i++) {
		if (f < 0 && sim_ran.slices[i].id == RAN_SLICE_INVALID_ID) {
			f = i;
		}

		if (sim_ran.slices[i].id == slice) {
			break;
		}
	}

	if (i >= RAN_SLICE_MAX) {
		if (f < 0) {
			LOG_RAN("No more free RAN Tenant slots\n");
			return ERR_RAN_ADD_FULL;
		}

		i = f;
	}

	/* Hardcoded RR user scheduler for everyone! */
	if (sched != 1) {
		LOG_RAN("RAN Tenant scheduler not RR!\n");
		return ERR_RAN_ADD_INVALID;
	}

	sim_ran.slices[i].id         = slice;
	sim_ran.slices[i].sched_id   = 1;
	sim_ran.slices[i].sched_init = 0;

	if (sim_ran.slices[i].sched_priv) {
		free(sim_ran.slices[i].sched_priv);
		sim_ran.slices[i].sched_priv = 0;
	}

	LOG_RAN("New RAN Tenant %ld inserted with scheduler %d\n",
		slice, sched);

	return SUCCESS;
}

u32 ran_rem_slice(u64 slice)
{
	int i;

	/* Check if the user exists of if there are any free slots */
	for (i = 0; i < RAN_SLICE_MAX; i++) {
		if (sim_ran.slices[i].id == slice) {
			break;
		}
	}

	if (i >= RAN_SLICE_MAX) {
		LOG_RAN("RAN Tenant %ld not found\n", slice);
		return ERR_RAN_REM_INVALID;
	}

	sim_ran.slices[i].id         = RAN_SLICE_INVALID_ID;
	sim_ran.slices[i].sched_id   = 0;
	sim_ran.slices[i].sched_init = 0;

	if (sim_ran.slices[i].sched_priv) {
		free(sim_ran.slices[i].sched_priv);
		sim_ran.slices[i].sched_priv = 0;
	}

	LOG_RAN("RAN Tenant %ld removed\n", slice);

	return SUCCESS;
}

u32 ran_format_slice_map(char * buf, int len)
{
	int i;
	int j ;
	int s = 0;

	for (i = 0; i < PHY_SUBFRAME_X_FRAME; i++) {
		for (j = 0; j < 32; j++) {
			/* Stop here, we will overflow, and problably it aready
			 * had!
			 */
			if (s > len) {
				LOG_RAN("!!!!! Overflow while formatting the "
					"slice map\n");

				return len;
			}

			s += sprintf(buf + s, "%ld,", ran_tss_map[i][j]);
		}
	}

	return s;
}

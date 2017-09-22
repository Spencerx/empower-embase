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
 * Empower Agent simulator PHY module.
 */

#include "emsim.h"

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

em_phy sim_phy = {0};

u32 sim_phy_dirty = 0;

/******************************************************************************
 * Public accessible procedures:                                              *
 ******************************************************************************/

u32 phy_init()
{
	int i;
	int j;

	sim_phy.pci       = 1;
	sim_phy.DL_earfcn = 1750;
	sim_phy.UL_earfcn = sim_phy.DL_earfcn + 18000;
	sim_phy.DL_prb    = 25;
	sim_phy.UL_prb    = 25;

	for(i = 0; i < sim_phy.DL_prb; i++) {
		for(j = 0; j < PHY_SUBFRAME_X_FRAME; j++) {
			sim_phy.DL[j][i] = 0;
		}
	}

	for(i = 0; i < sim_phy.UL_prb; i++) {
		for(j = 0; j < PHY_SUBFRAME_X_FRAME; j++) {
			sim_phy.UL[j][i] = 0;
		}
	}

	return SUCCESS;
}

/******************************************************************************
 * PHY simulation logic:                                                      *
 ******************************************************************************/

u32 phy_compute_DL()
{
	u32 d = 0;
	int i;

	for(i = 0; i < sim_nof_ues; i++) {
		if(sim_ues[i].rnti != UE_RNTI_INVALID) {
			d += sim_phy.DL_prb / UE_MAX;
		}
	}

	return d;
}

u32 phy_compute_UL() {
	u32 u = 0;
	int i;

	for(i = 0; i < sim_nof_ues; i++) {
		if(sim_ues[i].rnti != UE_RNTI_INVALID) {
			u += sim_phy.UL_prb / UE_MAX;
		}
	}

	return u;
}

u32 phy_compute()
{
	u32 dl = phy_compute_DL();
	u32 ul = phy_compute_UL();

	/* Do not compute on disconnected controller. */
	if(!em_is_connected(sim_ID)) {
		return SUCCESS;
	}

	if(dl != sim_phy.DL_used) {
		sim_phy.DL_used = dl;
		sim_phy_dirty = 1;
	}

	if(ul != sim_phy.UL_used) {
		sim_phy.UL_used = ul;
		sim_phy_dirty = 1;
	}

	if(sim_phy_dirty) {
		/* Operates only if the trigger is defined. */
		if(!sim_cell_stats_trigger) {
			goto no_dirt;
		}
#if 0
		/* Check and remove the trigger eventually. */
		if(!em_has_trigger(
			sim_ID,
			sim_cell_stats_trigger,
			EM_CELL_STATS_TRIGGER)) {

			sim_cell_stats_trigger = 0;
			return SUCCESS;
		}
#if 0
			if(msg_cell_stats(sim_ID, sim_cell_stat_mod, &msg)) {
				return ERR_UNKNOWN;
			}

			if(em_send(sim_ID, msg)) {
				emage_msg__free_unpacked(msg, 0);
				return ERR_UNKNOWN;
			}
#endif
#endif
no_dirt:
		sim_phy_dirty = 0;

	}

	return SUCCESS;
}

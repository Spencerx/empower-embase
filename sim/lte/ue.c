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
 * Empower Agent simulator ue module.
 */

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "emsim.h"

#define LOG_UE(x, ...)          LOG_TRACE(x, ##__VA_ARGS__)

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

/* Number of UEs actually active on this cell. */
u32 sim_nof_ues = 0;

/* UEs information. */
em_ue sim_ues[UE_MAX] = {{0}};

/* Identify if some modifications occurs on the UE list. */
u32 sim_ue_dirty = 0;

/******************************************************************************
 * Public accessible procedures:                                              *
 ******************************************************************************/

int ue_add(u16 pci, u32 earfcn, u16 rnti, u32 plmnid, u64 imsi, int rep)
{
	int i;
	int e;      /* Existing RNTI detected. */
	int f = -1; /* Detected free UE slot. */

	/* Check for already existing RNTIs, and continue to do us until a free
	 * one has been found.
	 */
	do {
		e = 0;

		for(i = 0; i < UE_MAX; i++) {
			if(f == -1 && sim_ues[i].rnti == UE_RNTI_INVALID) {
				f = i;
			}

			if(sim_ues[i].rnti == rnti) {
				e    = 1;
				rnti = ue_rnti_candidate();

				/* Do not break here; we need to find an 'f'*/
				if(f >= 0) {
					break;
				}
			}

			/* Two UE with the same IMSI are not allowed! */
			if(sim_ues[i].imsi == imsi) {
				LOG_UE("IMSI %"PRIu64" already exists!\n",
					imsi);

				return ERR_UE_ADD_EXISTS;
			}
		}
	} while(e);

	/* No slots available. */
	if(f < 0) {
		LOG_UE("No more free UE slots available.\n");
		return ERR_UE_ADD_FULL;
	}

	/* Clean everything before the use. */
	memset(&sim_ues[f], 0, sizeof(em_ue));

	/* Check the validity of the Physical Cell ID */
	for(i = 0; i < PHY_CELL_MAX; i++) {
		if(sim_phy.cells[i].pci == pci) {
			break;
		}
	}

	/* Cell where to include this UE not found */
	if(i == PHY_CELL_MAX) {
		LOG_UE("Cell %d not available for UE\n", pci);
		return ERR_UE_ADD_PCI_UNKNOWN;
	}

	sim_ues[f].pci   = pci;
	sim_ues[f].rnti  = rnti;
	sim_ues[f].plmn  = plmnid;
	sim_ues[f].imsi  = imsi;

	/* WARN: Hard-coded operating on band 7. */
	sim_ues[f].bands[0]        = 7;

	/* Measurements on place in the UE; by default the slot 0 is reserved
	 * to measurements on the attached cell. */
	sim_ues[f].meas[0].id      = 0;
	sim_ues[f].meas[0].tri_id  = -1;
	sim_ues[f].meas[0].pci     = pci;
	sim_ues[f].meas[0].earfcn  = earfcn;

	/* By default the level of the reference signal is at half. */
	sim_ues[f].meas[0].rs.rsrp =
		PHY_RSRP_LOWER - (PHY_RSRP_LOWER - PHY_RSRP_HIGHER) / 2;
	sim_ues[f].meas[0].rs.rsrq =
		PHY_RSRQ_LOWER - (PHY_RSRQ_LOWER - PHY_RSRQ_HIGHER) / 2;

	/* Force the first feedback feedback. */
	sim_ues[f].meas[0].dirty   = 1;

	sim_nof_ues++;

	if(rep) {
		/* Signal that the UEs list is dirty and shall be reported. */
		sim_ue_dirty = 1;
	}

	if(sim_mac.ran) {
		/* Add to the default user */
		ran_add_user(sim_ues[f].rnti, RAN_TENANT_DEFAULT);
	}

	LOG_UE("UE %u added; Cell=%d, PLMN=%x, IMSI=%"PRIu64".\n",
		sim_ues[f].rnti,
		sim_ues[f].pci,
		sim_ues[f].plmn,
		sim_ues[f].imsi);

	return f;
}

int ue_rem(u16 rnti, int rep)
{
	int i;
	int j;

	for(i = 0; i < UE_MAX; i++) {
		if(sim_ues[i].rnti == rnti) {
			sim_nof_ues--;

			sim_ues[i].rnti = UE_RNTI_INVALID;
			sim_ues[i].imsi = 0;
			sim_ues[i].plmn = 0;
			sim_ues[i].pci  = 0;

			/* Reset RRC measurements for that UE */
			for(j = 0; j < UE_RRCM_MAX; j++) {
				sim_ues[i].meas[j].id     = 0;
				sim_ues[i].meas[j].tri_id = 0;
				sim_ues[i].meas[j].mod_id = 0;
			}

			/* Reset the reference signal measured for every
			 * neighbor cell.
			 */
			for(j = 0; j < NEIGH_MAX; j++) {
				sim_neighs[j].rs[i].rsrp = PHY_RSRP_LOWER;
				sim_neighs[j].rs[i].rsrq = PHY_RSRQ_LOWER;
			}

			if (sim_mac.ran) {
				ran_rem_user(rnti, 0);
			}

			LOG_UE("UE %u removed\n", rnti);

			break;
		}
	}

	if(rep) {
		sim_ue_dirty = 1;
	}

	return SUCCESS;
}

u16 ue_rnti_candidate(void)
{
	return (u16)(rand() % (UE_RNTI_RESERVED - 1)) + 1;
}

/******************************************************************************
 * UE simulation logic:                                                       *
 ******************************************************************************/

u32 ue_compute_measurements()
{
	int           i;
	int           j;
	int           k;

	int           mlen;
	char          buf[MEDIUM_BUF];

	int           mi;
	ep_ue_measure m[UE_RRCM_MAX];

	/* Do not compute on disconnected controller. */
	if(!em_is_connected(sim_ID)) {
		return SUCCESS;
	}

	for(i = 0; i < UE_MAX; i++) {
		if(sim_ues[i].rnti == UE_RNTI_INVALID) {
			continue;
		}

		for(j = 0; j < UE_RRCM_MAX; j++) {
			if(sim_ues[i].meas[j].tri_id == 0) {
				continue;
			}

			/* Trigger removed; clean up */
			if(!em_has_trigger(
				sim_ID, sim_ues[i].meas[j].tri_id)) {

				/* Invalidate the trigger. */
				sim_ues[i].meas[j].tri_id = 0;
				sim_ues[i].meas[j].mod_id = 0;
				sim_ues[i].meas[j].dirty  = 0;

				continue;
			}

			if(!sim_ues[i].meas[j].dirty) {
				continue;
			}

			mi = 0;

			m[mi].meas_id = sim_ues[i].meas[j].id;
			m[mi].pci     = sim_ues[i].meas[j].pci;
			m[mi].rsrp    = sim_ues[i].meas[j].rs.rsrp;
			m[mi].rsrq    = sim_ues[i].meas[j].rs.rsrq;

			mi++;

			/* Look in every neighbor for measurements */
			for(k = 0 ; k < NEIGH_MAX; k++) {
				/* Skip neighbor */
				if(sim_neighs[k].id == NEIGH_INVALID_ID) {
					continue;
				}

				m[mi].meas_id = sim_ues[i].meas[j].id;
				m[mi].pci     = (u16)sim_neighs[k].pci;
				m[mi].rsrp    = sim_neighs[k].rs[i].rsrp;
				m[mi].rsrq    = sim_neighs[k].rs[i].rsrq;

				mi++;
			}

			mlen = epf_trigger_uemeas_rep(
				buf,
				MEDIUM_BUF,
				sim_ID,
				sim_ues[i].pci,
				sim_ues[i].meas[j].mod_id,
				mi,
				UE_RRCM_MAX,
				m);

			em_send(sim_ID, buf, mlen);

			/* Keep it dirty if some error occurs. */
			sim_ues[i].meas[j].dirty = 0;
		}
	}

	return SUCCESS;
}

u32 ue_compute(void)
{
	int           i;

	int           mlen;
	char          buf[MEDIUM_BUF];

	int           nof_ues;
	ep_ue_details ued[32];

	/* Do not compute on disconnected controller. */
	if(!em_is_connected(sim_ID)) {
		return SUCCESS;
	}

	/*
	 * If the UEs simulated are marked as dirty, the system will report to
	 * the controller in the case a trigger has been setup.
	 */
	if(sim_ue_dirty) {
		/* Check every time is the trigger is still there. */
		if(!em_has_trigger(
			sim_ID,
			sim_UE_rep_trigger))  {

			return 0;
		}

		for(i = 0, nof_ues = 0; i < UE_MAX; i++) {
			if(sim_ues[i].rnti == UE_RNTI_INVALID) {
				continue;
			}

			/* We have a limited amount of RRC meas. to send */
			if(nof_ues >= sim_UE_rep_max) {
				break;
			}

			ued[nof_ues].rnti = sim_ues[i].rnti;
			ued[nof_ues].imsi = sim_ues[i].imsi;
			ued[nof_ues].plmn = sim_ues[i].plmn;
			ued[nof_ues].pci  = sim_ues[i].pci;

			nof_ues++;
		}

		mlen = epf_trigger_uerep_rep(
			buf,
			MEDIUM_BUF,
			sim_ID,
			sim_phy.cells[0].pci,
			sim_UE_rep_mod,
			nof_ues,
			sim_UE_rep_max,
			ued);

		if(mlen > 0) {
			em_send(sim_ID, buf, mlen);
		}

		/* No dirty anymore.
		 * Failures keeps the flag dirty.
		 */
		sim_ue_dirty = 0;
	}

		/* Perform computation on UE measurement level. */
	return ue_compute_measurements();
}

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
 * Empower Agent simulator MAC module.
 */

#include <math.h>
#include <time.h>

#include "../emsim.h"
#include "stack_priv.h"

#define LOG_MAC(x, ...) 	LOG_TRACE(x, ##__VA_ARGS__)

#define LOG_MAC(x, ...)	LOG_TRACE(x, ##__VA_ARGS__)

/* Dif "b-a" two timespec structs and return such value in ms.*/
#define ts_diff_to_ms(a, b) 			\
	(((b.tv_sec - a.tv_sec) * 1000) +	\
	 ((b.tv_nsec - a.tv_nsec) / 1000000))

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

em_mac sim_mac = {0};

/******************************************************************************
 * Round-Robin schedulers:                                                    *
 ******************************************************************************/

 /* These variables are used locally for RR schedulers only: */

 int rr_DL_last = 0;	/* Index of the last UE scheduled in DL */
 int rr_UL_last = 0;	/* Index of the last UE scheduled in UL */

/* Performs RR operations on existing UE. This procedure is called every 
 * scheduler time unit (stu), and this variable can be adjusted to slow down or
 * speed up the computation.
 *
 * This scheduler assign one entire DL subframe per connected UE; RR style.
 */
u32 mac_rr_DL_scheduler(em_mac * mac, em_ue * ues, u32 nof_ues)
{
	int i    = (rr_DL_last + 1) % UE_MAX;
	int j;
	int t    = mac->DL.tti % 10;
	u16 rnti = UE_RNTI_INVALID;

	/* No-one to use the DL spectrum; clean ond go on... */
	if (nof_ues == 0) {
		goto assign;
	}

	/* Start from next index (module) and perform a entire loop on the UEs.
	 * Get the next valid one.
	 */
	//while (i != rr_DL_last) {
	for(; i < UE_MAX; i = (i + 1) % UE_MAX) {
		/* We pick a valid UE */
		if (ues[i].rnti != UE_RNTI_INVALID) {
			rnti = ues[i].rnti;
			rr_DL_last = i;
			break;
		}
	}

/* Assign the DL spectrum resources to the selected RNTI */
assign:
	for(i = 0; i < MAC_DL_PRBG_MAX; i++) {
		for(j = 0; j < MAC_DL_RGS_20; j++){
			mac->DL.PRBG[t][i].PRBG[j] = rnti;
		}

		mac->DL.PRBG[t][i].rnti = rnti;
	}

	/* Assume using all the resources of this sub-frame */
	mac->DL.prb_in_use += mac->DL.prb_max;

	return SUCCESS;
}

/* Performs RR operations on existing UE.
 * This scheduler assign 1 UL subframe per connected UE.
 */
u32 mac_rr_UL_scheduler(em_mac * mac, em_ue * ues)
{
	/* UL not supported yet... */

	return SUCCESS;
}

/******************************************************************************
 * MAC utilities:                                                             *
 ******************************************************************************/

u32 mac_dl_compute()
{
	int i;

	/* Stop flag if all the allocations are 5 or higher PRBs */
	int s;

	/* PRBs alloc per UE */
	int alloc[UE_MAX] = {0};

	/* PRB steps: this is the amount of PRB per single UE */
	int prbs;
	/* Actually used PRBS */
	int prbu = 0;
	/* Total PRB left */
	int prbt = sim_mac.DL_prb_max;

	if(sim_nof_ues) {
		prbs = sim_mac.DL_prb_max / sim_nof_ues;
	} else {
		prbs = 0;
	}

	/* Keep the used PRB used by each UE between 1 and 5 per UE */
	if(prbs == 0) {
		prbs = 1;
	} else {
		if(prbs > 5) {
			prbs = 5;
		}
	}

	/* Loop on all the UEs and perform initial allocation */
	for(i = 0; i < UE_MAX; i++) {
		if(sim_ues[i].rnti == UE_RNTI_INVALID) {
			continue;
		}

		/* No more resources to assign!*/
		if(prbt <= 0) {
			continue;
		}

		/* Less resources left than step ones; adapt it */
		if(prbt <= prbs) {
			prbs = prbt;
		}

		prbt     -= prbs;
		alloc[i] += prbs;
		prbu     += prbs;

	}

	/* Are there some PRB left behind? */
	if(prbt > 0) {
		/* Loop until all PRBS have been consumed, or while another
		 * condition is triggered (see 's' flag).
		 */
		while(prbt > 0) {
			s = 0;

			/* Reverse loop to allocate the last PRBs */
			for(i = UE_MAX - 1; i >= 0; i--) {
				if(sim_ues[i].rnti == UE_RNTI_INVALID) {
					continue;
				}

				/* Exit condition, for both the loops */
				if(prbt <= 0) {
					break;
				}

				if(alloc[i] < 5) {
					s = 1;

					alloc[i] += 1;
					prbt     -= 1;
					prbu     += 1;
				}
			}

			/* No allocations have be done, since all the UEs got
			 * their maximum amount of PRBS.
			 */
			if(!s) {
				break; /* Exit from endless while */
			}
		}
	}

	/* Update the amount of PRBS used for this sub-frame */
	if(sim_mac.DL_prb_in_use != prbu) {
		sim_mac.DL_prb_in_use = prbu;
	}

	/* Update the amount of PRBS used overall */
	for(i = 0; i < MAC_REPORT_MAX; i++) {
		if(!sim_mac.mac_rep[i].mod) {
			continue;
		}

		/* PRB used must be casted per sub-frame */
		sim_mac.mac_rep[i].DL_acc += (prbu * sim_loop_int);
	}

	return SUCCESS;
}

/* The behavior of the UL here is similar of the one in the DL, even if it does
 * not make sense. Usually the UL frame is entirely assigned to one UE, and not
 * to a set of them. Still, statistics are saved not per sub-frame, but per
 * bigger intervals, so this behavior is not immediately visible.
 */
u32 mac_ul_compute()
{
#if 0
	int i;

	/* PRB steps: this is the amount of PRB per single UE */
	int prbs;
	/* Actually used PRBS */
	int prbu = 0;
	/* Total PRB left */
	int prbt = sim_mac.UL_prb_max;

	if(sim_nof_ues) {
		prbs = sim_mac.UL_prb_max / sim_nof_ues;
	} else {
		prbs = 0;
	}

	/* Keep the used PRB used by each UE between 1 and 5 per UE */
	if(prbs == 0) {
		prbs = 1;
	} else {
		if(prbs > 5) {
			prbs = 5;
		}
	}

	for(i = 0; i < UE_MAX; i++) {
		if(sim_ues[i].rnti == UE_RNTI_INVALID) {
			continue;
		}

		/* No more resources to assign!*/
		if(prbt <= 0) {
			continue;
		}

		/* Less resources left than step ones; adapt it */
		if(prbt <= prbs) {
			prbs = prbt;
		}

		prbt -= prbs;
		prbu += prbs;
	}

	if(sim_mac.UL_prb_in_use != prbu) {
		sim_mac.UL_prb_in_use = prbu;
	}

	for(i=0; i < MAC_REPORT_MAX; i++) {
		if(!sim_mac.mac_rep[i].mod) {
			continue;
		}

		/* PRB used must be casted per sub-frame */
		sim_mac.mac_rep[i].UL_acc += (prbu * sim_loop_int);
	}
#endif

	return SUCCESS;
}

/******************************************************************************
 * MAC simulation logic:                                                      *
 ******************************************************************************/

u32 mac_init()
{
	int i;

	sim_mac.DL.prb_max = 0;
	sim_mac.UL.prb_max = 0;

	for(i = 0; i < sim_phy.nof_cells; i++) {
		if(sim_phy.cells[0].pci != 0xffff) {
			sim_mac.DL.prb_max += sim_phy.cells[0].DL_prb;
			sim_mac.UL.prb_max += sim_phy.cells[0].UL_prb;
		}
	}

	for(i = 0; i < MAC_REPORT_MAX; i++) {
		clock_gettime(CLOCK_REALTIME, &sim_mac.mac_rep[i].last);
	}

	/* 1 second of speed for schedulers */
	sim_mac.stu = 1000;

	sim_mac.DL.tti = 0;
	clock_gettime(CLOCK_REALTIME, &sim_mac.DL.last);

	return ran_init();
}

u32 mac_compute()
{
	int             i;
	int             ret;

	struct timespec now;

	int             mlen;
	char            buf[MEDIUM_BUF];

	ep_macrep_det   mac;

	ret = mac_ul_compute();

	if(ret) {
		return ret;
	}

	ret = mac_dl_compute();

	if(ret) {
		return ret;
	}

	clock_gettime(CLOCK_REALTIME, &now);

	for(i = 0; i < MAC_REPORT_MAX; i++) {
		/* Do not consider invalid reports */
		if(!sim_mac.mac_rep[i].mod) {
			continue;
		}

		if(ts_diff_to_ms(sim_mac.mac_rep[i].last, now) >=
			sim_mac.mac_rep[i].interval) {

			mac.DL_prbs_used   = sim_mac.mac_rep[i].DL_acc;
			mac.DL_prbs_total  = sim_mac.DL.prb_max;

			mac.UL_prbs_used   = sim_mac.mac_rep[i].UL_acc;
			mac.UL_prbs_total  = sim_mac.UL.prb_max;

			mlen = epf_trigger_macrep_rep(
				buf,
				MEDIUM_BUF,
				sim_ID,
				sim_phy.cells[0].pci,
				sim_mac.mac_rep[i].mod,
				&mac);

			em_send(sim_ID, buf, mlen);

			/* Reset the state of this report */
			sim_mac.mac_rep[i].last.tv_nsec = now.tv_nsec;
			sim_mac.mac_rep[i].last.tv_sec  = now.tv_sec;
			sim_mac.mac_rep[i].DL_acc       = 0;
			sim_mac.mac_rep[i].UL_acc       = 0;
		}
	}

	return SUCCESS;
}

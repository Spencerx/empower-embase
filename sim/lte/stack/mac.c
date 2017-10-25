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

/* Dif "b-a" two timespec structs and return such value in ms.*/
#define ts_diff_to_ms(a, b) 			\
	(((b.tv_sec - a.tv_sec) * 1000) +	\
	 ((b.tv_nsec - a.tv_nsec) / 1000000))

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

em_mac sim_mac = {0};

/******************************************************************************
 * MAC utilities:                                                             *
 ******************************************************************************/

u32 mac_dl_compute()
{
	struct timespec now;

	int i;
	int step = floor((float)sim_mac.DL_prb_max / (float)UE_MAX);
	/* User PRBs are proportional to number of UEs active */
	int tot  = step * sim_nof_ues;

	if(sim_mac.DL_prb_in_use != tot) {
		sim_mac.DL_prb_in_use = tot;
	}


	clock_gettime(CLOCK_REALTIME, &now);

	for(i=0; i < MAC_REPORT_MAX; i++) {
		if(!sim_mac.mac_rep[i].mod) {
			continue;
		}

		sim_mac.mac_rep[i].DL_acc += tot;

		if(ts_diff_to_ms(now, sim_mac.mac_rep[i].last) >=
			sim_mac.mac_rep[i].interval) {

			sim_mac.DL_prb_in_use = 0;

			sim_mac.mac_rep[i].last.tv_nsec = now.tv_nsec;
			sim_mac.mac_rep[i].last.tv_sec  = now.tv_sec;

			/* Send report! */
		}
	}

	return SUCCESS;
}

u32 mac_ul_compute()
{
	struct timespec now;

	int i;
	int step = floor((float)sim_mac.UL_prb_max / (float)UE_MAX);
	/* User PRBs are proportional to number of UEs active */
	int tot  = step * sim_nof_ues;

	if(sim_mac.UL_prb_in_use != tot) {
		sim_mac.UL_prb_in_use = tot;
	}

	clock_gettime(CLOCK_REALTIME, &now);

	for(i=0; i < MAC_REPORT_MAX; i++) {
		if(!sim_mac.mac_rep[i].mod) {
			continue;
		}

		sim_mac.mac_rep[i].UL_acc += tot;

		if(ts_diff_to_ms(now, sim_mac.mac_rep[i].last) >=
			sim_mac.mac_rep[i].interval) {

			sim_mac.mac_rep[i].UL_acc = 0;

			sim_mac.mac_rep[i].last.tv_nsec = now.tv_nsec;
			sim_mac.mac_rep[i].last.tv_sec  = now.tv_sec;

			/* Send report! */
		}
	}


	return SUCCESS;
}

/******************************************************************************
 * MAC simulation logic:                                                      *
 ******************************************************************************/

u32 mac_init()
{
	int i;

	for(i=0; i < MAC_REPORT_MAX; i++) {
		clock_gettime(CLOCK_REALTIME, &sim_mac.mac_rep[i].last);
	}

	return SUCCESS;
}

u32 mac_compute()
{
	int ret = mac_ul_compute();

	if(ret) {
		return ret;
	}

	ret = mac_dl_compute();

	return ret;
}

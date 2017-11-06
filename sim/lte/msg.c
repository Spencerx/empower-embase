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
 * Empower Agent simulator protocol parser module.
 */

#include <inttypes.h>

#include <stdlib.h>
#include <string.h>

#include <emage/emage.h>
#include <emage/emproto.h>

#include "emsim.h"

#define LOG_MSG(x, ...) 		LOG_TRACE(x, ##__VA_ARGS__)

/******************************************************************************
 * Private procedures:                                                        *
 ******************************************************************************/

/******************************************************************************
 * Public accessible procedures:                                              *
 ******************************************************************************/

void msg_dump(char * buf, int size)
{
	int i;

	LOG_MSG("Dumping message:\n");

	for(i = 0; i < size; i++) {
		if(i && i % 16 == 0) {
			LOG_TRAW("\n");
		}

		LOG_TRAW("%02x ", (unsigned char)buf[i]);
	}

	LOG_TRAW("\n");
}

int msg_fill_ue_details(ep_ue_details * ues)
{
	int i;
	int j;

	for(i = 0, j = 0; i < UE_MAX; i++) {
		if(sim_ues[i].rnti == UE_RNTI_INVALID) {
			continue;
		}

		if(j >= sim_UE_rep_max) {
			break;
		}

		ues[i].pci  = sim_ues[j].pci;
		ues[i].rnti = sim_ues[j].rnti;
		ues[i].plmn = sim_ues[j].plmn;
		ues[i].imsi = sim_ues[j].imsi;

		j++;
	}

	return j;
}

int msg_fill_ue_measurements(em_ue_rrcm * uem, ep_ue_measure * m)
{
	m->meas_id = uem->id;
	m->pci     = uem->pci;
	m->rsrp    = uem->rs.rsrp;
	m->rsrq    = uem->rs.rsrq;

	return 0;
}

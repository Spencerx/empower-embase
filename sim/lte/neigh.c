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
 * Empower Agent simulator neighbor cell modules.
 */

#include <string.h>

#include "emsim.h"

#define LOG_NEIGH(x, ...) 	LOG_TRACE(x, ##__VA_ARGS__)

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

/* PHY layer used. */
em_neigh sim_neighs[NEIGH_MAX] = {{0}};

/* Number of neighbors actual in use. */
u32 sim_nof_neigh = 0;

/******************************************************************************
 * Public accessible procedures:                                              *
 ******************************************************************************/

int neigh_add_ipv4(u32 id, u16 pci, char * ipv4, int port)
{
	int i;
	int f = -1; /* First free. */

	for(i = 0; i < NEIGH_MAX; i++) {
		if(f < 0 && sim_neighs[i].id == NEIGH_INVALID_ID) {
			f = i;
		}

		if(id == sim_neighs[i].id) {
			LOG_NEIGH("Neighbor %u already exists!\n", id);
			return ERR_NEI_ADD_EXISTS;
		}
	}

	if(f < 0) {
		LOG_NEIGH("No more free neighbors slot!\n");
		return ERR_NEI_ADD_FULL;
	}

	/* Power of all the UE with this new neighbors is at minimum... */
	for(i = 0; i < UE_MAX; i++) {
		sim_neighs[f].rs[i].rsrp = PHY_RSRP_LOWER;
		sim_neighs[f].rs[i].rsrq = PHY_RSRQ_LOWER;
	}

	sim_neighs[f].id  = id;
	sim_neighs[f].pci = pci;

	memcpy(sim_neighs[f].ipv4, ipv4, strnlen(ipv4, 16));

	inet_pton(AF_INET, sim_neighs[f].ipv4, &(sim_neighs[f].saddr.sin_addr));
	sim_neighs[f].saddr.sin_port = htons(port);

	/* Increment the number of known eNBs. */
	sim_nof_neigh++;

	LOG_NEIGH("eNB %u (%d) is now known at address %s:%u.\n",
		sim_neighs[f].id, f,
		sim_neighs[f].ipv4,
		ntohs(sim_neighs[f].saddr.sin_port));

	return f;
}

int neigh_rem(u32 id)
{
	int i;

	/* Search... */
	for(i = 0; i < NEIGH_MAX; i++) {
		if(sim_neighs[i].id == id) {
			LOG_NEIGH("eNB %u removed.\n", sim_neighs[i].id);

			/* Decrement the number of known eNBs. */
			if(sim_nof_neigh > 0) {
				sim_nof_neigh--;
			}

			memset(&sim_neighs[i], 0, sizeof(em_neigh));

			return SUCCESS;
		}
	}

	return ERR_NEI_REM_NOT_FOUND;
}


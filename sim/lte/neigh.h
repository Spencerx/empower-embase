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

#ifndef __EM_SIM_NEIGH_H
#define __EM_SIM_NEIGH_H

#include <arpa/inet.h>
#include <time.h>

#include "phy.h"
#include "ue.h"

/* Neigh with this identifier are considered as invalid. */
#define NEIGH_INVALID_ID		0

/* Maximum number of neighbors taken into account. */
#define NEIGH_MAX			10

/* Describes how a neighbor cell is seen by this one. */
typedef struct __em_sim_neighbor {
	/* Id of the agent which handles such cell. */
	u32 id;

	/* PCI of the neighbor cell. */
	u32 pci;

	/* Reference signal power/quality per UE. */
	em_phy_rs rs[UE_MAX];

	/* Human-readable IPv4 address. */
	char ipv4[16];
	/* Human-readable IPv6 address. */
	char ipv6[32];

	/* Socket compatible address. */
	struct sockaddr_in saddr;

	/* Last time this neighbor contacted us. */
	struct timespec last_seen;
} em_neigh;

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

/* PHY layer used. */
extern em_neigh sim_neighs[NEIGH_MAX];

/* Number of neighbors actual in use. */
extern u32 sim_nof_neigh;

/******************************************************************************
 * Public accessible procedures:                                              *
 ******************************************************************************/

/* Prepare a neighbor slot in order to describe the given cell.
 * Returns the chosen index, or a negative number on error.
 */
int neigh_add_ipv4(u32 id, u16 pci, char * ipv4);

/* Clean the neighbor slot by using the ID.
 * Returns 0 on success, otherwise a negative error code.
 */
int neigh_rem(u32 id);

#endif /* __EM_SIM_NEIGH_H */

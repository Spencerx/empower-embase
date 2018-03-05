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
 * Empower Agent simulator X2 interface module.
 */


#include <arpa/inet.h>
#include <inttypes.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "emsim.h"

#define LOG_X2(x, ...) 		LOG_TRACE(x, ##__VA_ARGS__)

#define X2_BUF_SIZE		1400

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

/* File descriptor used for network communication between eNBs. */
int            sim_x2_fd   = 0;
/* Port used for X2 interface */
unsigned short sim_x2_port = X2_DEFAULT_PORT;

/******************************************************************************
 * Private procedures for X2 module only:                                     *
 ******************************************************************************/

int x2_alive(struct x2_head * head, char * ipv4, unsigned short port)
{
	int i;
	int f = -1;

	/* Somebody with our same id is contacting us!
	 * Some serious mis-configuration is happening in the net.
	 */
	if(ntohl(head->base_id) == sim_ID) {
		LOG_X2("eNB id (%u) conflict with %s\n", head->base_id, ipv4);

		for(i = 0; i < sim_nof_neigh; i++) {
			if(memcmp(ipv4, sim_neighs[i].ipv4, 16) == 0) {
				sim_neighs[i].id = 0;
				break;
			}
		}

		return ERR_X2_ALIVE_ME;
	}

	for(i = 0; i < sim_nof_neigh; i++) {
		if(sim_neighs[i].id == ntohl(head->base_id)) {
			f = i;

			/* The cell id sent to us is always the most updated. */
			if(sim_neighs[i].pci != ntohs(head->cell_id)) {
				sim_neighs[i].pci = ntohs(head->cell_id);
			}

			/* Update the last time we seen it. */
			clock_gettime(CLOCK_REALTIME, &sim_neighs[i].last_seen);
			break;
		}
	}

	/* Not found; then add it to the known eNBs. */
	if(f < 0) {
		LOG_X2("Not known eNB %d is contacting us from %s\n",
			ntohl(head->base_id), ipv4);

		neigh_add_ipv4(
			ntohl(head->base_id), ntohl(head->cell_id), ipv4, port);
	}

	return SUCCESS;
}

int x2_handover(struct x2_head * head, char * buf, unsigned int size)
{
	char           msg[SMALL_BUF] = {0};
	int            mlen;

	unsigned short rnti = ue_rnti_candidate();
	struct x2_ho * ho = (struct x2_ho *)(buf + sizeof(struct x2_head));

	LOG_X2("UE %"PRIu64" handed over to us by eNB %d.\n",
		be64toh(ho->imsi),
		ntohl(head->base_id));

	mlen = epf_single_ho_rep(
		msg,
		SMALL_BUF,
		sim_ID,
		sim_phy.cells[0].pci,
		0,
		ntohl(head->base_id),
		ntohs(head->cell_id),
		ntohs(ho->rnti),
		rnti);

	if(mlen > 0) {
		em_send(sim_ID, msg, mlen);
	}

	ue_add(
		sim_phy.cells[0].pci,
		sim_phy.cells[0].DL_earfcn,
		rnti,
		ntohl(ho->plmnid),
		be64toh(ho->imsi));

	return SUCCESS;
}

int x2_send(void * buf, struct sockaddr_in * addr, unsigned int size)
{
	return sendto(
		sim_x2_fd,
		buf, size,
		0,
		(struct sockaddr *)addr, sizeof(struct sockaddr_in));
}

/******************************************************************************
 * Public procedures implementation:                                          *
 ******************************************************************************/

int x2_init()
{
	int status = 0;
	struct sockaddr_in addr = {0};

	sim_x2_fd = socket(AF_INET, SOCK_DGRAM, 0);

	if(sim_x2_fd < 0) {
		LOG_X2("Could not create X2 socket, error=%d\n", sim_x2_fd);
		return ERR_X2_INIT_SOCKET;
	}

	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port        = htons(sim_x2_port);

	status = bind(
		sim_x2_fd,
		(struct sockaddr*)&addr,
		sizeof(struct sockaddr_in));

	/* Bind the address with the socket. */
	if(status) {
		LOG_X2("Could not bind X2 socket, error=%d\n", status);
		return ERR_X2_INIT_BIND;
	}

	return SUCCESS;
}

int x2_hand_over(u16 rnti, u32 enb)
{
	int i;
	int u = -1;
	int e = -1;

	char buf[64] = {0};

	struct x2_head * hdr = (struct x2_head *)buf;
	struct x2_ho * ho    = (struct x2_ho *)(buf + sizeof(struct x2_head));

	for(i = 0; i < UE_MAX; i++) {
		if(sim_ues[i].rnti == rnti) {
			u = i;
			break;
		}
	}

	for(i = 0; i < NEIGH_MAX; i++) {
		if(sim_neighs[i].id == enb) {
			e = i;
			break;
		}
	}

	if(e < 0) {
		LOG_X2("Neighbor cell not found, e=%u\n", e);
		return ERR_X2_HO_CELL;
	}

	if(u < 0 ) {
		LOG_X2("UE not found, u=%u\n", u);
		return ERR_X2_HO_UE;
	}

	hdr->base_id = htonl(sim_ID);
	hdr->cell_id = htons(sim_phy.cells[0].pci);
	hdr->type    = X2_MSG_HANDOVER;

	ho->rnti     = htons(sim_ues[u].rnti);
	ho->imsi     = htobe64(sim_ues[u].imsi);
	ho->plmnid   = htonl(sim_ues[u].plmn);

	LOG_X2("Handing over UE %x to eNB %d\n", rnti, enb);

	x2_send(
		buf,
		&sim_neighs[e].saddr,
		sizeof(struct x2_head) + sizeof(struct x2_ho));

	return SUCCESS;
}

/******************************************************************************
 * X2 simulation logic:                                                       *
 ******************************************************************************/

u32 x2_compute()
{
	int i;
	int ret;

	char addr[INET_ADDRSTRLEN] = {0};
	char buf[X2_BUF_SIZE] = {0};

	struct x2_head * h;
	struct x2_head j;

	struct sockaddr_in sa;
	/* NOTE: This must be set with the address size to work. */
	socklen_t sa_len = sizeof(struct sockaddr_in);

	/*
	 * Receive stage:
	 */

	/* Continue as long as there are received packets.
	 *
	 * NOTE: I don't expect X2 to be frequently used, so this should not
	 * consume lot of resources and does not need a custom thread.
	 */
	do {
		ret = recvfrom(
			sim_x2_fd,
			buf,
			X2_BUF_SIZE,
			MSG_DONTWAIT | MSG_NOSIGNAL,
			(struct sockaddr *)&sa, &sa_len);

		/* DONTWAIT can returns immediately with -1. */
		if(ret > 0) {
			h = (struct x2_head *)buf;

			switch(h->type) {
			case X2_MSG_ALIVE:
				/* Translate the IP to human readable one. */
				inet_ntop(
					AF_INET,
					&(sa.sin_addr),
					addr,
					INET_ADDRSTRLEN);

				x2_alive(h, addr, ntohs(sa.sin_port));
				break;
			case X2_MSG_HANDOVER:
				x2_handover(h, buf, ret);
				break;
			}
		}
	} while(ret > 0);

	/*
	 * Send stage:
	 */

	for(i = 0; i < NEIGH_MAX; i++) {
		/* Only if its a valid one. */
		if(sim_neighs[i].id) {
			j.base_id = htonl(sim_ID);
			j.cell_id = htons(sim_phy.cells[0].pci);
			j.type = X2_MSG_ALIVE;

			/* Send the 'I'm alive' message. */
			ret = x2_send(
				&j,
				&sim_neighs[i].saddr,
				sizeof(struct x2_head));

			if(ret < 0) {
			       LOG_X2("Failed to present to neighbor cells.\n");
			}
		}
	}

	return SUCCESS;
}

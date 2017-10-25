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
 * Empower Agent simulator interface module.
 */

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../emsim.h"
#include "../iface_priv.h"

#define ts_msec(t)	((t.tv_sec * 1000) + (t.tv_nsec / 1000000))

#define IFACE_ENB_ID_MAX	10
#define IFACE_ENB_IP_MAX	15

/*
 * Handover stuff.
 */

u32 iface_enb_ho_mask= 0;
u32 iface_enb_ho_sel = 0;
u32 iface_enb_ho_idx = 0;

/*
 * Add eNB stuff.
 */

u32 iface_enb_add_mask= 0;
u32 iface_enb_add_sel = 0;

u32 iface_enb_add_id_idx = 0;
char iface_enb_add_id[IFACE_ENB_ID_MAX + 1] = {0};

u32 iface_enb_add_ip_idx = 0;
char iface_enb_add_ip[IFACE_ENB_IP_MAX + 1] = {0};

/*
 * UE measurements for neighbor cell/
 */

u32 iface_enb_uenm_mask = 0;

/*
 * Main UI stuff:
 */

s32 iface_enb_sel = 1;
s32 iface_enb_sel_idx = 0;

/******************************************************************************
 * UE measurements management for neighbor cells.                             *
 ******************************************************************************/

int iface_enb_handle_uenm_input(int key)
{
	switch(key) {
	/*
	 * These are 'hidden' commands that will be either documented or removed
	 * in the near future.
	 */
	case 'w':
		if(iface_ue_sel_idx < 0 || iface_ue_sel_idx >= UE_MAX) {
			break;
		}

		sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].rsrp += 0.1;

		if(sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].rsrp >
			PHY_RSRP_HIGHER) {

			sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].
				rsrp = PHY_RSRP_HIGHER;
		}

		/* This trigger an update for the controller. */
		sim_ues[iface_ue_sel_idx].meas[0].dirty = 1;

		break;
	case 'q':
		if(iface_ue_sel_idx < 0 || iface_ue_sel_idx >= UE_MAX) {
			break;
		}

		sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].rsrp -= 0.1f;

		if(sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].rsrp <
			PHY_RSRP_LOWER) {

			sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].
				rsrp = PHY_RSRP_LOWER;
		}

		/* This trigger an update for the controller. */
		sim_ues[iface_ue_sel_idx].meas[0].dirty = 1;

		break;
	case 'r':
		if(iface_ue_sel_idx < 0 || iface_ue_sel_idx >= UE_MAX) {
			break;
		}

		sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].rsrp += 1.0f;

		if(sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].rsrp >
			PHY_RSRP_HIGHER) {

			sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].
				rsrp = PHY_RSRP_HIGHER;
		}

		/* This trigger an update for the controller. */
		sim_ues[iface_ue_sel_idx].meas[0].dirty = 1;

		break;
	case 'e':
		if(iface_ue_sel_idx < 0 || iface_ue_sel_idx >= UE_MAX) {
			break;
		}

		sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].rsrp -= 1.0f;

		if(sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].rsrp <
			PHY_RSRP_LOWER) {

			sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].
				rsrp = PHY_RSRP_LOWER;
		}

		/* This trigger an update for the controller. */
		sim_ues[iface_ue_sel_idx].meas[0].dirty = 1;

		break;
	case 's':
		if(iface_ue_sel_idx < 0 || iface_ue_sel_idx >= UE_MAX) {
			break;
		}

		sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].rsrq += 0.1;

		if(sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].rsrq >
			PHY_RSRQ_HIGHER) {

			sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].
				rsrq = PHY_RSRQ_HIGHER;
		}

		/* This trigger an update for the controller. */
		sim_ues[iface_ue_sel_idx].meas[0].dirty = 1;

		break;
	case 'a':
		if(iface_ue_sel_idx < 0 || iface_ue_sel_idx >= UE_MAX) {
			break;
		}

		sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].rsrq -= 0.1f;

		if(sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].rsrq <
			PHY_RSRQ_LOWER) {

			sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].
				rsrq = PHY_RSRQ_LOWER;
		}

		/* This trigger an update for the controller. */
		sim_ues[iface_ue_sel_idx].meas[0].dirty = 1;

		break;
	case 'f':
		if(iface_ue_sel_idx < 0 || iface_ue_sel_idx >= UE_MAX) {
			break;
		}

		sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].rsrq += 1.0f;

		if(sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].rsrq >
			PHY_RSRQ_HIGHER) {

			sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].
				rsrq = PHY_RSRQ_HIGHER;
		}

		/* This trigger an update for the controller. */
		sim_ues[iface_ue_sel_idx].meas[0].dirty = 1;

		break;
	case 'd':
		if(iface_ue_sel_idx < 0 || iface_ue_sel_idx >= UE_MAX) {
			break;
		}

		sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].rsrq -= 1.0f;

		if(sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].rsrq <
			PHY_RSRQ_LOWER) {

			sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].
				rsrq = PHY_RSRQ_LOWER;
		}

		/* This trigger an update for the controller. */
		sim_ues[iface_ue_sel_idx].meas[0].dirty = 1;

		break;
	/* This is the ESCape key; remove this mask. */
	case 27:
		iface_enb_uenm_mask = 0;
		break;
	}

	return SUCCESS;
}

int iface_enb_draw_uenm()
{
	int i;
	float j;

	float p = sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].rsrp;
	float q = sim_neighs[iface_enb_sel_idx].rs[iface_ue_sel_idx].rsrq;

	char ti[] = "Measurements for UE ";
	char in[] = "Use {q,w,e,r} for RSRP, {a,s,d,f} for RSRQ";

	/* Has the RNTI been removed in the same time? */
	if(sim_ues[iface_ue_sel_idx].rnti == 0) {
		/* Navigate to the UE screen. */
		iface_to_ue_screen();
	}

	attron(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));

	for(i = 0; i < 7; i++) {
		for(j = 16; j < iface_col - 16; j++) {
			move((iface_row / 2) - 3 + i, j);
			printw(" ");
		}
	}

	move((iface_row / 2) - 3, (iface_col / 2) - (sizeof(ti) / 2));
	printw("%s", ti);
	printw("%d", sim_ues[iface_ue_sel_idx].rnti);

	move(((iface_row / 2) - 1), (iface_col / 2) - 19);
	printw("RSRP: ");

	if(p < UE_RSRP_ERR_LIMIT) {
		attron(COLOR_PAIR(IFACE_CPAIR_ERROR_H));
	} else if (p < UE_RSRP_WARN_LIMIT) {
		attron(COLOR_PAIR(IFACE_CPAIR_WARNING_H));
	} else {
		attron(COLOR_PAIR(IFACE_CPAIR_OK_H));
	}

	printw("%.2f ", p);

	if(p < UE_RSRP_ERR_LIMIT) {
		attroff(COLOR_PAIR(IFACE_CPAIR_ERROR_H));
		attron(COLOR_PAIR(IFACE_CPAIR_H_ERROR));
	} else if (p < UE_RSRP_WARN_LIMIT) {
		attroff(COLOR_PAIR(IFACE_CPAIR_WARNING_H));
		attron(COLOR_PAIR(IFACE_CPAIR_H_WARNING));
	} else {
		attroff(COLOR_PAIR(IFACE_CPAIR_OK_H));
		attron(COLOR_PAIR(IFACE_CPAIR_H_OK));
	}

	if(p < UE_RSRP_ERR_LIMIT) {
		attron(COLOR_PAIR(IFACE_CPAIR_H_ERROR));
	} else if (p < UE_RSRP_WARN_LIMIT) {
		attron(COLOR_PAIR(IFACE_CPAIR_H_WARNING));
	} else {
		attron(COLOR_PAIR(IFACE_CPAIR_H_OK));
	}

	move((iface_row / 2 - 1), (iface_col / 2));
	for(j = 0.0; j < 60.0; j += 3.0) {
		if(PHY_RSRP_LOWER + j <= p) {
			printw(" ");
		}
	}

	if(p < UE_RSRP_ERR_LIMIT) {
		attroff(COLOR_PAIR(IFACE_CPAIR_H_ERROR));
	} else if (p < UE_RSRP_WARN_LIMIT) {
		attroff(COLOR_PAIR(IFACE_CPAIR_H_WARNING));
	} else {
		attroff(COLOR_PAIR(IFACE_CPAIR_H_OK));
	}

	attron(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));

	move((iface_row / 2) + 1, (iface_col / 2) - 19);
	printw("RSRQ: ");

	if(q < UE_RSRQ_ERR_LIMIT) {
		attron(COLOR_PAIR(IFACE_CPAIR_ERROR_H));
	} else if (q < UE_RSRQ_WARN_LIMIT) {
		attron(COLOR_PAIR(IFACE_CPAIR_WARNING_H));
	} else {
		attron(COLOR_PAIR(IFACE_CPAIR_OK_H));
	}

	printw("%.2f ", q);

	if(q < UE_RSRQ_ERR_LIMIT) {
		attroff(COLOR_PAIR(IFACE_CPAIR_ERROR_H));
		attron(COLOR_PAIR(IFACE_CPAIR_H_ERROR));
	} else if (q < UE_RSRQ_WARN_LIMIT) {
		attroff(COLOR_PAIR(IFACE_CPAIR_WARNING_H));
		attron(COLOR_PAIR(IFACE_CPAIR_H_WARNING));
	} else {
		attroff(COLOR_PAIR(IFACE_CPAIR_OK_H));
		attron(COLOR_PAIR(IFACE_CPAIR_H_OK));
	}

	move((iface_row / 2) + 1, (iface_col / 2));
	for(j = 0.0; j < 17.0; j += 0.85) {
		if(PHY_RSRQ_LOWER + j <= q) {
			printw(" ");
		}
	}

	if(q < UE_RSRQ_ERR_LIMIT) {
		attroff(COLOR_PAIR(IFACE_CPAIR_H_ERROR));
	} else if (q < UE_RSRQ_WARN_LIMIT) {
		attroff(COLOR_PAIR(IFACE_CPAIR_H_WARNING));
	} else {
		attroff(COLOR_PAIR(IFACE_CPAIR_H_OK));
	}

	attron(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));

	move((iface_row / 2) + 3, (iface_col / 2) - sizeof(in) / 2);
	printw("%s", in);

	attroff(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));

	return SUCCESS;
}

/******************************************************************************
 * Handover management.                                                       *
 ******************************************************************************/

int iface_enb_handover_input(int key)
{
	switch(key) {
	case KEY_UP:
		iface_enb_ho_sel--;

		if(iface_enb_ho_sel < 0) {
			iface_enb_ho_sel = 0;
		}

		break;
	case KEY_DOWN:
		iface_enb_ho_sel++;

		if(iface_enb_ho_sel > sim_nof_ues - 1) {
			iface_enb_ho_sel = sim_nof_ues - 1;
		}

		break;
	/* This is the ENTER key; time to insert the new UE. */
	case 10:
		/* Perform the hand-over. */
		x2_hand_over(iface_enb_sel_idx, iface_enb_ho_idx);
		/* Remove the UE from here. */
		ue_rem(iface_enb_ho_idx);

		iface_enb_ho_mask = 0;
		break;
	/* This is the ESCape key; remove this mask. */
	case 27:
		iface_enb_ho_mask = 0;
		break;
	}
	return SUCCESS;
}

int iface_enb_draw_ho()
{
	int i;
	int j;
	int s = 0;

	/* Upper row where to start from. */
	int u = (iface_row / 2) - 2 - (sim_nof_ues / 2);

	char ti[] = "Select the UE to hand-over to the selected eNB ";

	attron(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));

	for(i = 0; i < 4 + sim_nof_ues; i++) {
		for(j = 16; j < iface_col - 16; j++) {
			move(u + i, j);
			printw(" ");
		}
	}

	move(u - 2, (iface_col / 2) - (sizeof(ti) / 2));
	printw("%s", ti);

	for(i = 0; i < sim_nof_ues; i++) {
		if(!sim_ues[i].rnti) {
			continue;
		}

		move(u + 2 + i, (iface_col / 2) - (15 / 2));

		if(iface_enb_ho_sel == s) {
			iface_enb_ho_idx = s;

			attroff(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));
			printw("%"PRIu64"", sim_ues[i].imsi);
			attron(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));
		} else {
			printw("%"PRIu64"", sim_ues[i].imsi);
		}

		s++;
	}

	move((iface_row / 2) + 2, iface_col - 16 - 24);
	printw("Press ENTER to hand over");

	attroff(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));

	return SUCCESS;
}

/******************************************************************************
 * Add new eNB.                                                               *
 ******************************************************************************/

int iface_enb_handle_add_input(int key)
{
	switch(key) {
	case KEY_LEFT:
		iface_enb_add_sel--;

		if(iface_enb_add_sel < 0) {
			iface_enb_add_sel = 0;
		}

		break;
	case KEY_RIGHT:
		iface_enb_add_sel++;

		if(iface_enb_add_sel > 1) {
			iface_enb_add_sel = 1;
		}

		break;
	/* This is the ENTER key; time to insert the new UE. */
	case 10:
		if(iface_enb_add_id[0] == 0 ||
			iface_enb_add_ip[0] == 0) {

			break;
		}

		neigh_add_ipv4(
			(unsigned int)atoi(iface_enb_add_id),
			0,
			iface_enb_add_ip);

		/* Reset all... */
		memset(
			iface_enb_add_id,
			0,
			sizeof(char) * IFACE_ENB_ID_MAX + 1);
		memset(
			iface_enb_add_ip,
			0,
			sizeof(char) * IFACE_ENB_IP_MAX + 1);

		iface_enb_add_id_idx= 0;
		iface_enb_add_ip_idx= 0;

		iface_enb_add_sel   = 0;
		iface_enb_add_mask  = 0;

		break;
	/* This is the ESCape key; remove this mask. */
	case 27:
		iface_enb_add_mask = 0;
		break;
	}

	/* Values between 0 and 9 are being pressed on the screen. */
	if(key >= 48 && key <= 57) {
		if(iface_enb_add_sel == 0) {
			if(iface_enb_add_id_idx >= IFACE_ENB_ID_MAX) {
				iface_enb_add_id_idx = IFACE_ENB_ID_MAX - 1;
			}

			iface_enb_add_id[iface_enb_add_id_idx] = key;
			iface_enb_add_id_idx++;
		} else if(iface_enb_add_sel == 1) {
			if(iface_enb_add_ip_idx >= IFACE_ENB_IP_MAX) {
				iface_enb_add_ip_idx = IFACE_ENB_IP_MAX - 1;
			}

			iface_enb_add_ip[iface_enb_add_ip_idx] = key;
			iface_enb_add_ip_idx++;
		}
	}

	/* The dot! */
	if(key == 46) {
		if(iface_enb_add_sel == 1) {
			if(iface_enb_add_ip_idx >= IFACE_ENB_IP_MAX) {
				iface_enb_add_ip_idx = IFACE_ENB_IP_MAX - 1;
			}

			iface_enb_add_ip[iface_enb_add_ip_idx] = key;
			iface_enb_add_ip_idx++;
		}
	}

	/* Back by one. */
	if(key == KEY_BACKSPACE) {
		if(iface_enb_add_sel == 0) {
			iface_enb_add_id_idx--;

			if(iface_enb_add_id_idx < 0) {
				iface_enb_add_id_idx = 0;
			}

			iface_enb_add_id[iface_enb_add_id_idx] = 0;
		} else if(iface_enb_add_sel == 1) {
			iface_enb_add_ip_idx--;

			if(iface_enb_add_ip_idx < 0) {
				iface_enb_add_ip_idx = 0;
			}

			iface_enb_add_ip[iface_enb_add_ip_idx] = 0;
		}
	}

	return SUCCESS;
}

int iface_enb_draw_add()
{
	int i;
	int j;

	char ti[] = "Fill all the eNB-specific information fields";

	attron(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));

	for(i = 0; i < 5; i++) {
		for(j = 16; j < iface_col - 16; j++) {
			move((iface_row / 2) - 2 + i, j);
			printw(" ");
		}
	}

	move((iface_row / 2) - 2, (iface_col / 2) - (sizeof(ti) / 2));
	printw("%s", ti);

	if(iface_enb_add_sel == 0) {
		attroff(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));
	}

	move((iface_row / 2), 24);
	printw("ID: %-11s", iface_enb_add_id);

	if(iface_enb_add_sel == 0) {
		attron(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));
	}

	/* 22 spaces for field. */

	if(iface_enb_add_sel == 1) {
		attroff(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));
	}

	move((iface_row / 2), 39);
	printw("IPv4: %-16s", iface_enb_add_ip);

	if(iface_enb_add_sel == 1) {
		attron(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));
	}

	move((iface_row / 2) + 2, iface_col - 16 - 21);
	printw("Press ENTER to create");

	attroff(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));

	return SUCCESS;
}


/******************************************************************************
 * Main eNB drawing area.                                                     *
 ******************************************************************************/

int iface_enb_handle_input(int key)
{
	if(iface_enb_add_mask) {
		iface_enb_handle_add_input(key);
		return 0;
	}

	if(iface_enb_ho_mask) {
		iface_enb_handover_input(key);
		return 0;
	}

	if(iface_enb_uenm_mask) {
		iface_enb_handle_uenm_input(key);
		return 0;
	}

	switch(key) {
	/* Move up between eNBs. */
	case KEY_UP:
		if(iface_enb_sel > 1) {
			iface_enb_sel--;
		}
		break;
	/* Move down between eNBs. */
	case KEY_DOWN:
		if(iface_enb_sel < sim_nof_neigh) {
			iface_enb_sel++;
		}
		break;
	/* Add a new eNB. */
	case 'a':
		iface_enb_add_mask = 1;
		break;
	/* Remove the selected eNB. */
	case 'r':
		neigh_rem(sim_neighs[iface_enb_sel_idx].id);

		if(iface_enb_sel > 1) {
			iface_enb_sel--;
		}
		break;
	case 'h':
		iface_enb_ho_mask = 1;
		break;
	case 'm':
		iface_enb_uenm_mask = 1;
		break;
	}

	return SUCCESS;
}

/* Main menu put on the left of the main screen. */
int iface_enb_draw_topbar()
{
	int i;
	int j;

	attron(COLOR_PAIR(1));

	for(i = 0; i < 3; i++) {
		for(j = 0; j < iface_col; j++) {
			move(i, j);
			printw(" ");
		}
	}

	move(0, 2);
	printw("Neighborhood eNB screen, perform operations with eNBs:");

	move(1, 8);
	printw("a - Add eNB");

	move(2, 8);
	printw("r - Remove eNB");

	move(1, 24);
	printw("h - Handover");

	move(2, 24);
	printw("m  - UE meas.");

	attroff(COLOR_PAIR(1));

	return SUCCESS;
}

int iface_enb_clean()
{
	iface_enb_add_mask = 0;
	iface_enb_ho_mask = 0;
	iface_enb_uenm_mask = 0;

	return SUCCESS;
}

int iface_enb_draw()
{
	int i;
	int j;
	int s = 1;

	char tmp[64] = {0};
	char th[] = "List of known cells in this eNB";

	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);

	iface_enb_draw_topbar();

	move(5, (iface_col / 2) - (sizeof(th) / 2));
	printw("%s", th);

	/* Print the table header. */
	move(7, 8);
	printw( "eNB id     "      /* 11 */
		"PCI        "      /* 11 */
		"IPv4            " /* 16 */
		"Status  ");

	move(8, 8);
	for(j = 0; j < iface_col - 16; j++) {
		printw("-");
	}

	/* Draw a list of active UEs! */
	for(i = 0; i < NEIGH_MAX; i++) {
		/* Skip empty elements. */
		if(sim_neighs[i].id == 0) {
			continue;
		}

		/* Highlight the selected element, and save its index. */
		if(iface_enb_sel == s) {
			iface_enb_sel_idx = i;
			attron(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));

			move(9 + s, 8);
			for(j = 0; j < iface_col - 16; j++) {
				printw(" ");
			}
		}

		move(9 + s, 8);
		sprintf(tmp, "%d", sim_neighs[i].id);
		printw("%-11s", tmp);

		sprintf(tmp, "%d", sim_neighs[i].pci);
		printw("%-11s", tmp);

		printw("%-16s", sim_neighs[i].ipv4);

		if(ts_msec(now) - ts_msec(sim_neighs[i].last_seen) >= 2500) {
			if(iface_enb_sel == s) {
				attron(COLOR_PAIR(IFACE_CPAIR_ERROR_H));
			} else {
				attron(COLOR_PAIR(IFACE_CPAIR_ERROR));
			}

			printw("Offline");

			if(iface_enb_sel == s) {
				attroff(COLOR_PAIR(IFACE_CPAIR_ERROR_H));
				attron(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));
			} else {
				attroff(COLOR_PAIR(IFACE_CPAIR_ERROR));
			}
		} else {
			if(iface_enb_sel == s) {
				attron(COLOR_PAIR(IFACE_CPAIR_OK_H));
			} else {
				attron(COLOR_PAIR(IFACE_CPAIR_OK));
			}

			printw("Online");

			if(iface_enb_sel == s) {
				attroff(COLOR_PAIR(IFACE_CPAIR_OK_H));
				attron(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));
			} else {
				attroff(COLOR_PAIR(IFACE_CPAIR_OK));
			}
		}

		if(iface_enb_sel == s) {
			attroff(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));
		}

		s++;
	}

	if(iface_enb_add_mask) {
		iface_enb_draw_add();
	} else if(iface_enb_ho_mask) {
		iface_enb_draw_ho();
	} else if(iface_enb_uenm_mask) {
		iface_enb_draw_uenm();
	}

	return SUCCESS;
}

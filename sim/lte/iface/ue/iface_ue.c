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

#include "../../emsim.h"
#include "../iface_priv.h"

/* This is the graphical representation of the selected UE */
s32 iface_ue_sel = 1;
/* The index to the data is updated during UI drawing */
s32 iface_ue_sel_idx = 0;

s32 iface_ue_add_mask= 0;
s32 iface_ue_add_sel = 0;

#define RNTI_MAX 5
u32 iface_ue_add_rnti_idx = 0;
char iface_ue_add_rnti[RNTI_MAX + 1] = {0};

#define IMSI_MAX 15
u32 iface_ue_add_imsi_idx = 0;
char iface_ue_add_imsi[IMSI_MAX + 1] = {0};

#define PLMN_MAX 6
u32 iface_ue_add_plmn_idx = 0;
char iface_ue_add_plmn[PLMN_MAX + 1] = {0};

/******************************************************************************
 * Detailed UE add mask.                                                      *
 ******************************************************************************/

int iface_ue_handle_add_input(int key)
{
	s32 e;

	switch(key) {
	case KEY_LEFT:
		iface_ue_add_sel--;

		if(iface_ue_add_sel < 0) {
			iface_ue_add_sel = 0;
		}

		break;
	case KEY_RIGHT:
		iface_ue_add_sel++;

		if(iface_ue_add_sel > 2) {
			iface_ue_add_sel = 2;
		}

		break;
	/* This is the ENTER key; time to insert the new UE. */
	case 10:
		if(iface_ue_add_rnti[0] == 0 ||
			iface_ue_add_imsi[0] == 0 ||
			iface_ue_add_plmn[0] == 0) {

			break;
		}

		/* Add a new UE with the given details. */
		e = ue_add(
			sim_phy.cells[0].pci,
			sim_phy.cells[0].DL_earfcn,
			(unsigned short)atoi(iface_ue_add_rnti),
			(unsigned int)atoi(iface_ue_add_plmn),
			(unsigned long long)atoll(iface_ue_add_imsi));

		if(e < 0) {
			iface_err = e;
		}

		/* Reset all... */
		memset(iface_ue_add_rnti, 0, sizeof(char) * RNTI_MAX + 1);
		memset(iface_ue_add_plmn, 0, sizeof(char) * PLMN_MAX + 1);
		memset(iface_ue_add_imsi, 0, sizeof(char) * IMSI_MAX + 1);

		iface_ue_add_rnti_idx= 0;
		iface_ue_add_plmn_idx= 0;
		iface_ue_add_imsi_idx= 0;

		iface_ue_add_sel     = 0;
		iface_ue_add_mask    = 0;

		break;
	/* This is the ESCape key; remove this mask. */
	case 27:
		iface_ue_add_mask = 0;
		break;
	}

	/* Values between 0 and 9 are being pressed on the screen. */
	if(key >= 48 && key <= 57) {
		if(iface_ue_add_sel == 0) {
			if(iface_ue_add_rnti_idx >= RNTI_MAX) {
				iface_ue_add_rnti_idx = RNTI_MAX - 1;
			}

			iface_ue_add_rnti[iface_ue_add_rnti_idx] = key;
			iface_ue_add_rnti_idx++;
		} else if(iface_ue_add_sel == 1) {
			if(iface_ue_add_imsi_idx >= IMSI_MAX) {
				iface_ue_add_imsi_idx = IMSI_MAX - 1;
			}

			iface_ue_add_imsi[iface_ue_add_imsi_idx] = key;
			iface_ue_add_imsi_idx++;
		} else {
			if(iface_ue_add_plmn_idx >= PLMN_MAX) {
				iface_ue_add_plmn_idx = PLMN_MAX - 1;
			}

			iface_ue_add_plmn[iface_ue_add_plmn_idx] = key;
			iface_ue_add_plmn_idx++;
		}
	}

	/* Back by one. */
	if(key == KEY_BACKSPACE) {
		if(iface_ue_add_sel == 0) {
			iface_ue_add_rnti_idx--;

			if(iface_ue_add_rnti_idx < 0) {
				iface_ue_add_rnti_idx = 0;
			}

			iface_ue_add_rnti[iface_ue_add_rnti_idx] = 0;
		} else if(iface_ue_add_sel == 1) {
			iface_ue_add_imsi_idx--;

			if(iface_ue_add_imsi_idx < 0) {
				iface_ue_add_imsi_idx = 0;
			}

			iface_ue_add_imsi[iface_ue_add_imsi_idx] = 0;
		} else {
			iface_ue_add_plmn_idx--;

			if(iface_ue_add_plmn_idx < 0) {
				iface_ue_add_plmn_idx = 0;
			}

			iface_ue_add_plmn[iface_ue_add_plmn_idx] = 0;
		}
	}

	return SUCCESS;
}


int iface_ue_draw_add()
{
	int i;
	int j;

	char ti[] = "Fill all the UE-specific information fields";

	attron(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));

	for(i = 0; i < 5; i++) {
		for(j = 16; j < iface_col - 16; j++) {
			move((iface_row / 2) - 2 + i, j);
			printw(" ");
		}
	}

	move((iface_row / 2) - 2, (iface_col / 2) - (sizeof(ti) / 2));
	printw("%s", ti);

	if(iface_ue_add_sel == 0) {
		attroff(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));
	}

	move((iface_row / 2), 24);
	printw("RNTI: %-19s", iface_ue_add_rnti);

	if(iface_ue_add_sel == 0) {
		attron(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));
	}

	/* 22 spaces for field. */

	if(iface_ue_add_sel == 1) {
		attroff(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));
	}

	move((iface_row / 2), 46);
	printw("IMSI: %-19s", iface_ue_add_imsi);

	if(iface_ue_add_sel == 1) {
		attron(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));
	}

	if(iface_ue_add_sel == 2) {
		attroff(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));
	}

	move((iface_row / 2), 68);
	printw("PLMN: %-19s", iface_ue_add_plmn);

	if(iface_ue_add_sel == 2) {
		attron(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));
	}

	move((iface_row / 2) + 2, iface_col - 16 - 21);
	printw("Press ENTER to create");

	attroff(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));

	return SUCCESS;
}

/******************************************************************************
 * Main UE drawing area.                                                      *
 ******************************************************************************/

int iface_ue_handle_input(int key)
{
	/* Inhibit everything because of the focus on the mask. */
	if(iface_ue_add_mask) {
		iface_ue_handle_add_input(key);
		return 0;
	}

	switch(key) {
	/* Move up between UEs. */
	case KEY_UP:
		if(iface_ue_sel > 1) {
			iface_ue_sel--;
		}
		break;
	/* Move down between UEs. */
	case KEY_DOWN:
		if(iface_ue_sel < sim_nof_ues) {
			iface_ue_sel++;
		}
		break;
	/* Add a new UE. */
	case 'a':
		iface_ue_add_mask = 1;
		break;
	/* Remove the selected UE. */
	case 'r':
		ue_rem(sim_ues[iface_ue_sel_idx].rnti);

		if(iface_ue_sel > 1) {
			iface_ue_sel--;
		}
		break;
	/* Increase the RSRP of the selected UE. */
	case 'i':
		sim_ues[iface_ue_sel_idx].meas[0].rs.rsrp += 0.1f;

		if(sim_ues[iface_ue_sel_idx].meas[0].rs.rsrp >
			PHY_RSRP_HIGHER) {

			sim_ues[iface_ue_sel_idx].meas[0].rs.rsrp =
				PHY_RSRP_HIGHER;
		}

		/* Update also the measurement profile for this cell. */
		sim_ues[iface_ue_sel_idx].meas[0].dirty = 1;

		break;
	/* Decrease the RSRP of the selected UE. */
	case 'k':
		sim_ues[iface_ue_sel_idx].meas[0].rs.rsrp -= 0.1f;

		if(sim_ues[iface_ue_sel_idx].meas[0].rs.rsrp <
			PHY_RSRP_LOWER) {

			sim_ues[iface_ue_sel_idx].meas[0].rs.rsrp =
				PHY_RSRP_LOWER;
		}

		/* Update also the measurement profile for this cell. */
		sim_ues[iface_ue_sel_idx].meas[0].dirty = 1;

		break;
	/* Increase the RSRQ of the selected UE. */
	case 'o':
		sim_ues[iface_ue_sel_idx].meas[0].rs.rsrq += 0.1f;

		if(sim_ues[iface_ue_sel_idx].meas[0].rs.rsrq >
			PHY_RSRQ_HIGHER) {

			sim_ues[iface_ue_sel_idx].meas[0].rs.rsrq =
				PHY_RSRQ_HIGHER;
		}

		/* Update also the measurement profile for this cell. */
		sim_ues[iface_ue_sel_idx].meas[0].dirty = 1;

		break;
	/* Decrease the RSRQ of the selected UE. */
	case 'l':
		sim_ues[iface_ue_sel_idx].meas[0].rs.rsrq -= 0.1f;

		if(sim_ues[iface_ue_sel_idx].meas[0].rs.rsrq <
			PHY_RSRQ_LOWER) {

			sim_ues[iface_ue_sel_idx].meas[0].rs.rsrq =
					PHY_RSRQ_LOWER;
		}

		/* Update also the measurement profile for this cell. */
		sim_ues[iface_ue_sel_idx].meas[0].dirty = 1;

		break;
	}

	return SUCCESS;
}

/* Main menu put on the left of the main screen. */
int iface_ue_draw_topbar()
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
	printw("User Equipments screen, perform operations on UEs:");

	move(1, 8);
	printw("a - Add UE");

	move(2, 8);
	printw("r - Remove UE");

	attroff(COLOR_PAIR(1));

	return SUCCESS;
}

int iface_ue_clean()
{
	iface_ue_add_mask = 0;
	return SUCCESS;
}

int iface_ue_draw()
{
	int i;
	int j;
	int s = 1;

	char tmp[64] = {0};
	char th[] = "List of active UE in this eNB";

	iface_ue_draw_topbar();

	move(5, (iface_col / 2) - (sizeof(th) / 2));
	printw("%s", th);

	/* Print the table header. */
	move(7, 8);
	printw( "RNTI      "
		"PLMN      "
		"IMSI            "
		"RSRP(dBm) "
		"RSRQ(dBm) ");

	move(8, 8);
	for(j = 0; j < iface_col - 16; j++) {
		printw("-");
	}

	/* Draw a list of active UEs! */
	for(i = 0; i < UE_MAX; i++) {
		/* Skip empty elements. */
		if(sim_ues[i].rnti == UE_RNTI_INVALID) {
			continue;
		}

		/* Selected element. */
		if(iface_ue_sel == s) {
			iface_ue_sel_idx = i;
			attron(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));

			move(9 + s, 8);
			for(j = 0; j < iface_col - 16; j++) {
				printw(" ");
			}
		}

		move(9 + s, 8);
		sprintf(tmp, "%d", sim_ues[i].rnti);
		printw("%-10s", tmp);

		sprintf(tmp, "%x", sim_ues[i].plmn);
		printw("%-10s", tmp);

		sprintf(tmp, "%"PRIu64"", sim_ues[i].imsi);
		printw("%-16s", tmp);

		if(sim_ues[i].meas[0].rs.rsrp < UE_RSRP_ERR_LIMIT) {
			if(iface_ue_sel == s) {
				attron(COLOR_PAIR(IFACE_CPAIR_ERROR_H));
			} else {
				attron(COLOR_PAIR(IFACE_CPAIR_ERROR));
			}
		} else if (sim_ues[i].meas[0].rs.rsrp < UE_RSRP_WARN_LIMIT) {
			if(iface_ue_sel == s) {
				attron(COLOR_PAIR(IFACE_CPAIR_WARNING_H));
			} else {
				attron(COLOR_PAIR(IFACE_CPAIR_WARNING));
			}
		} else {
			if(iface_ue_sel == s) {
				attron(COLOR_PAIR(IFACE_CPAIR_OK_H));
			} else {
				attron(COLOR_PAIR(IFACE_CPAIR_OK));
			}
		}

		sprintf(tmp, "%.2f", sim_ues[i].meas[0].rs.rsrp);
		printw("%-10s", tmp);

		if(sim_ues[i].meas[0].rs.rsrq < UE_RSRQ_ERR_LIMIT) {
			if(iface_ue_sel == s) {
				attron(COLOR_PAIR(IFACE_CPAIR_ERROR_H));
			} else {
				attron(COLOR_PAIR(IFACE_CPAIR_ERROR));
			}
		} else if (sim_ues[i].meas[0].rs.rsrq < UE_RSRQ_WARN_LIMIT) {
			if(iface_ue_sel == s) {
				attron(COLOR_PAIR(IFACE_CPAIR_WARNING_H));
			} else {
				attron(COLOR_PAIR(IFACE_CPAIR_WARNING));
			}
		} else {
			if(iface_ue_sel == s) {
				attron(COLOR_PAIR(IFACE_CPAIR_OK_H));
			} else {
				attron(COLOR_PAIR(IFACE_CPAIR_OK));
			}
		}

		sprintf(tmp, "%.2f", sim_ues[i].meas[0].rs.rsrq);
		printw("%-10s", tmp);

		if(sim_ues[i].meas[0].rs.rsrq < UE_RSRQ_ERR_LIMIT) {
			if(iface_ue_sel == s) {
				attroff(COLOR_PAIR(IFACE_CPAIR_ERROR_H));
			} else {
				attroff(COLOR_PAIR(IFACE_CPAIR_ERROR));
			}
		} else if (sim_ues[i].meas[0].rs.rsrq < UE_RSRQ_WARN_LIMIT) {
			if(iface_ue_sel == s) {
				attroff(COLOR_PAIR(IFACE_CPAIR_WARNING_H));
			} else {
				attroff(COLOR_PAIR(IFACE_CPAIR_WARNING));
			}
		} else {
			if(iface_ue_sel == s) {
				attroff(COLOR_PAIR(IFACE_CPAIR_OK_H));
			} else {
				attroff(COLOR_PAIR(IFACE_CPAIR_OK));
			}
		}

		if(iface_ue_sel == s) {
			attron(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));
		}

		if(iface_ue_sel == s) {
			attroff(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));
		}

		s++;
	}

	/* Draw detailed UE add mask. */
	if(iface_ue_add_mask) {
		iface_ue_draw_add();
	}

	return SUCCESS;
}

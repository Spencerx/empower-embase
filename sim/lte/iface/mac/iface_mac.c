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
#include "../../plmn.h"
#include "../iface_priv.h"

/******************************************************************************
 * Main MAC drawing area.                                                     *
 ******************************************************************************/

int iface_mac_handle_input(int key)
{
	switch(key) {
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case 'r':
		sim_mac.ran = !sim_mac.ran;
		
		if (sim_mac.ran) {
			ran_bootstrap();
		}

		break;
	case '+':
		if (sim_mac.stu != 2000) {
			sim_mac.stu += 100;
		}
		break;
	case '-':
		if (sim_mac.stu != 100) {
			sim_mac.stu -= 100;
		}
		break;
	}

	return SUCCESS;
}

/* Main menu put on the left of the main screen. */
int iface_mac_draw_topbar()
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
	printw("MAC screen, perform operations on MAC layer:");

	if (!sim_mac.ran) {
		move(1, 8);
		printw("r - Enable RAN");
	} else {
		move(1, 8);
		printw("r - Disable RAN");
	}
	
	move(1, 25);
	printw("+ - Speed up MAC");

	move(2, 25);
	printw("- - Speed down MAC");

	attroff(COLOR_PAIR(1));

	return SUCCESS;
}

int iface_mac_clean()
{
	return SUCCESS;
}

int iface_mac_draw()
{
	int  i;
	int  j;

	char th[] = "Current status of the MAC DL PRBs";

	iface_mac_draw_topbar();

	move(5, (iface_col / 2) - (sizeof(th) / 2));
	printw("%s", th);

	/* Show 10 subframes view (one Frame) */
	for (i = 0; i < 10; i++) {
		move(7, (iface_col / 2) - 50 + (i * 10));
		printw("SF %d", i);

		for (j = 0; j < MAC_DL_PRBG_MAX; j++) {
			move(9 + (j * 2), (iface_col / 2) - 51 + (i * 10));

			if (i == sim_mac.DL.tti % 10) {
				attron(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));
			}

			printw(" %05d ", sim_mac.DL.PRBG[i][j].rnti);		

			if (i == sim_mac.DL.tti % 10) {
				attroff(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));
			}
		}
	}

	move(7, iface_col - 14);
	printw("TTI: %05d", sim_mac.DL.tti);

	move(9, iface_col - 14);
	printw("STU: %4d ms", sim_mac.stu);

	return SUCCESS;
}

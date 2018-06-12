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
 * Empower Agent simulator PHY module.
 */

#include "../emsim.h"

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

em_phy sim_phy = {0};

/******************************************************************************
 * Public accessible procedures:                                              *
 ******************************************************************************/

u32 phy_init()
{
	int i;

	/* Initialize cells with invalid ids */
	for(i = 0; i < PHY_CELL_MAX; i++) {
		sim_phy.cells[i].pci = PHY_PCI_INVALID;
	}
#if 0
	/* Only one cell by default */
	sim_phy.nof_cells          = 1;

	sim_phy.cells[0].pci       = 1;
	sim_phy.cells[0].DL_earfcn = 1750;
	sim_phy.cells[0].UL_earfcn = sim_phy.cells[0].DL_earfcn + 18000;
	sim_phy.cells[0].DL_prb    = 25;
	sim_phy.cells[0].UL_prb    = 25;
#endif
	return SUCCESS;
}

/******************************************************************************
 * PHY simulation logic:                                                      *
 ******************************************************************************/

u32 phy_compute()
{
	return SUCCESS;
}

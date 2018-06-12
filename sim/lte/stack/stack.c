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
 * Empower Agent LTE stack simulation entry.
 */

#include "../emsim.h"

#include "stack_priv.h"

#define LOG_STACK(x, ...)	LOG_TRACE(x, ##__VA_ARGS__)

/******************************************************************************
 * Stack simulation logic:                                                    *
 ******************************************************************************/

u32 stack_add_cell(u16 pci, u32 dl_earfcn, u32 ul_earfcn, u8 dl_prb, u8 ul_prb)
{
	int i;

	for(i = 0; i < PHY_CELL_MAX; i++) {
		if(sim_phy.cells[i].pci == PHY_PCI_INVALID) {
			break;
		}
	}

	if(i >= PHY_CELL_MAX) {
		LOG_STACK("No more cell slots available!\n");
		return ERR_STK_ADD_CELL_NOSLOTS;
	}

	/*
	 * Changes which affects the PHY layer:
	 */

	sim_phy.cells[i].pci       = pci;
	sim_phy.cells[i].DL_earfcn = dl_earfcn;
	sim_phy.cells[i].UL_earfcn = ul_earfcn;
	sim_phy.cells[i].DL_prb    = dl_prb;
	sim_phy.cells[i].UL_prb    = ul_prb;

	LOG_STACK("Cell added to eNB: "
		"PCI=%d, "
		"DL.freq=%d, "
		"UL.freq=%d, "
		"DL.PRBs=%d, "
		"UL.PRBs=%d\n",
		sim_phy.cells[i].pci,
		sim_phy.cells[i].DL_earfcn,
		sim_phy.cells[i].UL_earfcn,
		sim_phy.cells[i].DL_prb,
		sim_phy.cells[i].UL_prb);

	sim_phy.nof_cells++;

	/*
	 * Changes which affects the MAC layer:
	 */

	sim_mac.DL.prb_max += dl_prb;
	sim_mac.UL.prb_max += ul_prb;

	return SUCCESS;
}

u32 stack_compute()
{
	int err = phy_compute();

	if(err) {
		return err;
	}

	err = mac_compute();

	if(err) {
		return err;
	}

	return SUCCESS;
}

u32 stack_init()
{
	int err = phy_init();

	if(err) {
		return err;
	}

	err = mac_init();

	if(err) {
		return err;
	}

	return SUCCESS;
}

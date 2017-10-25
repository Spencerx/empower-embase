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
 * Empower Agent simulator wrapper around the eNB agent.
 */

#include <inttypes.h>

#include <emage/emproto.h>

#include "emsim.h"

#define LOG_WRAP(x, ...)        LOG_TRACE(x, ##__VA_ARGS__)

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

s32 sim_UE_rep_max         = 32;
u32 sim_UE_rep_mod         = 0;
s32 sim_UE_rep_trigger     = 0;

s32 sim_cell_stats_trigger = 0;
u32 sim_cell_stat_mod      = 0;

char * sim_ue_buf;

/******************************************************************************
 * Callback implementation.                                                   *
 ******************************************************************************/

int wrap_init()
{
	/*
	 * Add here your custom interaction mechanism with the LTE stack.
	 */

	return 0;
}

int wrap_release()
{
	/*
	 * Add here your custom interaction mechanism with the LTE stack.
	 */

	return 0;
}

int wrap_cell_setup_request(uint32_t mod, uint16_t cell_id)
{
	int         i;
	char        buf[SMALL_BUF] = {0};
	ep_cell_det cell;
	int         blen;

	LOG_WRAP("Cell setup request received!\n");

	for(i = 0; i < sim_phy.nof_cells; i++) {
		if(cell_id == sim_phy.cells[i].pci) {
			break;
		}
	}

	/* Cell not found */
	if(i == PHY_CELL_MAX) {
		blen = epf_single_ecap_rep_fail(
			buf, SMALL_BUF, sim_ID, cell_id, mod);

		if(blen < 0) {
			LOG_WRAP("Cannot format cell setup reply!\n");
			return -1;
		}

		return em_send(sim_ID, buf, blen);
	}

	cell.cap       = EP_CCAP_NOTHING;
	cell.pci       = sim_phy.cells[i].pci;
	cell.DL_earfcn = sim_phy.cells[i].DL_earfcn;
	cell.UL_earfcn = sim_phy.cells[i].UL_earfcn;
	cell.DL_prbs   = (uint8_t)sim_phy.cells[i].DL_prb;
	cell.UL_prbs   = (uint8_t)sim_phy.cells[i].UL_prb;

	blen = epf_single_ccap_rep(
		buf,
		SMALL_BUF,
		sim_ID,
		sim_phy.cells[i].pci,
		mod,
		&cell);

	if(blen < 0) {
		LOG_WRAP("Cannot format cell setup reply!\n");
		return -1;
	}

	em_send(sim_ID, buf, blen);

	return 0;
}


int wrap_enb_setup_request()
{
	int         i;
	char        buf[SMALL_BUF] = {0};
	ep_cell_det cells[PHY_CELL_MAX];
	int         blen;

	LOG_WRAP("eNB setup request received!\n");

	for(i = 0; i < sim_phy.nof_cells; i++) {
		cells[i].cap       = EP_CCAP_NOTHING;
		cells[i].pci       = sim_phy.cells[i].pci;
		cells[i].DL_earfcn = sim_phy.cells[i].DL_earfcn;
		cells[i].UL_earfcn = sim_phy.cells[i].UL_earfcn;
		cells[i].DL_prbs   = (uint8_t)sim_phy.cells[i].DL_prb;
		cells[i].UL_prbs   = (uint8_t)sim_phy.cells[i].UL_prb;
	}

	blen = epf_single_ecap_rep(
		buf,
		SMALL_BUF,
		sim_ID,
		0,
		0,
		EP_ECAP_UE_REPORT,
		cells,
		sim_phy.nof_cells);

	if(blen < 0) {
		LOG_WRAP("Cannot format eNB setup reply!\n");
		return -1;
	}

	em_send(sim_ID, buf, blen);

	return 0;
}

int wrap_ue_report(uint32_t mod, int trig_id)
{
	LOG_WRAP("eNB %d UE report (id=%d)\n", sim_ID, trig_id);

	sim_UE_rep_trigger = trig_id;
	sim_UE_rep_mod     = mod;
	sim_ue_dirty       = 1;

	return 0;
}

int wrap_ue_measure(
	uint32_t     mod,
	int          trig_id,
	uint8_t      measure_id,
	uint16_t     rnti,
	uint16_t     earfcn,
	uint16_t     interval,
	int16_t      max_cells,
	int16_t      max_meas)
{
	int  i;
	int  j;
	char buf[SMALL_BUF] = {0};
	int  blen;

	LOG_WRAP("Controller module %d requested UE %x measure on freq %d\n",
		mod, rnti, earfcn);

	for(i = 0; i < UE_MAX; i++) {
		if(sim_ues[i].rnti == rnti) {
			break;
		}
	}

	/* UE not found */
	if(i == UE_MAX) {
		LOG_WRAP("UE %x not found\n", rnti);

		blen = epf_trigger_uemeas_rep_fail(
			buf, SMALL_BUF, sim_ID, 0, mod);

		if(blen < 0) {
			LOG_WRAP("Cannot format UE measure reply!\n");
			return 0;
		}

		em_send(sim_ID, buf, blen);

		return 0;
	}

	for(j = 0; j < UE_RRCM_MAX; j++) {
		if(sim_ues[i].meas[j].tri_id == 0) {
			break;
		}
	}

	if(j == UE_RRCM_MAX) {
		LOG_WRAP("Too many active measurements for RNTI %x\n", rnti);

		blen = epf_trigger_uemeas_rep_fail(
			buf, SMALL_BUF, sim_ID, 0, mod);

		if(blen < 0) {
			LOG_WRAP("Cannot format UE measure reply!\n");
			return 0;
		}

		em_send(sim_ID, buf, blen);

		return 0;
	}

	sim_ues[i].meas[j].id       = measure_id;
	sim_ues[i].meas[j].mod_id   = mod;
	sim_ues[i].meas[j].tri_id   = trig_id;
	sim_ues[i].meas[j].earfcn   = earfcn;
	sim_ues[i].meas[j].interval = interval;
	/* For the first measurement */
	sim_ues[i].meas[j].dirty    = 1;

	sim_ues[i].meas[j].rs.rsrp  = PHY_RSRP_LOWER;
	sim_ues[i].meas[j].rs.rsrq  = PHY_RSRQ_LOWER;

	return 0;
}

/* Operations offered by this technology abstraction module. */
struct em_agent_ops sim_ops = {
	.init                   = wrap_init,
	.release                = wrap_release,
	.cell_setup_request     = wrap_cell_setup_request,
	.enb_setup_request      = wrap_enb_setup_request,
	.ue_report              = wrap_ue_report,
	.ue_measure             = wrap_ue_measure,
};

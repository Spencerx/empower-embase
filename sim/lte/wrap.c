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
#include <string.h>

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

int wrap_disconnected()
{
	int i;
	int j;

	LOG_WRAP("Controller disconnected!\n");
	LOG_WRAP("    Cleaning UE reporting\n");

	sim_UE_rep_trigger = 0;
	sim_UE_rep_mod     = 0;

	/*
	 * Clean sessions IDs for UE and measurements
	 */

	LOG_WRAP("    Cleaning UE measurement reporting\n");

	for(i = 0; i < UE_MAX; i++) {
		for(j = 0; j < UE_RRCM_MAX; j++) {
			sim_ues[i].meas[j].id       = 0;
			sim_ues[i].meas[j].mod_id   = 0;
			sim_ues[i].meas[j].tri_id   = 0;
		}
	}

	/*
	 * Clean sessions IDs for MAC reports
	 */

	LOG_WRAP("    Cleaning MAC reporting\n");

	for(i = 0; i < MAC_REPORT_MAX; i++) {
		sim_mac.mac_rep[i].mod = 0;
	}

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

int wrap_handover(
	uint32_t mod,
	uint16_t source_cell,
	uint16_t rnti,
	uint32_t target_enb,
	uint16_t target_cell,
	uint8_t  cause)
{
	char        buf[SMALL_BUF] = {0};
	int         blen;

	LOG_WRAP("UE handover requested for RNTI %x\n", rnti);

	if(x2_hand_over(rnti, target_enb)) {
		LOG_WRAP("Failed to hand RNTI %x over\n", rnti);

		blen = epf_single_ho_rep_fail(
			buf,
			SMALL_BUF,
			sim_ID,
			source_cell,
			mod,
			sim_ID,
			source_cell,
			rnti,
			0);

		return -1;
	} else {
		blen = epf_single_ho_rep(
			buf,
			SMALL_BUF,
			sim_ID,
			source_cell,
			mod,
			sim_ID,
			source_cell,
			rnti,
			0);

		ue_rem(rnti);
	}

	if(blen < 0) {
		return 0;
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
	int  k;
	char buf[SMALL_BUF] = {0};
	int  blen;

	LOG_WRAP("Controller module %d requested UE %d measure %d on freq %d\n",
		mod, rnti, measure_id, earfcn);

	for(i = 0; i < UE_MAX; i++) {
		if(sim_ues[i].rnti == rnti) {
			break;
		}
	}

	/* UE not found */
	if(i == UE_MAX) {
		LOG_WRAP("UE %d not found\n", rnti);

		blen = epf_trigger_uemeas_rep_fail(
			buf, SMALL_BUF, sim_ID, 0, mod);

		if(blen < 0) {
			LOG_WRAP("Cannot format UE measure reply!\n");
			return -1;
		}

		em_send(sim_ID, buf, blen);

		return -1;
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
			return -1;
		}

		em_send(sim_ID, buf, blen);

		return -1;
	}

	/* Measure was already active inside UE internals? */
	for(k = 0; k < UE_RRCM_MAX; k++) {
		if(sim_ues[i].meas[k].earfcn == earfcn) {
			/* Update meaningful fields */
			sim_ues[i].meas[k].id       = measure_id;
			sim_ues[i].meas[k].mod_id   = mod;
			sim_ues[i].meas[k].tri_id   = trig_id;
			/* Send an update of such measure */
			sim_ues[i].meas[k].dirty    = 1;

			return 0;
		}
	}

	sim_ues[i].meas[j].id       = measure_id;
	sim_ues[i].meas[j].mod_id   = mod;
	sim_ues[i].meas[j].tri_id   = trig_id;
	/* NOTE:
	 * Does the UE support such earfcn?
	 * Add support for bands.
	 */
	sim_ues[i].meas[j].earfcn   = earfcn;
	sim_ues[i].meas[j].interval = interval;

	if(sim_ues[i].meas[j].rs.rsrp == 0) {
		sim_ues[i].meas[j].rs.rsrp  = PHY_RSRP_LOWER + 10.0;
	}

	if(sim_ues[i].meas[j].rs.rsrq == 0) {
		sim_ues[i].meas[j].rs.rsrq  = PHY_RSRQ_LOWER +  5.0;
	}

	return 0;
}

int wrap_mac_report(uint32_t mod, int32_t interval, int trig_id)
{
	int  i;
	int  m = -1;

	char buf[SMALL_BUF] = {0};
	int  blen;

	LOG_WRAP("Controller module %d requested a MAC report\n", mod);

	for(i = 0; i < MAC_REPORT_MAX; i++) {
		if(sim_mac.mac_rep[i].mod == 0) {
			m = i;
		}

		/* Report already there */
		if(sim_mac.mac_rep[i].mod == mod) {
			sim_mac.mac_rep[i].interval = interval;
			return 0;
		}
	}

	if(m == -1) {
		blen = epf_trigger_macrep_rep_fail(
			buf, SMALL_BUF, sim_ID, 0, mod);

		em_send(sim_ID, buf, blen);

		return -1;
	}

	sim_mac.mac_rep[m].interval = interval;
	sim_mac.mac_rep[m].mod      = mod;

	return 0;
}

/* Handles a RAN setup request */
int wrap_ran_setup(uint32_t mod)
{
	char buf[SMALL_BUF];
	int  blen;

	ep_ran_det rd;

	LOG_WRAP("Controller module %d requested a RAN setup\n", mod);
    
	/* Negative reply if RAN mechanism is offline */
	if (!sim_mac.ran) {
		LOG_WRAP("RAN subsystem disabled; notifying...\n");

		blen = epf_single_ran_setup_ns(
			buf, 
			SMALL_BUF, 
			sim_ID, 
			sim_phy.cells[0].pci, 
			mod);

		if (blen < 0) {
			return 0;
		}

		return em_send(sim_ID, buf, blen);
	}

	rd.tenant_sched = sim_ran.sched_id;

	blen = epf_single_ran_setup_rep(
		buf, 
		SMALL_BUF,
		sim_ID,
		sim_phy.cells[0].pci,
		mod,
		&rd);

	if (blen < 0) {
		return blen;
	}
    
	return em_send(sim_ID, buf, blen);
}

/* Handles the RAN request for an user status */
int wrap_ran_user(uint32_t mod, uint16_t rnti) 
{
	int i;

	char buf[SMALL_BUF];
	int  blen;

	uint32_t        nofu = 0;
	ep_ran_user_det ud[RAN_USER_MAX]   = { 0 };

	LOG_WRAP("Controller module %d requested status of RAN user %d\n",
		mod, rnti);

	/* Negative reply if RAN mechanism is offline */
	if (!sim_mac.ran) {
		LOG_WRAP("RAN subsystem disabled; notifying...\n");

		blen = epf_single_ran_user_ns(
			buf,
			SMALL_BUF,
			sim_ID,
			sim_phy.cells[0].pci,
			mod);

		if (blen < 0) {
			return 0;
		}

		return em_send(sim_ID, buf, blen);
	}

	/* Need feedback from a single user */
	if(rnti != 0) {
		for (i = 0; i < RAN_USER_MAX; i++) {
			if (sim_ran.users[i].rnti == rnti) {
				ud[0].id     = sim_ran.users[i].rnti;
				ud[0].tenant = sim_ran.users[i].tenant[0];
				nofu         = 1;
				break;
			}
		}
	} 
	/* Need feedback from all the users */
	else {
		for (i = 0; i < RAN_USER_MAX; i++) {
			if (sim_ran.users[i].rnti != RAN_USER_INVALID_ID) {
				ud[nofu].id     = sim_ran.users[i].rnti;
				ud[nofu].tenant = sim_ran.users[i].tenant[0];
				nofu++;
			}
		}
	}

	/* User(s) NOT found */
	if (rnti != 0 && nofu == 0) {
		LOG_WRAP("RAN user %d not found in RAN subsystem!\n", rnti);

		blen = epf_single_ran_user_fail(
			buf,
			SMALL_BUF,
			sim_ID,
			sim_phy.cells[0].pci,
			mod);

		if (blen < 0) {
			return 0;
		}

		return em_send(sim_ID, buf, blen);
	}

	blen = epf_single_ran_usr_rep(
		buf,
		SMALL_BUF,
		sim_ID,
		sim_phy.cells[0].pci,
		mod,
		nofu,
		ud);

	if (blen < 0) {
		return 0;
	}

	return em_send(sim_ID, buf, blen);
}

int wrap_ran_add_user(uint32_t mod, uint16_t rnti, uint64_t tenant)
{
	char buf[SMALL_BUF];
	int  blen;

	ep_ran_user_det ud;

	LOG_WRAP("Controller module %d requested adding %d ---> %ld mapping\n",
		mod, rnti, tenant);

	/* Negative reply if RAN mechanism is offline */
	if (!sim_mac.ran) {
		LOG_WRAP("RAN subsystem disabled; notifying...\n");

		blen = epf_single_ran_user_ns(
			buf,
			SMALL_BUF,
			sim_ID,
			sim_phy.cells[0].pci,
			mod);

		if (blen < 0) {
			return 0;
		}

		return em_send(sim_ID, buf, blen);
	}

	/* Operation was not successfull... */
	if (ran_add_user(rnti, tenant) != SUCCESS) {
		LOG_WRAP("RAN cannot map %d --> %ld\n", rnti, tenant);

		blen = epf_single_ran_user_fail(
			buf,
			SMALL_BUF,
			sim_ID,
			sim_phy.cells[0].pci,
			mod);

		if (blen < 0) {
			return 0;
		}

		return em_send(sim_ID, buf, blen);
	}

	/* Success reported by sending back info on the new mapping */

	ud.id     = rnti;
	ud.tenant = tenant;

	blen = epf_single_ran_usr_rep(
		buf,
		SMALL_BUF,
		sim_ID,
		sim_phy.cells[0].pci,
		mod,
		1,
		&ud);

	if (blen < 0) {
		return 0;
	}

	return em_send(sim_ID, buf, blen);
}

int wrap_ran_rem_user(uint32_t mod, uint16_t rnti)
{
	char buf[SMALL_BUF];
	int  blen;

	ep_ran_user_det ud;

	LOG_WRAP("Controller module %d remove user %d\n", mod, rnti);

	/* Negative reply if RAN mechanism is offline */
	if (!sim_mac.ran) {
		LOG_WRAP("RAN subsystem disabled; notifying...\n");

		blen = epf_single_ran_user_ns(
			buf,
			SMALL_BUF,
			sim_ID,
			sim_phy.cells[0].pci,
			mod);

		if (blen < 0) {
			return 0;
		}

		return em_send(sim_ID, buf, blen);
	}

	if (ran_rem_user(rnti, 0) != SUCCESS) {
		LOG_WRAP("RAN cannot remove user %d\n", rnti);

		blen = epf_single_ran_user_fail(
			buf,
			SMALL_BUF,
			sim_ID,
			sim_phy.cells[0].pci,
			mod);

		if (blen < 0) {
			return 0;
		}

		return em_send(sim_ID, buf, blen);
	}

	/* Success reported by sending back info on the new mapping  */

	ud.id     = rnti;
	ud.tenant = 0;    /* For zero here we express no association */

	blen = epf_single_ran_usr_rep(
		buf,
		SMALL_BUF,
		sim_ID,
		sim_phy.cells[0].pci,
		mod,
		1,
		&ud);

	if (blen < 0) {
		return 0;
	}

	return em_send(sim_ID, buf, blen);
}

int wrap_ran_ten(uint32_t mod, uint64_t tenant)
{
	int  i;

	char buf[SMALL_BUF];
	int  blen;

	int  noft = 0;

	ep_ran_tenant_det td[RAN_TENANT_MAX] = {0};

	LOG_WRAP("Controller module %d requested status of tenant %ld\n",
		mod, tenant);

	/* Negative reply if RAN mechanism is offline */
	if (!sim_mac.ran) {
		LOG_WRAP("RAN subsystem disabled; notifying...\n");

		blen = epf_single_ran_tenant_ns(
			buf,
			SMALL_BUF,
			sim_ID,
			sim_phy.cells[0].pci,
			mod);

		if (blen < 0) {
			return 0;
		}

		return em_send(sim_ID, buf, blen);
	}

	/* Tenant at zero means all the tenants... */
	if (!tenant) {
		for (i = 0; i < RAN_TENANT_MAX; i++) {
			if (sim_ran.tenants[i].id != RAN_TENANT_INVALID_ID) {
				td[noft].id    = sim_ran.tenants[i].id;
				td[noft].sched = sim_ran.tenants[i].sched_id;
				noft++;
			}
		}
	}
	/* ...otherwise we are looking for a particular tenant */
	else {
		for (i = 0; i < RAN_TENANT_MAX; i++) {
			if (sim_ran.tenants[i].id != tenant) {
				td[noft].id    = sim_ran.tenants[i].id;
				td[noft].sched = sim_ran.tenants[i].sched_id;
				noft++;
				break;
			}
		}
	}

	blen = epf_single_ran_ten_rep(
		buf, 
		SMALL_BUF, 
		sim_ID, 
		sim_phy.cells[0].pci,
		mod,
		noft,
		td);

	if (blen < 0) {
		return blen;
	}

	em_send(sim_ID, buf, blen);

	return 0;
}

int wrap_ran_add_ten(uint32_t mod, uint64_t tenant, uint32_t sched)
{
	char buf[SMALL_BUF];
	int  blen;

	ep_ran_tenant_det td = { 0 };

	LOG_WRAP("Controller module %d requested to add tenant %ld\n",
		mod, tenant);

	/* Negative reply if RAN mechanism is offline */
	if (!sim_mac.ran) {
		LOG_WRAP("RAN subsystem disabled; notifying...\n");

		blen = epf_single_ran_tenant_ns(
			buf,
			SMALL_BUF,
			sim_ID,
			sim_phy.cells[0].pci,
			mod);

		if (blen < 0) {
			return 0;
		}

		return em_send(sim_ID, buf, blen);
	}

	/* Not accepting tenant IDs 0 or 1 (reserved ones) */
	if (tenant == 0 || tenant == 1) {
		LOG_WRAP("Cannot add Tenant ID %ld; it's reserved\n", tenant);

		blen = epf_single_ran_tenant_fail(
			buf,
			SMALL_BUF,
			sim_ID,
			sim_phy.cells[0].pci,
			mod);

		if (blen < 0) {
			return blen;
		}

		return em_send(sim_ID, buf, blen);
	}

	/* Try to add the tenant */
	if (ran_add_tenant(tenant, sched) != SUCCESS) {
		LOG_WRAP("Failed to add new tenant\n");

		blen = epf_single_ran_tenant_fail(
			buf,
			SMALL_BUF,
			sim_ID,
			sim_phy.cells[0].pci,
			mod);

		if (blen < 0) {
			return blen;
		}

		return em_send(sim_ID, buf, blen);
	}

	td.id    = tenant;
	td.sched = sched;

	blen = epf_single_ran_ten_rep(
		buf,
		SMALL_BUF,
		sim_ID,
		sim_phy.cells[0].pci,
		mod,
		1,
		&td);

	if (blen < 0) {
		return blen;
	}

	return em_send(sim_ID, buf, blen);
}

int wrap_ran_rem_ten(uint32_t mod, uint64_t tenant)
{
	char buf[SMALL_BUF];
	int  blen;

	ep_ran_tenant_det td = { 0 };

	LOG_WRAP("Controller module %d requested to remove tenant %ld\n",
		mod, tenant);

	/* Negative reply if RAN mechanism is offline */
	if (!sim_mac.ran) {
		LOG_WRAP("RAN subsystem disabled; notifying...\n");

		blen = epf_single_ran_tenant_ns(
			buf,
			SMALL_BUF,
			sim_ID,
			sim_phy.cells[0].pci,
			mod);

		if (blen < 0) {
			return 0;
		}

		return em_send(sim_ID, buf, blen);
	}

	/* Not accepting tenant IDs 0 or 1 (reserved ones) */
	if (tenant == 0 || tenant == 1) {
		LOG_WRAP("Cannot remove Tenant ID %ld; it's reserved\n", 
			tenant);

		blen = epf_single_ran_tenant_fail(
			buf,
			SMALL_BUF,
			sim_ID,
			sim_phy.cells[0].pci,
			mod);

		if (blen < 0) {
			return blen;
		}

		return em_send(sim_ID, buf, blen);
	}

	/* Try to remove the tenant */
	if (ran_rem_tenant(tenant) != SUCCESS) {
		LOG_WRAP("Failed to remove tenant\n");

		blen = epf_single_ran_tenant_fail(
			buf,
			SMALL_BUF,
			sim_ID,
			sim_phy.cells[0].pci,
			mod);

		if (blen < 0) {
			return blen;
		}

		return em_send(sim_ID, buf, blen);
	}

	td.id    = tenant;
	td.sched = 0; /* Zero identify an invalid tenant */

	blen = epf_single_ran_ten_rep(
		buf,
		SMALL_BUF,
		sim_ID,
		sim_phy.cells[0].pci,
		mod,
		1,
		&td);

	if (blen < 0) {
		return blen;
	}

	return em_send(sim_ID, buf, blen);
}

int wrap_ran_get_param(
	uint32_t            mod,
	uint32_t            id,
	uint8_t             type,
	uint64_t            tenant,
	ep_ran_sparam_det * param)
{
	char buf[MEDIUM_BUF];
	int  blen;

	int  tmsize;
	char tm[MEDIUM_BUF];

	ep_ran_sparam_det p;

	LOG_WRAP("Controller module %d requested a parameter\n", mod);

	/* Negative reply if RAN mechanism is offline */
	if (!sim_mac.ran) {
		LOG_WRAP("RAN subsystem disabled; notifying...\n");

		blen = epf_single_ran_schedule_ns(
			buf,
			SMALL_BUF,
			sim_ID,
			sim_phy.cells[0].pci,
			mod);

		if (blen < 0) {
			return 0;
		}

		return em_send(sim_ID, buf, blen);
	}

	/* Trying to tune schedulers different than '1' is not implemented */
	if (id != 1) {
		LOG_WRAP("RAN Scheduler %d not supported...\n", id);

		blen = epf_single_ran_schedule_ns(
			buf,
			SMALL_BUF,
			sim_ID,
			sim_phy.cells[0].pci,
			mod);

		if (blen < 0) {
			return 0;
		}

		return em_send(sim_ID, buf, blen);
	}

	/* It's a top-level, tenants scheduler */
	if (type == EP_RAN_SCHED_TENANT_TYPE) {
		/* Requesting to know the state of TTI window of scheduler 1*/
		if (strncmp(param->name, "tti_window", param->name_len) == 0) {
			p.name      = "tti_window";
			p.name_len  = sizeof("tti_window");
			p.value     = "10";
			p.value_len = sizeof("10");

			blen = epf_single_ran_sch_rep(
				buf,
				MEDIUM_BUF,
				sim_ID,
				sim_phy.cells[0].pci,
				mod,
				1,
				0,
				&p);

			if (blen < 0) {
				return 0;
			}

			return em_send(sim_ID, buf, blen);
		}

		/* Requesting to know the state of the tenants map */
		if (strncmp(param->name, "tenant_map", param->name_len) == 0) {
			tmsize = ran_format_tenant_map(tm, MEDIUM_BUF);

			p.name      = "tenant_map";
			p.name_len  = sizeof("tenant_map");
			p.value     = tm;
			p.value_len = tmsize;

			blen = epf_single_ran_sch_rep(
				buf,
				MEDIUM_BUF,
				sim_ID,
				sim_phy.cells[0].pci,
				mod,
				1,
				0,
				&p);

			if (blen < 0) {
				return 0;
			}

			return em_send(sim_ID, buf, blen);
		}
	}
	/* It's an user scheduler which belongs to a tenant */
	else {
		LOG_WRAP("You are not supposed to tune sched 1 of tenant %ld\n",
			tenant);

		blen = epf_single_ran_schedule_ns(
			buf,
			MEDIUM_BUF,
			sim_ID,
			sim_phy.cells[0].pci,
			mod);

		if (blen < 0) {
			return 0;
		}

		return em_send(sim_ID, buf, blen);
	}

	return 0;
}

int wrap_ran_set_param(
	uint32_t            mod,
	uint32_t            id,
	uint8_t             type,
	uint64_t            tenant,
	ep_ran_sparam_det * param)
{
	char buf[MEDIUM_BUF];
	int  blen;

	LOG_WRAP("Setting parameters is not supported yet\n");

	blen = epf_single_ran_schedule_ns(
		buf,
		SMALL_BUF,
		sim_ID,
		sim_phy.cells[0].pci,
		mod);

	if (blen < 0) {
		return 0;
	}

	return em_send(sim_ID, buf, blen);
}

/* Operations offered by this technology abstraction module. */
struct em_agent_ops sim_ops = {
	.init                    = wrap_init,
	.release                 = wrap_release,
	.disconnected            = wrap_disconnected,
	.cell_setup_request      = wrap_cell_setup_request,
	.enb_setup_request       = wrap_enb_setup_request,
	.handover_UE             = wrap_handover,
	.ue_report               = wrap_ue_report,
	.ue_measure              = wrap_ue_measure,
	.mac_report              = wrap_mac_report,
	.ran_setup_request       = wrap_ran_setup,
	.ran_user_request        = wrap_ran_user,
	.ran_user_add            = wrap_ran_add_user,
	.ran_user_rem            = wrap_ran_rem_user,
	.ran_tenant_request      = wrap_ran_ten,
	.ran_tenant_add          = wrap_ran_add_ten,
	.ran_tenant_rem          = wrap_ran_rem_ten,
	.ran_sched_get_parameter = wrap_ran_get_param,
	.ran_sched_set_parameter = wrap_ran_set_param,
};

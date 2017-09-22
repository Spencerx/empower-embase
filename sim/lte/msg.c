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
 * Empower Agent simulator protocol parser module.
 */

#include <inttypes.h>

#include <stdlib.h>
#include <string.h>

#include <emage/emage.h>
#include <emage/emproto.h>

#include "emsim.h"

#define LOG_MSG(x, ...) 		LOG_TRACE(x, ##__VA_ARGS__)

/******************************************************************************
 * Private procedures:                                                        *
 ******************************************************************************/
#if 0
/* Free a single RbsAllocPerCell message. */
void msg_free_rbsapc(RbsAllocPerCell * r)
{
	int i;
	int j;

	if(!r->sf) {
		goto out;
	}

	for(i = 0; i < r->n_sf; i++) {
		if(!r->sf[i]) {
			continue;
		}

		if(r->sf[i]->rbs_alloc) {
			for(j = 0; j < r->sf[i]->n_rbs_alloc; j++) {
				if(!r->sf[i]->rbs_alloc[j]) {
					continue;
				}

				free(r->sf[i]->rbs_alloc[j]);
			}
		}

		free(r->sf[i]);
	}

	free(r->sf);

out:
	free(r);
}
#endif
/******************************************************************************
 * Public accessible procedures:                                              *
 ******************************************************************************/

int msg_fill_ue_details(ep_ue_details * ues)
{
	int i;
	int j;

	for(i = 0, j = 0; i < UE_MAX; i++) {
		if(sim_ues[i].rnti == UE_RNTI_INVALID) {
			continue;
		}

		if(j >= EP_UE_REPORT_MAX_UES) {
			break;
		}

		ues[i].pci  = sim_phy.pci;
		ues[i].rnti = sim_ues[j].rnti;
		ues[i].plmn = sim_ues[j].plmn;
		ues[i].imsi = sim_ues[j].imsi;

		j++;
	}

	return j;
}

#if 0
int msg_cell_stats(u32 agent_id, u32 mod_id, EmageMsg ** ret)
{
	EmageMsg * msg            = malloc(sizeof(EmageMsg));
	Header * hdr              = malloc(sizeof(Header));
	TriggerEvent * te         = malloc(sizeof(TriggerEvent));
	CellStats * cs            = malloc(sizeof(CellStats));
	CellStatsRepl * cr        = malloc(sizeof(CellStatsRepl));
	CellPrbUtilization * prb  = malloc(sizeof(CellPrbUtilization));
	DlCellPrbUtilization * dl = malloc(sizeof(DlCellPrbUtilization));
	UlCellPrbUtilization * ul = malloc(sizeof(UlCellPrbUtilization));

	if(!msg || !hdr || !te || !cs || !cr || !prb || !dl || !ul) {
		goto err;
	}

	memset(msg, 0, sizeof(EmageMsg));
	memset(hdr, 0, sizeof(Header));
	memset(te,  0, sizeof(TriggerEvent));
	memset(cs,  0, sizeof(CellStats));
	memset(cr,  0, sizeof(CellStatsRepl));
	memset(prb, 0, sizeof(CellPrbUtilization));
	memset(dl,  0, sizeof(DlCellPrbUtilization));
	memset(ul,  0, sizeof(UlCellPrbUtilization));

	emage_msg__init(msg);
	header__init(hdr);
	trigger_event__init(te);
	cell_stats__init(cs);
	cell_stats_repl__init(cr);
	cell_prb_utilization__init(prb);
	dl_cell_prb_utilization__init(dl);
	ul_cell_prb_utilization__init(ul);

	ul->has_num_prbs  = 1;
	ul->has_perc_prbs = 1;
	ul->num_prbs      = sim_phy.UL_used;
	ul->perc_prbs     =
		((float)sim_phy.UL_used / (float)sim_phy.UL_prb) * 100.0;

	dl->has_num_prbs  = 1;
	dl->has_perc_prbs = 1;
	dl->num_prbs      = sim_phy.DL_used;
	dl->perc_prbs     =
		((float)sim_phy.DL_used / (float)sim_phy.DL_prb) * 100.0;

	prb->dl_prb_utilz = dl;
	prb->ul_prb_utilz = ul;

	cr->prb_utilz = prb;
	cr->status = STATS_REQ_STATUS__SREQS_SUCCESS;

	cs->cell_stats_m_case = CELL_STATS__CELL_STATS_M_REPL;
	cs->repl = cr;

	te->events_case = TRIGGER_EVENT__EVENTS_M_CELL_STATS;
	te->mcell_stats = cs;

	hdr->b_id = agent_id;
	hdr->vers = 1;
	hdr->seq  = 0;
	hdr->t_id = mod_id;

	msg->event_types_case = EMAGE_MSG__EVENT_TYPES_TE;
	msg->te = te;
	msg->head = hdr;

	*ret = msg;

	LOG_MSG("Cell Statistic message allocated.\n");

	return SUCCESS;

err:
	if(cr) {
		free(cr);
	}

	if(cs) {
		free(cs);
	}

	if(te) {
		free(te);
	}

	if(hdr) {
		free(hdr);
	}

	if(msg) {
		free(msg);
	}

	LOG_MSG("Failed to wrap up a Cell Statistic message.\n");

	return ERR_MSG_CS_MEM;
}

int msg_RRC_meas(u32 agent_id, u32 UE_idx, u32 meas_idx, EmageMsg ** ret)
{
	EmageMsg *               msg = malloc(sizeof(EmageMsg));
	Header *                 hdr = malloc(sizeof(Header));
	TriggerEvent *           te  = malloc(sizeof(TriggerEvent));
	RrcMeas *                rm  = malloc(sizeof(RrcMeas));
	RrcMeasRepl *            rmr = malloc(sizeof(RrcMeasRepl));
	NeighCellsMeasurements * nc  = malloc(sizeof(NeighCellsMeasurements));

	int i;

	if(!msg || !hdr || !te || !rm || !rmr || !nc) {
		goto err;
	}

	emage_msg__init(msg);
	header__init(hdr);
	trigger_event__init(te);
	rrc_meas__init(rm);
	rrc_meas_repl__init(rmr);
	neigh_cells_measurements__init(nc);

	nc->n_eutra_meas = sim_nof_neigh;
	nc->eutra_meas = malloc(sizeof(EUTRAMeasurements *) * nc->n_eutra_meas);

	if(!nc->eutra_meas) {
		goto err;
	}

	memset(
		nc->eutra_meas,
		0,
		sizeof(EUTRAMeasurements *) * nc->n_eutra_meas);

	for(i = 0; i < NEIGH_MAX; i++) {
		/* Skip non-active neighbor cells. */
		if(!sim_neighs[i].id) {
			continue;
		}

		nc->eutra_meas[i] = malloc(sizeof(EUTRAMeasurements));

		if(!nc->eutra_meas[i]) {
			goto err;
		}

		eutra_measurements__init(nc->eutra_meas[i]);

		nc->eutra_meas[i]->has_phys_cell_id = 1;
		nc->eutra_meas[i]->phys_cell_id = sim_neighs[i].pci;
		nc->eutra_meas[i]->cgi_meas = 0;

		nc->eutra_meas[i]->meas_result =
			malloc(sizeof(EUTRARefSignalMeas));

		if(!nc->eutra_meas[i]->meas_result) {
			goto err;
		}

		eutra_ref_signal_meas__init(nc->eutra_meas[i]->meas_result);

		nc->eutra_meas[i]->meas_result->has_rsrp = 1;
		nc->eutra_meas[i]->meas_result->rsrp =
			sim_neighs[i].rs[UE_idx].rsrp;
		nc->eutra_meas[i]->meas_result->has_rsrq = 1;
		nc->eutra_meas[i]->meas_result->rsrq =
			sim_neighs[i].rs[UE_idx].rsrq;
	}

	rmr->rnti           = sim_ues[UE_idx].rnti;
	rmr->neigh_meas     = nc;
	rmr->status         = STATS_REQ_STATUS__SREQS_SUCCESS;
	rmr->has_measid     = 1;
	rmr->measid         = sim_ues[UE_idx].meas[meas_idx].id;
	rmr->has_freq       = 1;
	rmr->freq           = sim_ues[UE_idx].meas[meas_idx].earfcn;
	rmr->has_pcell_rsrp = 1;
	rmr->pcell_rsrp     = sim_ues[UE_idx].meas[meas_idx].rs.rsrp;
	rmr->has_pcell_rsrq = 1;
	rmr->pcell_rsrq     = sim_ues[UE_idx].meas[meas_idx].rs.rsrq;

	rm->rrc_meas_m_case = RRC_MEAS__RRC_MEAS_M_REPL;
	rm->repl = rmr;

	te->events_case = TRIGGER_EVENT__EVENTS_M_RRC_MEAS;
	te->mrrc_meas = rm;

	hdr->b_id = agent_id;
	hdr->vers = 1;
	hdr->seq  = 0;
	hdr->t_id = sim_ues[UE_idx].meas[meas_idx].mod_id;

	msg->event_types_case = EMAGE_MSG__EVENT_TYPES_TE;
	msg->te = te;
	msg->head = hdr;

	*ret = msg;

	LOG_MSG("RRC measurement for %d message allocated.\n",
		sim_ues[UE_idx].rnti);

	return SUCCESS;

err:
	if(nc) {
		if(nc->eutra_meas) {
			for(i = 0; i < nc->n_eutra_meas; i++) {
				if(nc->eutra_meas[i]) {
					if(nc->eutra_meas[i]->meas_result) {
						free(nc->eutra_meas[i]->
							meas_result);
					}

					free(nc->eutra_meas[i]);
				}
			}

			free(nc->eutra_meas);
		}

		free(nc);
	}

	if(rmr) {
		free(rmr);
	}

	if(rm) {
		free(rm);
	}

	if(te) {
		free(te);
	}

	if(hdr) {
		free(hdr);
	}

	if(msg) {
		free(msg);
	}

	LOG_MSG("Failed to wrap up a RRC Measurement message.\n");

	return ERR_MSG_RM_MEM;
}

int msg_RRC_meas_conf(u32 agent_id, u32 mod_id, u32 UE_idx, EmageMsg ** ret) {
	EmageMsg * msg         = malloc(sizeof(EmageMsg));
	Header * hdr           = malloc(sizeof(Header));
	TriggerEvent * te      = malloc(sizeof(TriggerEvent));
	UeRrcMeasConf * mc     = malloc(sizeof(UeRrcMeasConf));
	UeRrcMeasConfRepl * cr = malloc(sizeof(UeRrcMeasConfRepl));

	int i; /* Generic purpose index. */
	int j; /* Generic purpose index. */

	if(!msg || !hdr || !te || !mc || !cr) {
		goto err;
	}

	emage_msg__init(msg);
	header__init(hdr);
	trigger_event__init(te);
	ue_rrc_meas_conf__init(mc);
	ue_rrc_meas_conf_repl__init(cr);

	cr->capabilities = malloc(sizeof(UeCapabilities));

	if(!cr->capabilities) {
		goto err;
	}

	memset(cr->capabilities, 0, sizeof(UeCapabilities));

	/*
	 * Bands on which the UE is operating:
	 */

	for(i = 0; i < UE_BAND_MAX; i++) {
		if(sim_ues[UE_idx].bands[i] != 0) {
			j++;
		}
	}

	cr->capabilities->n_band = j;
	cr->capabilities->band   = malloc(sizeof(uint32_t) * j);

	if(!cr->capabilities->band) {
		goto err;
	}

	memset(cr->capabilities->band, 0, sizeof(uint32_t) * j);

	for(i = 0, j = 0; i < UE_BAND_MAX; i++) {
		if(sim_ues[UE_idx].bands[i] != 0) {
			cr->capabilities->band[j] = sim_ues[UE_idx].bands[i];
			j++;
		}
	}

	/*
	 * Measurements objects:
	 */

	/* Clean 'j' in order to contains the number of meas id active. */
	for(i = 0, j = 0; i < UE_BAND_MAX; i++) {
		if(sim_ues[UE_idx].meas[i].id != UE_RRCM_INVALID) {
			j++;
		}
	}

	cr->n_m_obj = j;
	cr->m_obj = malloc(sizeof(MeasObject *) * j);

	if(!cr->m_obj) {
		goto err;
	}

	memset(cr->m_obj, 0, sizeof(MeasObject *) * j);

	for(i = 0, j = 0; i < UE_RRCM_MAX; i++) {
		if(sim_ues[UE_idx].meas[i].id != 0) {
			cr->m_obj[j] = malloc(sizeof(MeasObject));

			if(!cr->m_obj[j]) {
				goto err;
			}

			memset(cr->m_obj[j], 0, sizeof(MeasObject));

			cr->m_obj[j]->measobjid = sim_ues[UE_idx].meas[i].id;
			cr->m_obj[j]->meas_obj_case =
				MEAS_OBJECT__MEAS_OBJ_MEAS_OBJ__EUTRA;

			cr->m_obj[j]->measobj_eutra =
				malloc(sizeof(MeasObjEUTRA));

			if(!cr->m_obj[j]->measobj_eutra) {
				goto err;
			}

			memset(
				cr->m_obj[j]->measobj_eutra,
				0,
				sizeof(MeasObjEUTRA));

			cr->m_obj[j]->measobj_eutra->has_carrier_freq = 1;
			cr->m_obj[j]->measobj_eutra->carrier_freq =
				sim_ues[UE_idx].meas[i].earfcn;

			cr->m_obj[j]->measobj_eutra->has_meas_bw = 1;
			cr->m_obj[j]->measobj_eutra->meas_bw =
				sim_ues[UE_idx].meas[i].bw;

			/* This complexity is getting ridiculous...
			 * Let's stop here.
			 */
			cr->m_obj[j]->measobj_eutra->n_bkl_cells = 0;
			cr->m_obj[j]->measobj_eutra->n_cells = 0;

			j++;
		}
	}

	mc->ue_rrc_meas_conf_m_case = UE_RRC_MEAS_CONF__UE_RRC_MEAS_CONF_M_REPL;
	mc->repl = cr;

	te->events_case = TRIGGER_EVENT__EVENTS_M_UE_RRC_MEAS_CONF;
	te->mue_rrc_meas_conf = mc;

	msg->head->b_id = agent_id;
	msg->head->t_id = mod_id;
	msg->head->seq  = 0;
	msg->head->vers = 1;

	msg->event_types_case = EMAGE_MSG__EVENT_TYPES_TE;
	msg->te = te;

	*ret = msg;

	LOG_MSG("Failed to wrap up a RRC Measurement Configuration "
		"message.\n");

	return SUCCESS;

err:
	if(cr) {
		if(cr->n_m_obj > 0) {
			for(i = 0; i < cr->n_m_obj; i++) {
				if(cr->m_obj[i]) {
					if(cr->m_obj[i]->measobj_eutra) {
						free(cr->m_obj[i]->measobj_eutra);
					}

					free(cr->m_obj[i]);
				}
			}

			free(cr->m_obj);
		}

		if(cr->capabilities) {
			if(cr->capabilities->band) {
				free(cr->capabilities->band);
			}

			free(cr->capabilities);
		}

		free(cr);
	}

	if(mc) {
		free(mc);
	}

	if(te) {
		free(te);
	}

	if(hdr) {
		free(hdr);
	}

	if(msg) {
		free(msg);
	}

	LOG_MSG("Failed to wrap up a RRC Measurement Configuration "
		"message.\n");

	return ERR_MSG_RMC_MEM;
}

int msg_RRC_meas_fail(u32 agent_id, u32 mod_id, EmageMsg ** ret)
{
	EmageMsg * msg    = malloc(sizeof(EmageMsg));
	Header * hdr      = malloc(sizeof(Header));
	TriggerEvent * te = malloc(sizeof(TriggerEvent));
	RrcMeas * rm      = malloc(sizeof(RrcMeas));
	RrcMeasRepl * rmr = malloc(sizeof(RrcMeasRepl));

	if(!msg || !hdr || !te || !rm || !rmr) {
		goto err;
	}

	emage_msg__init(msg);
	header__init(hdr);
	trigger_event__init(te);
	rrc_meas__init(rm);
	rrc_meas_repl__init(rmr);

	rmr->status = STATS_REQ_STATUS__SREQS_FAILURE;

	rm->rrc_meas_m_case = RRC_MEAS__RRC_MEAS_M_REPL;
	rm->repl = rmr;

	te->events_case = TRIGGER_EVENT__EVENTS_M_RRC_MEAS;
	te->mrrc_meas = rm;

	hdr->b_id = agent_id;
	hdr->vers = 1;
	hdr->seq  = 0;
	hdr->t_id = mod_id;

	msg->event_types_case = EMAGE_MSG__EVENT_TYPES_TE;
	msg->te = te;
	msg->head = hdr;

	*ret = msg;

	LOG_MSG("RRC Measurement Configuration failure message allocated.\n");

	return SUCCESS;

err:

	if(rmr) {
		free(rmr);
	}

	if(rm) {
		free(rm);
	}

	if(te) {
		free(te);
	}

	if(hdr) {
		free(hdr);
	}

	if(msg) {
		free(msg);
	}

	LOG_MSG("Failed to wrap up a RRC Measurement Configuration failure "
		"message.\n");

	return ERR_MSG_RMF_MEM;
}

int msg_UE_report(u32 agent_id, u32 mod_id, EmageMsg ** ret)
{
	EmageMsg * msg    = 0;
	Header * hdr      = 0;
	TriggerEvent * te = 0;
	UesId * uid       = 0;
	UesIdRepl * urpl  = 0;

	int i = 0;
	int u = 0;

	char * plmn = 0;
	char * imsi = 0;

	msg  = malloc(sizeof(EmageMsg));
	hdr  = malloc(sizeof(Header));
	te   = malloc(sizeof(TriggerEvent));
	uid  = malloc(sizeof(UesId));
	urpl = malloc(sizeof(UesIdRepl));

	if(!msg || !hdr || !te || !uid || !urpl) {
		goto err;
	}

	memset(msg,  0, sizeof(EmageMsg));
	memset(hdr,  0, sizeof(Header));
	memset(te,   0, sizeof(TriggerEvent));
	memset(uid,  0, sizeof(UesId));
	memset(urpl, 0, sizeof(UesIdRepl));

	emage_msg__init(msg);
	header__init(hdr);
	trigger_event__init(te);
	ues_id__init(uid);
	ues_id_repl__init(urpl);

	urpl->n_active_ue_id = sim_nof_ues;

	if(urpl->n_active_ue_id > 0) {
		urpl->active_ue_id = malloc(
			sizeof(ActiveUe *) * urpl->n_active_ue_id);

		if(!urpl->active_ue_id) {
			goto err;
		}

		memset(
			urpl->active_ue_id,
			0,
			sizeof(ActiveUe *) * urpl->n_active_ue_id);

		for(i = 0; i < UE_MAX; i++) {
			/* Skip the non-active UEs. */
			if(sim_ues[i].rnti == 0) {
				continue;
			}

			urpl->active_ue_id[u] = malloc(sizeof(ActiveUe));

			if(!urpl->active_ue_id[u]) {
				goto err;
			}

			plmn = malloc(sizeof(char) * 64);
			imsi = malloc(sizeof(char) * 64);

			if(!plmn || !imsi) {
				goto err;
			}

			sprintf(plmn, "%06d", sim_ues[i].plmn);
			sprintf(imsi, "%"PRIu64"", sim_ues[i].imsi);

			active_ue__init(urpl->active_ue_id[u]);

			urpl->active_ue_id[u]->rnti    = sim_ues[i].rnti;
			urpl->active_ue_id[u]->plmn_id = plmn;
			urpl->active_ue_id[u]->imsi    = imsi;

			u++;
		}
	} else {
		urpl->active_ue_id = 0;
	}

	urpl->n_inactive_ue_id = 0;
	urpl->inactive_ue_id = 0;

	urpl->status = CONF_REQ_STATUS__CREQS_SUCCESS;

	uid->ues_id_m_case = UES_ID__UES_ID_M_REPL;
	uid->repl = urpl;

	te->events_case = TRIGGER_EVENT__EVENTS_M_UES_ID;
	te->mues_id = uid;

	hdr->b_id = agent_id;
	hdr->vers = 1;
	hdr->seq  = 0;
	hdr->t_id = mod_id;

	msg->event_types_case = EMAGE_MSG__EVENT_TYPES_TE;
	msg->te = te;
	msg->head = hdr;

	*ret = msg;

	LOG_MSG("UEs report message allocated.\n");

	return SUCCESS;

err:
	if(plmn) {
		free(plmn);
	}

	if(imsi) {
		free(imsi);
	}

	if(urpl) {
		if(urpl->active_ue_id) {
			for(i = 0; i < urpl->n_active_ue_id; i++) {
				if(urpl->active_ue_id[i]) {
					if(urpl->active_ue_id[i]->imsi) {
						free(urpl->active_ue_id[i]->
							imsi);
					}

					if(urpl->active_ue_id[i]->plmn_id) {
						free(urpl->active_ue_id[i]->
							plmn_id);
					}

					free(urpl->active_ue_id[i]);
				}
			}

			free(urpl->active_ue_id);
		}

		free(urpl);
	}

	if(uid) {
		free(uid);
	}

	if(te) {
		free(te);
	}

	if(hdr) {
		free(hdr);
	}

	if(msg) {
		free(msg);
	}

	LOG_MSG("Failed to wrap up a UEs Report failure message.\n");

	return ERR_MSG_UER_MEM;
}

int msg_cell_report(u32 agent_id, u32 mod_id, EmageMsg ** ret)
{
	EmageMsg * msg    = 0;
	Header * hdr      = 0;
	SingleEvent * se  = 0;
	ENBCells * ec     = 0;
	ENBCellsRepl * er = 0;

	msg = malloc(sizeof(EmageMsg));
	hdr = malloc(sizeof(Header));
	se  = malloc(sizeof(SingleEvent));
	ec  = malloc(sizeof(ENBCells));
	er  = malloc(sizeof(ENBCellsRepl));

	if(!msg || !hdr || !se || !ec || !er) {
		goto err;
	}

	memset(msg, 0, sizeof(EmageMsg));
	memset(hdr, 0, sizeof(Header));
	memset(se,  0, sizeof(SingleEvent));
	memset(ec,  0, sizeof(ENBCells));
	memset(er,  0, sizeof(ENBCellsRepl));

	emage_msg__init(msg);
	header__init(hdr);
	single_event__init(se);
	e_nb_cells__init(ec);
	e_nb_cells_repl__init(er);

	er->n_cells = 1;
	er->cells = malloc(sizeof(CellInformation *) * er->n_cells);

	if(!er->cells) {
		goto err;
	}

	er->cells[0] = malloc(sizeof(CellInformation));

	if(!er->cells[0]) {
		goto err;
	}

	cell_information__init(er->cells[0]);

	er->cells[0]->phys_cell_id   = sim_phy.pci;
	er->cells[0]->carrier_freq   = sim_phy.earfcn;
	er->cells[0]->has_num_rbs_dl = 1;
	er->cells[0]->num_rbs_dl     = sim_phy.DL_prb;
	er->cells[0]->has_num_rbs_ul = 1;
	er->cells[0]->num_rbs_ul     = sim_phy.UL_prb;

	ec->e_nb_cells_m_case = E_NB_CELLS__E_NB_CELLS_M_REPL;
	ec->repl = er;

	se->events_case = SINGLE_EVENT__EVENTS_M_ENB_CELLS;
	se->menb_cells = ec;

	hdr->b_id = agent_id;
	hdr->vers = 1;
	hdr->seq  = 0;
	hdr->t_id = mod_id;

	msg->event_types_case = EMAGE_MSG__EVENT_TYPES_SE;
	msg->head = hdr;
	msg->se   = se;

	*ret = msg;

	LOG_MSG("Cell Report message allocated.\n");

	return SUCCESS;

err:
	if(er) {
		if(er->cells) {
			if(er->cells[0]) {
				free(er->cells[0]);
			}

			free(er->cells);
		}

		free(er);
	}

	if(ec) {
		free(ec);
	}

	if(se) {
		free(se);
	}

	if(hdr) {
		free(hdr);
	}

	if(msg) {
		free(msg);
	}

	LOG_MSG("Failed to wrap up a Cell Report message.\n");

	return ERR_MSG_CR_MEM;
}

int msg_cell_report_fail(u32 agent_id, u32 mod_id, EmageMsg ** ret)
{
	EmageMsg * msg    = 0;
	Header * hdr      = 0;
	SingleEvent * se  = 0;
	ENBCells * ec     = 0;
	ENBCellsRepl * er = 0;

	msg = malloc(sizeof(EmageMsg));
	hdr = malloc(sizeof(Header));
	se  = malloc(sizeof(SingleEvent));
	ec  = malloc(sizeof(ENBCells));
	er  = malloc(sizeof(ENBCellsRepl));

	if(!msg || !hdr || !se || !ec || !er) {
		goto err;
	}

	memset(msg, 0, sizeof(EmageMsg));
	memset(hdr, 0, sizeof(Header));
	memset(se,  0, sizeof(SingleEvent));
	memset(ec,  0, sizeof(ENBCells));
	memset(er,  0, sizeof(ENBCellsRepl));

	emage_msg__init(msg);
	header__init(hdr);
	single_event__init(se);
	e_nb_cells__init(ec);
	e_nb_cells_repl__init(er);

	er->status = CONF_REQ_STATUS__CREQS_FAILURE;

	ec->e_nb_cells_m_case = E_NB_CELLS__E_NB_CELLS_M_REPL;
	ec->repl = er;

	se->events_case = SINGLE_EVENT__EVENTS_M_ENB_CELLS;
	se->menb_cells = ec;

	hdr->b_id = agent_id;
	hdr->vers = 1;
	hdr->seq  = 0;
	hdr->t_id = mod_id;

	msg->event_types_case = EMAGE_MSG__EVENT_TYPES_SE;
	msg->head = hdr;
	msg->se   = se;

	*ret = msg;

	LOG_MSG("Cell Statistic failure message allocated.\n");

	return SUCCESS;

err:
	if(er) {
		free(er);
	}

	if(ec) {
		free(ec);
	}

	if(se) {
		free(se);
	}

	if(hdr) {
		free(hdr);
	}

	if(msg) {
		free(msg);
	}

	LOG_MSG("Failed to wrap up a Cell Statistic Failure message.\n");

	return ERR_MSG_CRF_MEM;
}

int msg_RAN_report(u32 agent_id, u32 mod_id, EmageMsg ** ret)
{
	int i;
	int j;

	u32 * DL;
	u32 * UL;

	EmageMsg * msg         = 0;
	Header * hdr           = 0;
	SingleEvent * se       = 0;
	ENBCells * ec          = 0;
	ENBCellsRepl * er      = 0;
	ENBRanSharingInfo * rs = 0;

	msg = malloc(sizeof(EmageMsg));
	hdr = malloc(sizeof(Header));
	se  = malloc(sizeof(SingleEvent));
	ec  = malloc(sizeof(ENBCells));
	er  = malloc(sizeof(ENBCellsRepl));
	rs  = malloc(sizeof(ENBRanSharingInfo));

	if(!msg || !hdr || !se || !ec || !er || !rs) {
		goto err;
	}

	memset(msg, 0, sizeof(EmageMsg));
	memset(hdr, 0, sizeof(Header));
	memset(se,  0, sizeof(SingleEvent));
	memset(ec,  0, sizeof(ENBCells));
	memset(er,  0, sizeof(ENBCellsRepl));
	memset(rs,  0, sizeof(ENBRanSharingInfo));

	emage_msg__init(msg);
	header__init(hdr);
	single_event__init(se);
	e_nb_cells__init(ec);
	e_nb_cells_repl__init(er);
	e_nb_ran_sharing_info__init(rs);

	/*
	 * PRB in the DL:
	 */

	rs->n_rbs_alloc_dl = 1;
	rs->rbs_alloc_dl = malloc(
		sizeof(RbsAllocPerCell *) * rs->n_rbs_alloc_dl);

	if(!rs->rbs_alloc_dl) {
		goto err;
	}

	rs->rbs_alloc_dl[0] = malloc(sizeof(RbsAllocPerCell));

	if(!rs->rbs_alloc_dl[0]) {
		goto err;
	}

	rbs_alloc_per_cell__init(rs->rbs_alloc_dl[0]);

	rs->rbs_alloc_dl[0]->n_sf = sim_ran.DL_win;
	rs->rbs_alloc_dl[0]->sf = malloc(
		sizeof(RbsAllocOverSf *) * rs->rbs_alloc_dl[0]->n_sf);

	if(!rs->rbs_alloc_dl[0]->sf) {
		goto err;
	}

	/* 'i' enumerates the sub-frame number. */
	for(i = 0; i < rs->rbs_alloc_dl[0]->n_sf; i++) {
		rs->rbs_alloc_dl[0]->sf[i] = malloc(sizeof(RbsAllocOverSf));

		if(!rs->rbs_alloc_dl[0]->sf[i]) {
			goto err;
		}

		rbs_alloc_over_sf__init(rs->rbs_alloc_dl[0]->sf[i]);

		rs->rbs_alloc_dl[0]->sf[i]->n_rbs_alloc = sim_phy.DL_prb;
		rs->rbs_alloc_dl[0]->sf[i]->rbs_alloc = malloc(
			sizeof(char *) * sim_phy.DL_prb);

		/* 'j' enumerates the PRB number. */
		for(j = 0; j < sim_phy.DL_prb; j++) {
			/* ASCII string allocation. */
			rs->rbs_alloc_dl[0]->sf[i]->rbs_alloc[j] =
				malloc(sizeof(char) * 7);

			if(!rs->rbs_alloc_dl[0]->sf[i]->rbs_alloc[j]) {
				goto err;
			}

			DL = sim_ran.DL + ((i * sim_phy.DL_prb) + j);

			/* Format the newly allocated string properly. */
			sprintf(rs->rbs_alloc_dl[0]->sf[i]->rbs_alloc[j],
				"%06d",
				*DL);
		}

	}

	/*
	 * PRB in the UL:
	 */

	rs->n_rbs_alloc_ul = 1;
	rs->rbs_alloc_ul = malloc(
		sizeof(RbsAllocPerCell *) * rs->n_rbs_alloc_ul);

	if(!rs->rbs_alloc_ul) {
		goto err;
	}

	rs->rbs_alloc_ul[0] = malloc(sizeof(RbsAllocPerCell));

	if(!rs->rbs_alloc_ul[0]) {
		goto err;
	}

	rbs_alloc_per_cell__init(rs->rbs_alloc_ul[0]);

	rs->rbs_alloc_ul[0]->n_sf = sim_ran.DL_win;
	rs->rbs_alloc_ul[0]->sf = malloc(
		sizeof(RbsAllocOverSf *) * rs->rbs_alloc_ul[0]->n_sf);

	if(!rs->rbs_alloc_ul[0]->sf) {
		goto err;
	}

	/* 'i' enumerates the sub-frame number. */
	for(i = 0; i < rs->rbs_alloc_ul[0]->n_sf; i++) {
		rs->rbs_alloc_ul[0]->sf[i] = malloc(sizeof(RbsAllocOverSf));

		if(!rs->rbs_alloc_ul[0]->sf[i]) {
			goto err;
		}

		rbs_alloc_over_sf__init(rs->rbs_alloc_ul[0]->sf[i]);

		rs->rbs_alloc_ul[0]->sf[i]->n_rbs_alloc = sim_phy.UL_prb;
		rs->rbs_alloc_ul[0]->sf[i]->rbs_alloc = malloc(
			sizeof(char *) * sim_phy.UL_prb);

		/* 'j' enumerates the PRB number. */
		for(j = 0; j < sim_phy.UL_prb; j++) {
			/* ASCII string allocation. */
			rs->rbs_alloc_ul[0]->sf[i]->rbs_alloc[j] =
				malloc(sizeof(char) * 7);

			if(!rs->rbs_alloc_ul[0]->sf[i]->rbs_alloc[j]) {
				goto err;
			}

			UL = sim_ran.UL + ((i * sim_phy.UL_prb) + j);

			/* Format the newly allocated string properly. */
			sprintf(rs->rbs_alloc_ul[0]->sf[i]->rbs_alloc[j],
				"%06d",
				*UL);
		}

	}

	/*
	 * Tenant scheduler:
	 */

	rs->n_tenant_schedulers_dl = sim_ran.nof_ts;

	/* Skip on 0. */
	if(!rs->n_tenant_schedulers_dl) {
		goto ue;
	}

	rs->tenant_schedulers_dl = malloc(
		sizeof(char *) * rs->n_tenant_schedulers_dl);

	if(!rs->tenant_schedulers_dl) {
		goto err;
	}

	for(i = 0; i < RAN_MAX_SCHEDULERS; i++) {
		/* Empty string... go on. */
		if(sim_ran.t_sched[i][0] == 0) {
			continue;
		}

		rs->tenant_schedulers_dl[i] = malloc(
			sizeof(char) * strlen(sim_ran.t_sched[i]));

		if(!rs->tenant_schedulers_dl[i]) {
			goto err;
		}

		/* Copy that algorithm name. */
		strcpy(rs->tenant_schedulers_dl[i], sim_ran.t_sched[i]);
	}

	/*
	 * UE scheduler:
	 */
ue:
	rs->n_ue_schedulers_dl = sim_ran.nof_ts;

	/* Skip on 0. */
	if(!rs->n_ue_schedulers_dl) {
		goto cells;
	}

	rs->ue_schedulers_dl =
		malloc(sizeof(char *) * rs->n_ue_schedulers_dl);

	if(!rs->ue_schedulers_dl) {
		goto err;
	}

	for(i = 0; i < RAN_MAX_SCHEDULERS; i++) {
		/* Empty string... go on. */
		if(sim_ran.u_sched[i][0] == 0) {
			continue;
		}

		rs->ue_schedulers_dl[i] = malloc(
			sizeof(char) * strlen(sim_ran.u_sched[i]));

		if(!rs->ue_schedulers_dl[i]) {
			goto err;
		}

		/* Copy that algorithm name. */
		strcpy(rs->ue_schedulers_dl[i], sim_ran.u_sched[i]);
	}

	/*
	 * Cells:
	 */
cells:
	rs->n_cell = 1;
	rs->cell   = malloc(sizeof(CellRanSharingInfo *) * rs->n_cell);

	if(!rs->cell) {
		goto err;
	}

	rs->cell[0] = malloc(sizeof(CellRanSharingInfo));

	if(!rs->cell[0]) {
		goto err;
	}

	cell_ran_sharing_info__init(rs->cell[0]);

	rs->cell[0]->phys_cell_id = sim_phy.pci;
	rs->cell[0]->avail_rbs_dl = sim_phy.DL_prb;
	rs->cell[0]->avail_rbs_ul = sim_phy.UL_prb;

	rs->sched_window_dl = sim_ran.DL_win;
	rs->sched_window_ul = sim_ran.UL_win;

	er->ran_sh_i = rs;
	er->status = CONF_REQ_STATUS__CREQS_SUCCESS;

	ec->e_nb_cells_m_case = E_NB_CELLS__E_NB_CELLS_M_REPL;
	ec->repl = er;

	se->events_case = SINGLE_EVENT__EVENTS_M_ENB_CELLS;
	se->menb_cells = ec;

	hdr->b_id = agent_id;
	hdr->vers = 1;
	hdr->seq  = 0;
	hdr->t_id = mod_id;

	msg->event_types_case = EMAGE_MSG__EVENT_TYPES_SE;
	msg->head = hdr;
	msg->se   = se;

	*ret = msg;

	LOG_MSG("Cell Statistic failure message allocated.\n");
	return SUCCESS;

err:
	if(rs) {
		if(rs->rbs_alloc_dl) {
			for(i = 0; i < rs->n_rbs_alloc_dl; i++) {
				if(rs->rbs_alloc_dl[i]) {
					/* Free this guy... */
					msg_free_rbsapc(rs->rbs_alloc_dl[i]);
				}
			}

			free(rs->rbs_alloc_dl);
		}

		if(rs->tenant_schedulers_dl) {
			for(i = 0; i < rs->n_tenant_schedulers_dl; i++) {
				if(rs->tenant_schedulers_dl[i]) {
					free(rs->tenant_schedulers_dl[i]);
				}
			}

			free(rs->tenant_schedulers_dl);
		}

		if(rs->ue_schedulers_dl) {
			for(i = 0; i < rs->n_ue_schedulers_dl; i++) {
				if(rs->ue_schedulers_dl[i]) {
					free(rs->ue_schedulers_dl[i]);
				}
			}

			free(rs->ue_schedulers_dl);
		}

		if(rs->cell) {
			for(i = 0; i < rs->n_cell; i++) {
				if(rs->cell[i]) {
					free(rs->cell[i]);
				}
			}

			free(rs->cell);
		}

		free(rs);
	}

	if(er) {
		free(er);
	}

	if(ec) {
		free(ec);
	}

	if(se) {
		free(se);
	}

	if(hdr) {
		free(hdr);
	}

	if(msg) {
		free(msg);
	}

	return ERR_MSG_RRE_MEM;
}
#endif

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
 * Empower Agent simulator ue module.
 */

#ifndef __EM_SIM_UE_H
#define __EM_SIM_UE_H

#include "phy.h"

/******************************************************************************
 * RNTI information.                                                          *
 * Source: 36.321, Table 7.1-1                                                *
 ******************************************************************************/

/* Invalid RNTI value. */
#define UE_RNTI_INVALID			0

/*
 * From 0x1 to 0xfff3 there is the valid range of RNTIs.
 */

/* Start of RNTI reserved area. */
#define UE_RNTI_RESERVED		0xfff4
/* Number of reserved RNTI slots. */
#define UE_RNTI_NOF_RESERVED		8

#define UE_RNTI_M			0xfffd
#define UE_RNTI_P			0xfffe
#define UE_RNTI_SI			0xffff

/* Max number of UE taken in account. */
#define UE_MAX				10

/* Maximum number of supported bands. */
#define UE_BAND_MAX			32
/* Invalid band identifier. */
#define UE_BAND_INVALID			0

/* Maximum number of measurements that can be issued on a UE. */
#define UE_RRCM_MAX			32
/* Invalid identifier for a measurement. */
#define UE_RRCM_INVALID			0

/* RRC measurement issued to an UE to scan a certain frequency. */
typedef struct __em_sim_rrc_measurement {
	/* Id of this particular measurement. */
	u32 id;
	/* Trigger id assigned by the agent subsystem. */
	u32 trigger;
	/* Transaction id assigned by controller. */
	u32 mod_id;
	/* Frequency to scan. */
	u32 earfcn;

	/* PCI detected on such measurement, if any. */
	u16 pci;
	/* Reference signal detected. */
	em_phy_rs rs;

	/* Bandwidth for this measurement. */
	u32 bw;

	/* Modifications occurs on such measurements? */
	u32 dirty;
} em_ue_rrcm;

/* Describes how a neighbor cell is seen by this one. */
typedef struct __em_sim_user_equipment{
	/* Radio Network Temporary Identifier. */
	u16 rnti;

	/* Public Land Mobile Network id. */
	u32 plmn;

	/* International Mobile Subscriber Identity. */
	u64 imsi;

	/* Bands on which the UE can operate on. */
	u32 bands[UE_BAND_MAX];

	/* Measurements issued to an UE. */
	em_ue_rrcm meas[UE_RRCM_MAX];
} em_ue;

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

/* Number of UEs actually active on this cell. */
extern u32 sim_nof_ues;

/* UEs information. */
extern em_ue sim_ues[UE_MAX];

/* Identify if some modifications occurs on the UE list. */
extern u32 sim_ue_dirty;

/******************************************************************************
 * Public accessible procedures:                                              *
 ******************************************************************************/

/* Adds a new UE in the managed ones.
 * Returns the UE slot index on success, otherwise a negative error code.
 */
int ue_add(u16 rnti, u32 plmid, u64 imsi);

/* Simulates the UE attached to this cell. This procedure is part of the
 * simulator computation loops, which updates status and perform custom
 * heuristics.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
u32 ue_compute(void);

/* Removes a managed UE by looking for its RNTI.
 * Returns 0 on success, otherwise a negative error code.
 */
int ue_rem(u16 rnti);

/* Returns a possible candidate for an UE RNTI.
 */
u16 ue_rnti_candidate(void);

#endif /* __EM_SIM_UE_H */

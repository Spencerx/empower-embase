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

#ifndef __EM_SIM_PHY_H
#define __EM_SIM_PHY_H

#include <emtypes.h>

/* 3 kinds of Primary Sync. sequence for 168 of Secondary Sync. sequences. */
#define PHY_PCI_MAX			504

#define PHY_RSRP_LOWER			-140.0
#define PHY_RSRP_HIGHER			-80.0

#define PHY_RSRQ_LOWER			-20.0
#define PHY_RSRQ_HIGHER			-3.0

#define PHY_6PRBS_BW			1.4f
#define PHY_15PRBS_BW			3.0f
#define PHY_25PRBS_BW			5.0f
#define PHY_50PRBS_BW			10.0f
#define PHY_75PRBS_BW			15.0f
#define PHY_100PRBS_BW			20.0f

#define PHY_1_4MHZ_PRBS			6
#define PHY_3MHZ_PRBS			15
#define PHY_5MHZ_PRBS			25
#define PHY_10MHZ_PRBS			50
#define PHY_15MHZ_PRBS			75
#define PHY_20MHZ_PRBS			100

#define PHY_SUBFRAME_X_FRAME		10

/* Provides a description for the Reference Signal. */
typedef struct __em_sim_phy_ref_signal {
	/* Reference Signal Received Power. */
	sp rsrp;
	/* Reference Signal Received Quality. */
	sp rsrq;
} em_phy_rs;

/* Provides information of PHY layer for a single cell. */
typedef struct __em_sim_phy {
	/* Physical Cell Identifier. */
	u16 pci;

	/* Frequency at which the PHY is operating. */
	u32 earfcn;

	/* PRBs in the DL. */
	u32 DL_prb;
	/* PRBs currently in use in the DL. */
	u32 DL_used;
	/* DL PRB organization in the larger case considered. */
	u32 DL[PHY_SUBFRAME_X_FRAME][PHY_20MHZ_PRBS];

	/* PRBs in the UL. */
	u32 UL_prb;
	/* PRBs currently in use in the UL. */
	u32 UL_used;
	/* UL PRB organization in the larger case considered. */
	u32 UL[PHY_SUBFRAME_X_FRAME][PHY_20MHZ_PRBS];
} em_phy;

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

/* PHY layer used. */
extern em_phy sim_phy;

/* Identify if the simulated PHY is dirty. */
extern u32 sim_phy_dirty;

/******************************************************************************
 * Public accessible procedures:                                              *
 ******************************************************************************/

/* Initializes the PHY layer subsystems.
 * Before calling this you will need to set properly the sim_phy global variable
 * values.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
u32 phy_init(void);

/******************************************************************************
 * PHY simulation logic:                                                      *
 ******************************************************************************/

/* Simulates the PHY of this cell.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
u32 phy_compute(void);

#endif

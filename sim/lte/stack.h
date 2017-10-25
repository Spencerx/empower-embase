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
 * Empower Agent simulator LTE stack simulation.
 */

#ifndef __EM_SIM_STACK_H
#define __EM_SIM_STACK_H

#include <time.h>

#include <emtypes.h>

/* 3 kinds of Primary Sync. sequence for 168 of Secondary Sync. sequences. */
#define PHY_PCI_MAX                     504

#define PHY_RSRP_LOWER                 -140.0
#define PHY_RSRP_HIGHER                -80.0

#define PHY_RSRQ_LOWER                 -20.0
#define PHY_RSRQ_HIGHER                -3.0

#define PHY_6PRBS_BW                    1.4f
#define PHY_15PRBS_BW                   3.0f
#define PHY_25PRBS_BW                   5.0f
#define PHY_50PRBS_BW                   10.0f
#define PHY_75PRBS_BW                   15.0f
#define PHY_100PRBS_BW                  20.0f

#define PHY_1_4MHZ_PRBS                 6
#define PHY_3MHZ_PRBS                   15
#define PHY_5MHZ_PRBS                   25
#define PHY_10MHZ_PRBS                  50
#define PHY_15MHZ_PRBS                  75
#define PHY_20MHZ_PRBS                  100

#define PHY_SUBFRAME_X_FRAME            10
#define PHY_CELL_MAX                    8

#define MAC_REPORT_MAX			8

/* Provides a description for the Reference Signal */
typedef struct __em_sim_phy_ref_signal {
	/* Reference Signal Received Power. */
	sp rsrp;
	/* Reference Signal Received Quality. */
	sp rsrq;
} em_phy_rs;

/* Provides the description of a single cell at PHY layer. */
typedef struct __em_sim_phy_cell {
	/* Changes occurred in the cell? */
	u32 dirty;

	/* Physical Cell Identifier. */
	u16 pci;

	/* Frequency at which the cell is operating. */
	u32 DL_earfcn;
	/* Down-link PRBs of this cell. */
	u16 DL_prb;

	/* Frequency at which the cell is operating. */
	u32 UL_earfcn;
	/* Up-link PRBs of this cell. */
	u16 UL_prb;
}em_phy_cell;

/* Provides the description of the PHY layer for the simulator */
typedef struct __em_sim_phy {
	/* Changes occurred in the PHY? */
	u32 dirty;

	/* Number of cells present */
	u8          nof_cells;
	/* Information of each cell. */
	em_phy_cell cells[PHY_CELL_MAX];
} em_phy;

/* Provides the descriptor for the MAC layer reports */
typedef struct __em_sim_mac_report {
	/* Module which requested the measurement */
	u32 mod;

	/* Interval for the statistics in ms */
	u32 interval;

	/* Accumulator for the Downlink */
	u64 DL_acc;
	/* Accumulator for the Uplink*/
	u64 UL_acc;

	/* Last time the interval has been triggered */
	struct timespec last;
}em_mac_rep;

/* Provides the description of the MAC layer for the simulator */
typedef struct __em_sim_mac {
	/* Changes occurred in the MAC? */
	u32 dirty;

	/* Total amount of DL PRBs that can be allocated */
	u16 DL_prb_max;
	/* Total amount of DL PRBs in use */
	u16 DL_prb_in_use;

	/* Total amount of UL PRBs that can be allocated */
	u16 UL_prb_max;
	/* Total amount of UL PRBs in use */
	u16 UL_prb_in_use;

	/* Active reports on the MAC layer */
	em_mac_rep mac_rep[MAC_REPORT_MAX];
} em_mac;

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

/* PHY layer */
extern em_phy sim_phy;
/* MAC layer */
extern em_mac sim_mac;

/******************************************************************************
 * Procedures:                                                                *
 ******************************************************************************/

/* Configures a new cell into the eNB.
 *
 * returns 0 on success, otherwise a negative error code.
 */
u32 stack_add_cell(u16 pci, u32 dl_earfcn, u32 ul_earfcn, u8 dl_prb, u8 ul_prb);

/* Simulates the LTE stack of this cell.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
u32 stack_compute();

/* Initializes the stack elements.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
u32 stack_init();

#endif /* __EM_SIM_STACK_H */

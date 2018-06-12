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
#define PHY_PCI_MAX                     503
#define PHY_PCI_INVALID                 0xffff

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
#define PHY_MAX_FRAMES			1024
#define PHY_MAX_TTI			10240

/*
 * MAC-related data structures:
 */

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

/*
 * MAC-related data structures:
 */

#define MAC_PRB_1_4			6
#define MAC_PRB_3			15
#define MAC_PRB_5			25
#define MAC_PRB_10			50
#define MAC_PRB_15			75
#define MAC_PRB_20			100

#define MAC_DL_RGS_1_4			1
#define MAC_DL_RGS_3			2
#define MAC_DL_RGS_5			2
#define MAC_DL_RGS_10			3
#define MAC_DL_RGS_15			4
#define MAC_DL_RGS_20			4

#define MAC_DL_PRBG_MAX			25

#define MAC_REPORT_MAX			8

/* Provides the descriptor for the MAC layer reports */
typedef struct __em_sim_mac_report {
	/* Module which requested the measurement */
	u32 mod;

	/* Interval for the statistics in ms */
	u32 interval;

	/* Accumulator for the Downlink */
	u32 DL_acc;
	/* Accumulator for the Uplink*/
	u32 UL_acc;

	/* Last time the interval has been triggered */
	struct timespec last;
} em_mac_rep;

/* Organization of the DL in the MAC */
typedef struct __em_sim_mac_PRBG {
	/* Each single Physical Resource Block information here */
	int      PRBG[MAC_DL_RGS_20];
	/* RNTI which owns this group of resources */
	uint16_t rnti;
} em_mac_PRBG;

/* Organization of the DL in the MAC */
typedef struct __em_sim_mac_DL {
	/* Group of Physical Resource Blocks for a frame */
	em_mac_PRBG     PRBG[PHY_SUBFRAME_X_FRAME][MAC_DL_PRBG_MAX];

	/* TTI of the DL */
	int             tti;

	/* Total amount of DL PRBs that can be allocated */
	u8              prb_max;
	/* Total amount of DL PRBs in use */
	u8              prb_in_use;

	/* Last time the DL has been scheduled */
	struct timespec last;
} em_mac_DL;

/* Organization of the UL in the MAC */
typedef struct __em_sim_mac_UL {
	/* Total amount of DL PRBs that can be allocated */
	u16             prb_max;
	/* Total amount of DL PRBs in use */
	u16             prb_in_use;

	/* Last time the DL has been scheduled */
	struct timespec last;
} em_mac_UL;

/* Provides the description of the MAC layer for the simulator */
typedef struct __em_sim_mac {
	/* Changes occurred in the MAC? */
	u32        dirty;

	/* Defines, in 'ms' the unit of time forthe schedulers; tuning this 
	 * variable allows to speed up/down speed of schedulers.
	 */
	u32        stu;

	/* DL part of the MAC scheduler */
	em_mac_DL  DL;

	/* UL part of the MAC scheduler */
	em_mac_UL  UL;

	/* Flag which identifies if RAN sharing is enabled or not */
	int ran;

	/* Active reports on the MAC layer */
	em_mac_rep mac_rep[MAC_REPORT_MAX];
} em_mac;

/*
 * RAN-related data structures:
 */

 #define RAN_USER_MAX		8
 #define RAN_USER_INVALID_ID	0x0

 #define RAN_TENANT_MAX		8
 #define RAN_TENANT_INVALID_ID	0x0
 #define RAN_TENANT_DEFAULT	0x1

/* Description of a RAN UE */
typedef struct __em_sim_ran_UE {
	/* RNTI associated with the UE */
	uint16_t rnti;
	/* Tenant associated with this UE */
	uint64_t tenant[RAN_TENANT_MAX];
} em_ran_user;

/* Description of a RAN tenant */
typedef struct __em_sim_ran_tenant {
	/* ID of this tenant */
	uint64_t  id;

	/* User scheduler associated with the tenant */
	uint32_t  sched_id;
	/* Has the user scheduler being initialized before? */
	int       sched_init;
	/* Private data used by the lower-level scheduler */
	void *    sched_priv;
} em_ran_tenant;

/* Provides a description of the RAN module of the simulator */
typedef struct __em_sim_ran {
	/* Id of the scheduler in charge of manage tenants */
	uint32_t sched_id;

	/* Tenants handled by the RAN module */
	em_ran_tenant tenants[RAN_TENANT_MAX];
	/* Registered users in RAN scheduler */
	em_ran_user   users[RAN_USER_MAX];
} em_ran;

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

/* PHY layer */
extern em_phy sim_phy;
/* MAC layer */
extern em_mac sim_mac;
/* RAN module of the MAC layer */
extern em_ran sim_ran;

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

/*
 * RAN Sharing procedures:
 */

/* Perform RAN initialization procedures.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
u32 ran_bootstrap();

/* Adds a new RAN user->tenant association.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
u32 ran_add_user(u16 user, u64 tenant);

/* Removes a RAN user->tenant association.
 * If 'tenant' is specified to be 0, the entire user is removed.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
u32 ran_rem_user(u16 user, u64 tenant);

/* Adds a new Tenant inside the RAN subsystem.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
u32 ran_add_tenant(u64 tenant, u32 sched);

/* Removes an existing Tenant from the RAN subsystem.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
u32 ran_rem_tenant(u64 tenant);

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * WARNING: This part is temporary and can be removed in future.
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

/* Format a given buffer with the tenant map.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
u32 ran_format_tenant_map(char * buf, int len);

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * End of WARNING part.
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

#endif /* __EM_SIM_STACK_H */

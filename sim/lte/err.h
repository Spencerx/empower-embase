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
 * Empower Agent simulator errors.
 */

#ifndef __EM_SIM_ERR_H
#define __EM_SIM_ERR_H

#include <emtypes.h>

#define ERR_MAX_ERRORS	4096

/*
 * Here the enumerated list of errors that can occurs within the simulator.
 */
enum __em_sim_errors {
	ERR_UNKNOWN = -ERR_MAX_ERRORS,

	/*
	 * LOG errors:
	 */

	/* I/O error occurred during the initialization of the log mechanism. */
	ERR_LOG_INIT_IO,

	/*
	 * MSG errors:
	 */

	/* No more memory in cell statistic message creation. */
	ERR_MSG_CS_MEM,
	/* No more memory in RRC measurements message creation. */
	ERR_MSG_RM_MEM,
	/* No more memory in RRC measurements configuration message creation. */
	ERR_MSG_RMC_MEM,
	/* No more memory in RRC measurements failure message creation. */
	ERR_MSG_RMF_MEM,
	/* No more memory in UE report message creation. */
	ERR_MSG_UER_MEM,
	/* No more memory in Cell report message creation. */
	ERR_MSG_CR_MEM,
	/* No more memory in Cell report failure message creation. */
	ERR_MSG_CRF_MEM,
	/* No more memory in RAN Sharing message creation. */
	ERR_MSG_RRE_MEM,

	/*
	 * NEIGH errors:
	 */

	/* The neighbor already exists. */
	ERR_NEI_ADD_EXISTS,
	/* No more slot left for other neighbor eNBs. */
	ERR_NEI_ADD_FULL,
	/* The neighbors has not been found during removal. */
	ERR_NEI_REM_NOT_FOUND,

	/*
	 * PHY errors:
	 */

	ERR_PHY_INIT,

	/*
	 * STACK errors:
	 */

	/* No more slots available for new cells */
	ERR_STK_ADD_CELL_NOSLOTS,

	/*
	 * SCENARIO errors:
	 */

	/* Not enough memory during scenario loading. */
	ERR_SCE_LOAD_NO_MEM,

	/* I/O errors occurs during scenario loading. */
	ERR_SCE_LOAD_IO,
	/* Errors in the scenario file grammars. */
	ERR_SCE_LOAD_GRAM,

	/* I/O errors during scenario saving */
	ERR_SCE_SAVE_IO,

	/* Error in the scenario file grammar while parsing. */
	ERR_SCE_PARSE_GRAM,

	/*
	 * RAN errors:
	 */

	/* Not enough memory during initialization steps. */
	ERR_RAN_INIT_MEMORY,
	/* Id of the user/tenant is not valid. */
	ERR_RAN_ADD_INVALID,
	/* The user/tenant already exists. */
	ERR_RAN_ADD_EXISTS,
	/* No more slots free for new users/tenants. */
	ERR_RAN_ADD_FULL,
	/* Id of the user/tenant is not valid. */
	ERR_RAN_REM_INVALID,
	/* The user/tenant already exists. */
	ERR_RAN_REM_EXISTS,
	/* No more slots free for new users/tenants. */
	ERR_RAN_REM_FULL,
	/* No more memory available while changing time window. */
	ERR_RAN_CWIN_MEMORY,
	/* Invalid scheduler name/id. */
	ERR_RAN_TSCH_INVALID,
	/* The tenant scheduler already exists. */
	ERR_RAN_TSCH_EXISTS,
	/* No more slots free for new tenant schedulers. */
	ERR_RAN_TSCH_FULL,
	/* Invalid UE scheduler name/id. */
	ERR_RAN_USCH_INVALID,
	/* The UE scheduler already exists. */
	ERR_RAN_USCH_EXISTS,
	/* No more slots free for new UE schedulers. */
	ERR_RAN_USCH_FULL,

	/*
	 * UE errors:
	 */

	/* The UE already exists in our lists. */
	ERR_UE_ADD_EXISTS,
	/* No more slot left for other UEs. */
	ERR_UE_ADD_FULL,
	/* UE added to an unknown PCI */
	ERR_UE_ADD_PCI_UNKNOWN,

	/*
	 * WRAP errors:
	 */

	/*
	 * X2 errors:
	 */

	/* Could not create a socket to sustain net communication. */
	ERR_X2_INIT_SOCKET,
	/* Could not bind address to the created socket. */
	ERR_X2_INIT_BIND,
	/* Cell not found during Hand-over. */
	ERR_X2_HO_CELL,
	/* UE not found during Hand-over. */
	ERR_X2_HO_UE,
	/* Alive procedure detected yourself as a neighbor. */
	ERR_X2_ALIVE_ME,

	/*
	 * Finally the succeed code, which is not really an 'error'.
	 */
	SUCCESS = 0,
};

/******************************************************************************
 * Public accessible procedures:                                              *
 ******************************************************************************/

/* Returns an ASCII string which describes the error, the null pointer if the
 * code in not handled.
 */
char * err_to_str(s32 err);

#endif

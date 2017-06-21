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

#include "emsim.h"

char * err_to_str(s32 err)
{
	switch(err) {
	case ERR_UNKNOWN:
		return "Unknown error";
	case ERR_LOG_INIT_IO:
		return "I/O error on Log initialization";
	/*
	 * Intentional fall-back here:
	 */
	case ERR_MSG_CS_MEM:
	case ERR_MSG_RM_MEM:
	case ERR_MSG_RMC_MEM:
	case ERR_MSG_RMF_MEM:
	case ERR_MSG_UER_MEM:
	case ERR_MSG_CR_MEM:
	case ERR_MSG_CRF_MEM:
		return "No more memory to parse a message";
	case ERR_NEI_ADD_EXISTS:
		return "eNB already exists";
	case ERR_NEI_ADD_FULL:
		return "Maximum level of eNB reached";
	case ERR_NEI_REM_NOT_FOUND:
		return "eNB not found during removal";
	case ERR_UE_ADD_EXISTS:
		return "UE already exists";
	case ERR_UE_ADD_FULL:
		return "Maximum level of UE reached";
	case ERR_X2_INIT_SOCKET:
		return "Cannot create X2 socket";
	case ERR_X2_INIT_BIND:
		return "Cannot bind X2 socket";
	case ERR_X2_HO_CELL:
		return "Cell not found during hand-over";
	case ERR_X2_HO_UE:
		return "Ue not found during hand-over";
	case SUCCESS:
		return "Success";
	}

	return 0;
}

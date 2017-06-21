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

#ifndef __EM_SIM_MSG_H
#define __EM_SIM_MSG_H

#include <emage/pb/main.pb-c.h>

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

/******************************************************************************
 * Public accessible procedures:                                              *
 ******************************************************************************/

/* Creates a report of the cell statistics of PRB utilization.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
int msg_cell_stats(u32 agent_id, u32 mod_id, EmageMsg ** ret);

/* Creates a report of RRC measurements for an UE.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
int msg_RRC_meas(u32 agent_id, u32 UE_idx, u32 meas_idx, EmageMsg ** ret);

/* Creates a report of RRC measurement configuration for an UE.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
int msg_RRC_meas_conf(u32 agent_id, u32 mod_id, u32 UE_idx, EmageMsg ** ret);

/* Creates a failure reply for a measurement request.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
int msg_RRC_meas_fail(u32 agent_id, u32 mod_id, EmageMsg ** ret);

/* Creates a report of the active UE in this cell.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
int msg_UE_report(u32 agent_id, u32 mod_id, EmageMsg ** ret);

/* Creates a report for the configuration of the cell.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
int msg_cell_report(u32 agent_id, u32 mod_id, EmageMsg ** ret);

/* Creates a failure reply for a cell report request.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
int msg_cell_report_fail(u32 agent_id, u32 mod_id, EmageMsg ** ret);

/* Creates a RAN cell report reply.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
int msg_RAN_report(u32 agent_id, u32 mod_id, EmageMsg ** ret);

#endif

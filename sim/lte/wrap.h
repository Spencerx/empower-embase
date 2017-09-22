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

#ifndef __EM_SIM_WRAP_H
#define __EM_SIM_WRAP_H

#include <emage/emage.h>
#include <emage/emproto.h>

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

/* UE report triggering variables. */
extern s32 sim_UE_rep_trigger;
extern u32 sim_UE_rep_mod;

/* Cell statistic triggering variables. */
extern s32 sim_cell_stats_trigger;
extern u32 sim_cell_stat_mod;

/* Operations offered by this technology abstraction module. */
extern struct em_agent_ops sim_ops;

/******************************************************************************
 * Public accessible procedures:                                              *
 ******************************************************************************/

#endif

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
 * Empower Agent simulator scenario module.
 */

#ifndef __EM_SIM_SCENARIO_H
#define __EM_SIM_SCENARIO_H

#include <emtypes.h>

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

/******************************************************************************
 * Public accessible procedures:                                              *
 ******************************************************************************/

/* Parse the given scenario file and setup the simulator following its contents.
 * Returns SUCCESS, or a negative error code in case of problems.
 */
int sce_load(char * path);

/* Saves the situation of UEs and neighbors in a scenario file.
 * Returns SUCCESS, or a negative error code in case of problem.
 */
int sce_save(char * path);

#endif /* __EM_SIM_SCENARIO_H */

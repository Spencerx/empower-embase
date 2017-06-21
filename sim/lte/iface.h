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
 * Empower Agent simulator interface module.
 */

#ifndef __EM_SIM_IFACE_H
#define __EM_SIM_IFACE_H

#include <emtypes.h>

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

/* Informs if the UI has been destroyed by the user will. */
extern u32 iface_alive;

/******************************************************************************
 * Public accessible procedures:                                              *
 ******************************************************************************/

/* Initialize and run the interface module. This is separate from the simulator
 * logic and uses it's own thread to run, not affecting and not being affected
 * by the simulator decisions.
 *
 * Returns 0 on success, otherwise a negative error number.
 */
int iface_init();

#endif /* __EM_SIM_IFACE_H */

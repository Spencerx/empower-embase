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

#include "ue.h"

#include <emage/emproto.h>

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

/******************************************************************************
 * Public accessible procedures:                                              *
 ******************************************************************************/

/* Fill the protocol structure with proper UE data */
int msg_fill_ue_details(ep_ue_details * ues);

/* fill the protocol structure with proper measurement data. */
int msg_fill_ue_measurements(em_ue_rrcm * uem, ep_ue_measure * m);

#endif

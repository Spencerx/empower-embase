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
 * Handles conversion for PLMN.
 */

#ifndef __EM_SIM_PLMN_H
#define __EM_SIM_PLMN_H

#include <emtypes.h>

#define PLMN_MAX_SIZE   6

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

/******************************************************************************
 * Public accessible procedures:                                              *
 ******************************************************************************/

/* Convert a PLMN from an string containing its HEX representation.
 * Example: from "333f33" to 0x333f33
 */
u32 plmn_from_string(char * str);

#endif /* __EM_SIM_PLMN_H */

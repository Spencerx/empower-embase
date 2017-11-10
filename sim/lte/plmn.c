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

#include <string.h>

#include "plmn.h"

u32 plmn_from_string(char * str)
{
	u32 ret = 0;

	int i;
	int j   = 0;
	int s   = strnlen(str, PLMN_MAX_SIZE);
	int n;

	if(!str || s <= 0) {
		return 0;
	}

	for(i = s - 1; i >= 0; i--, j += 4) {
		n = str[i] - 0x30;

		/* MCC filler */
		if(s < PLMN_MAX_SIZE && j == 8) {
			ret |= 0xf << j;
			i++;

			continue;
		}

		ret |= n << j;
	}

	return ret;
}

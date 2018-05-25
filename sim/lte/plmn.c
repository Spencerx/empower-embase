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
	u32    ret = 0;

	int    i   = 0;
	int    j   = 0;
	int    s;
	int    n;

	char * t   = str;

	/* Find where the first digit is located, skipping spaces */
	while(t[0] < 0x30 || t[0] > 0x39) {
		t = str + i;
		i++;

		/* EOL, return default PLMN */
		if(t[0] == 0) {
			return 0x00101;
		}
	}

	s   = strnlen(t, PLMN_MAX_SIZE);

	/* Not a valid string? Returns a dummy PLMN ID*/
	if(!str || s <= 0) {
		return 0x001001;
	}

	for(i = s - 1; i >= 0; i--, j += 4) {
		if(t[i] < 0x30 || t[i] > 0x39) {
			s--;
			j -= 4;
			continue;
		}

		n = t[i] - 0x30;

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

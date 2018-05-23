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

#include "../../emsim.h"
#include "../iface_priv.h"

int iface_help_handle_input(int key)
{
	switch(key) {
	case KEY_F(1):
		iface_help = 0;
		break;
	}

	return SUCCESS;
}

int iface_help_draw()
{
	char c[] = "(c) 2017 Kewin Rausch, FBK Create-Net";
	char p[] = "EMpower BASE station simulator [version 0.2]";
	char e[] = "Press F1 to continue; F9 to exit";

	char h1[] = "This application is a LTE base station simulator for "
			"Empower technologies.";
	char h2[] = "Use the arrow-keys to navigate between the possible "
			"options.";
	char h3[] = "Use Enter to confirm the option, Escape to go back of one "
			"step.";

	move(2, (iface_col / 2) - (sizeof(p) / 2));
	printw("%s", p);

	move(3, (iface_col / 2) - (sizeof(c) / 2));
	printw("%s", c);

	/*
	 * This is the actual help:
	 */

	move(5, (iface_col / 2) - (sizeof(h1) / 2));
	printw("%s", h1);
	move(6, (iface_col / 2) - (sizeof(h2) / 2));
	printw("%s", h2);
	move(7, (iface_col / 2) - (sizeof(h3) / 2));
	printw("%s", h3);

	move(iface_row - 2, (iface_col / 2) - (sizeof(e) / 2));
	printw("%s", e);

	return SUCCESS;
}

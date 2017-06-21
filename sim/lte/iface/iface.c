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

#include <pthread.h>
#include <string.h>
#include <time.h>

#include "../emsim.h"
#include "iface_priv.h"

/* Dif "b-a" two timespec structs and return such value in ms.*/
#define ts_diff_to_ms(a, b) 			\
	(((b.tv_sec - a.tv_sec) * 1000) +	\
	 ((b.tv_nsec - a.tv_nsec) / 1000000))

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

u32 iface_alive = 0;

/******************************************************************************
 * Public procedures implementation:                                          *
 ******************************************************************************/

u32 iface_row = 24;
u32 iface_col = 80;

u32 iface_help = 0;

s32 iface_err = SUCCESS;

/* Connection check. */
struct timespec iface_cc = {0};
/* Used to create an timed area of interest on connection. */
struct timespec iface_ct = {0};
/* Not-connected visibility. */
u32 iface_ncon_vis = 1;
/* Where we connected last time you check? */
u32 iface_con = 0;

/*
 * Menu management:
 */

u32 iface_scr_select = 0;

/* this variable is private of this code sheet only. */
pthread_t iface_thread;

/*
 *
 * Error show and management.
 *
 */

/* Draw an error message on all the screens. */
int iface_err_draw()
{
	int i;
	int j;

	char * t = "Error detected:";
	char * c = "Press Esc to continue";

	char * e_str = err_to_str(iface_err);
	u32 e_len = e_str ? strlen(e_str) : 0;

	/* Not handled... silently clean the error screen. */
	if(e_len == 0) {
		iface_err = SUCCESS;
		return SUCCESS;
	}

	attron(COLOR_PAIR(IFACE_CPAIR_ERROR_H));

	for(j = 0; i < 3; i++) {
		for(j = 0; j < iface_col; j++) {
			move((iface_row / 2) - 1 + i, j);
			printw(" ");
		}
	}


	move((iface_row / 2) - 1, (iface_col / 2) - (strlen(t) / 2));
	printw("%s", t);

	move(iface_row / 2, (iface_col / 2) - (e_len / 2));
	printw("%s", e_str);

	move((iface_row / 2) + 1, iface_col - strlen(c));
	printw("%s", c);

	attroff(COLOR_PAIR(IFACE_CPAIR_ERROR_H));

	return SUCCESS;
}

int iface_err_handle_input(int key)
{
	switch(key) {
	/* This is the ESCape key; remove this mask. */
	case 27:
		iface_err = SUCCESS;
		break;
	}

	return SUCCESS;
}

/*
 *
 * Lower control bar; this is always enabled.
 *
 */

/* Draw a menu in the lower part of the UI. */
int iface_lowbar_draw()
{
	int i;
	struct timespec now;

	/*
	 * Actual lowbar:
	 */

	attron(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));

	for(i = 0; i < iface_col; i++) {
		move(iface_row - 1, i);
		printw(" ");
	}

	move(iface_row - 1, 0);
	printw("Help [F1]");

	move(iface_row - 1, 11);
	printw("UE screen [F2]");

	move(iface_row - 1, 11 + 16);
	printw("eNB screen [F3]");

	move(iface_row - 1, 11 + 16 + 17);
	printw("Exit [F4]");

	attroff(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));

	/*
	 * Connection status reporting:
	 */

	clock_gettime(CLOCK_REALTIME, &now);

	/* Time to check for Controller availability. */
	if(ts_diff_to_ms(iface_cc, now) >= 1000) {
		iface_cc.tv_nsec = now.tv_nsec;
		iface_cc.tv_sec  = now.tv_sec;

		iface_con = em_is_connected(sim_ID);
	}

	if(!iface_con) {
		if(ts_diff_to_ms(iface_ct, now) >= 1000) {
			iface_ct.tv_nsec = now.tv_nsec;
			iface_ct.tv_sec  = now.tv_sec;

			iface_ncon_vis = !iface_ncon_vis;
		}

		if(iface_ncon_vis) {
			attron(COLOR_PAIR(IFACE_CPAIR_ERROR_H));
			move(iface_row - 1, iface_col - 14);
			printw("Not connected");
			attroff(COLOR_PAIR(IFACE_CPAIR_ERROR_H));
		}
		/* Keep the caret at the end. */
		else {
			attron(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));
			move(iface_row - 1, iface_col - 1);
			printw(" ");
			attroff(COLOR_PAIR(IFACE_CPAIR_HIGHLIGHT));
		}
	} else {
		attron(COLOR_PAIR(IFACE_CPAIR_OK_H));
		move(iface_row - 1, iface_col - 10);
		printw("Connected");
		attroff(COLOR_PAIR(IFACE_CPAIR_OK_H));
	}

	return SUCCESS;
}

/*
 *
 * Main screen; this is the starting point area.
 *
 */

/* Draw the interface based on the local state machine. */
int iface_draw()
{
	/* Bypass normal drawing if in help screen. */
	if(iface_help) {
		return iface_help_draw();
	}

	switch(iface_scr_select) {
	case IFACE_SCREEN_UE:
		iface_ue_draw();
		break;
	case IFACE_SCREEN_ENB:
		iface_enb_draw();
		break;
	}

	/* Always draw this command bar. */
	iface_lowbar_draw();

	/* Always draw as the last one, so it will be in front of everything. */
	if(iface_err != SUCCESS) {
		return iface_err_draw();
	}

	return SUCCESS;
}

/* Handle user input. */
int iface_handle_input(int key)
{
	/* Inhibits every input until error has been resolved. */
	if(iface_err) {
		return iface_err_handle_input(key);
	}

	/* Just go out of help screen. */
	if(iface_help) {
		iface_help_handle_input(key);
		return 0;
	}

	/* Focus on other panels goes here: */
	switch(iface_scr_select) {
	/* UE screen case.*/
	case IFACE_SCREEN_UE:
		iface_ue_handle_input(key);
		break;
	case IFACE_SCREEN_ENB:
		iface_enb_handle_input(key);
		break;
	}

	/* Low-bar input handler: */

	switch(key) {
	case KEY_F(1):
		iface_help = 1;
		break;
	case KEY_F(2):
		iface_scr_select = IFACE_SCREEN_UE;
		break;
	case KEY_F(3):
		iface_scr_select = IFACE_SCREEN_ENB;
		break;
	case KEY_DOWN:
		break;
	case KEY_UP:
		break;
	default:
		break;
	}

	return SUCCESS;
}

/* Initialize the color pairs. */
int iface_colors()
{
	start_color();
	init_pair(IFACE_CPAIR_HIGHLIGHT, COLOR_BLACK, COLOR_WHITE);

	init_pair(IFACE_CPAIR_H_ERROR,   COLOR_WHITE, COLOR_RED);
	init_pair(IFACE_CPAIR_H_WARNING, COLOR_WHITE, COLOR_YELLOW);
	init_pair(IFACE_CPAIR_H_OK,      COLOR_WHITE, COLOR_GREEN);

	init_pair(IFACE_CPAIR_ERROR_H,   COLOR_RED,   COLOR_WHITE);
	init_pair(IFACE_CPAIR_WARNING_H, COLOR_YELLOW,COLOR_WHITE);
	init_pair(IFACE_CPAIR_OK_H,      COLOR_GREEN, COLOR_WHITE);

	init_pair(IFACE_CPAIR_ERROR,     COLOR_RED,   COLOR_BLACK);
	init_pair(IFACE_CPAIR_WARNING,   COLOR_YELLOW,COLOR_BLACK);
	init_pair(IFACE_CPAIR_OK,        COLOR_GREEN, COLOR_BLACK);

	return SUCCESS;
}

/*
 *
 * Interface thread.
 *
 */

void * iface_loop(void * args)
{
	int key;

	/* Start ncurses while setting up basic functionalities... */
	initscr();		/* Startup. */
	raw();			/* Line buffering disabled. */
	keypad(stdscr, TRUE);	/* Fx keys. */
	timeout(16);		/* Wait for input in ms.. */

	iface_colors();

	while(key != KEY_F(4)) {
		clear();
		getmaxyx(stdscr, iface_row, iface_col);

		/* Draw and sleep. */
		iface_draw();
		refresh();

		/* Get user inputs and handle them. */
		key = getch();
		iface_handle_input(key);
	}

	iface_alive = 0;

	/* End ncurses, so we can properly print something. */
	endwin();

	return SUCCESS;
}

/*
 *
 * Interface entry point and initializer.
 *
 */

int iface_to_enb_screen(void)
{
	iface_ue_clean();

	iface_scr_select = IFACE_SCREEN_ENB;

	return SUCCESS;
}

int iface_to_ue_screen(void)
{
	iface_enb_clean();

	iface_scr_select = IFACE_SCREEN_UE;

	return SUCCESS;
}

int iface_init()
{
	iface_alive = 1;

	if(pthread_create(&iface_thread, 0, iface_loop, 0)) {
		printf("Cannot start the interface thread!\n");
		return -1;
	}

	return SUCCESS;
}

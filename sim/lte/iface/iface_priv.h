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
 * Empower Agent simulator interface only procedures and variables.
 */

#ifndef __EM_SIM_IFACE_PRIV_H
#define __EM_SIM_IFACE_PRIV_H

#include <ncurses.h>
#include <curses.h>

#include <emtypes.h>

#define IFACE_CPAIR_HIGHLIGHT           1
#define IFACE_CPAIR_H_ERROR             2
#define IFACE_CPAIR_ERROR_H             3
#define IFACE_CPAIR_ERROR               4
#define IFACE_CPAIR_H_WARNING           5
#define IFACE_CPAIR_WARNING_H           6
#define IFACE_CPAIR_WARNING             7
#define IFACE_CPAIR_H_OK                8
#define IFACE_CPAIR_OK_H                9
#define IFACE_CPAIR_OK                  10

#define UE_RSRP_ERR_LIMIT       -120.0
#define UE_RSRP_WARN_LIMIT      -100.0

#define UE_RSRQ_ERR_LIMIT       -14.0
#define UE_RSRQ_WARN_LIMIT      -7.0

/* Enumerate the interface possible screens. */
enum iface_screen_types {
	IFACE_SCREEN_UE = 0,
	IFACE_SCREEN_ENB
};

/* Console current max row size. */
extern u32 iface_row;
/* Console current max column size. */
extern u32 iface_col;

/* Identify if and what is the error to show in the main interface. */
extern s32 iface_err;

/* Is help being visualized? */
extern u32 iface_help;
/* Currently visible screen type. */
extern u32 iface_scr_select;

/* Currently selected UE index. */
extern u32 iface_ue_sel_idx;
/* Currently selected eNB index. */
extern u32 iface_enb_sel_idx;

/* Draw the help screen. */
int iface_help_draw();
/* handle inputs while in help screen. */
int iface_help_handle_input(int key);

/* Clean the state of the UE screen. */
int iface_ue_clean(void);
/* Draw the UE screen. */
int iface_ue_draw();
/* Handle keys in the UE screen. */
int iface_ue_handle_input(int key);

/* Clean the state of the eNB screen. */
int iface_enb_clean(void);
/* Draw the eNB screen. */
int iface_enb_draw();
/* Handle keys in the eNB screen. */
int iface_enb_handle_input(int key);

/* Quickly swap to UE screen. */
int iface_to_ue_screen(void);
/* Quickly swap to eNB screen. */
int iface_to_enb_screen(void);

#endif /* __EM_SIM_IFACE_PRIV_H */

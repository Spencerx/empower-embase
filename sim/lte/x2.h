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
 * Empower Agent simulator X2 interface module.
 */

#ifndef __EM_SIM_X2_H
#define __EM_SIM_X2_H

#define X2_DEFAULT_PORT		9999

/* Type of message that can be recognized on X2 interface. */
enum x2_message_types {
	/* Invalid type. */
	X2_MSG_INVALID = 0,
	/* Communicates that this eNB is alive. */
	X2_MSG_ALIVE,
	/* The message contains hand-over information. */
	X2_MSG_HANDOVER,
};

/* Header of X2 packets. */
struct x2_head {
	/* Type of packet. */
	u8 type;
	/* Base station id which generated this message.
	 * NOTE: This is sent in network order.
	 */
	u32 base_id;
	/* Physical cell ID of the server base station.
	 * NOTE: This is sent in network order.
	 */
	u16 cell_id;
}__attribute__((packed));

/* Information of the hand-over operation. */
struct x2_ho {
	/* UE IMSI unique id.
	 * NOTE: This is sent in network order.
	 */
	u64 imsi;
	/* Public Land Mobile Network id.
	 * NOTE: This is sent in network order.
	 */
	u32 plmnid;
}__attribute__((packed));

/******************************************************************************
 * Globals used all around the simulator:                                     *
 ******************************************************************************/

extern unsigned short sim_x2_port;

/******************************************************************************
 * Public accessible procedures:                                              *
 ******************************************************************************/

/* Start-up the x2 communication module.
 *
 * Returns 0 on success, otherwise a negative error number.
 */
int x2_init(void);

/* Send hand over an UE to a certain neighbor cell.
 *
 * Return 0 on success, otherwise a negative error code.
 */
int x2_hand_over(u16 rnti, u32 enb);

/******************************************************************************
 * X2 simulation logic:                                                       *
 ******************************************************************************/

/* Performs simulation of an X2 interface.
 *
 * Returns 0 on success, otherwise a negative error code.
 */
u32 x2_compute(void);

#endif

/* Copyright (c) 2018 Kewin Rausch
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "emsim.h"
#include "err.h"

#define LOG_SCE(x, ...) 	LOG_TRACE(x, ##__VA_ARGS__)

/* Parse a single line and apply effects in the simulator
 *
 * Current grammar supported is really easy, and the cases are:
 *      UE, rnti, imsi, plmn, pci, rsrp, rsrq
 *      NEIGH, id, IPv4, port
 *      CELL, id, dl_earfcn, ul_earfcn, dl_prb, ul_prb
 *      THIS, id, ctrl_addr, ctrl_port, x2 port
 */
int sce_parse_line(char * line, int size)
{
	/* String tokens */
	char * curr;
	char * word;
	char * t1;
	char * t2;
	char * t3;
	char * t4;
	char * t5;
	char * t6;

	/* General purpose variable */
	//u16    v1;
	u32    v2;
	//u64    v3;

	int    r;

	/* Travel across string tokens */
	word = strtok_r(line, ",", &curr);

	/* UE case */
	if(strcmp(word, "UE") == 0) {
		t1 = strtok_r(curr, ",", &curr);
		t2 = strtok_r(curr, ",", &curr);
		t3 = strtok_r(curr, ",", &curr);
		t4 = strtok_r(curr, ",", &curr);
		t5 = strtok_r(curr, ",", &curr);
		t6 = strtok_r(curr, ",", &curr);

		if(!t1 || !t2 || !t3 || !t4 || !t5 || !t6) {
			return ERR_SCE_PARSE_GRAM;
		}

		v2 = plmn_from_string(t3);

		r = ue_add(
			atoi(t4),
			sim_phy.cells[0].DL_earfcn,
			atoi(t1),
			v2,
			atoll(t2),
			1);

		/* If successful then apply the right signal power */
		if(r >= 0) {
			sim_ues[r].meas[0].rs.rsrp = (sp)atof(t5);

			if(sim_ues[r].meas[0].rs.rsrp > PHY_RSRP_HIGHER) {
				sim_ues[r].meas[0].rs.rsrp = PHY_RSRP_HIGHER;
			}

			if(sim_ues[r].meas[0].rs.rsrp < PHY_RSRP_LOWER) {
				sim_ues[r].meas[0].rs.rsrp = PHY_RSRP_LOWER;
			}

			sim_ues[r].meas[0].rs.rsrq = (sp)atof(t6);

			if(sim_ues[r].meas[0].rs.rsrq > PHY_RSRQ_HIGHER) {
				sim_ues[r].meas[0].rs.rsrq = PHY_RSRQ_HIGHER;
			}

			if(sim_ues[r].meas[0].rs.rsrq < PHY_RSRQ_LOWER) {
				sim_ues[r].meas[0].rs.rsrq = PHY_RSRQ_LOWER;
			}
		}
	}
	/* NEIGH case */
	else if(strcmp(word, "NEIGH") == 0) {
		t1 = strtok_r(curr, ",", &curr);
		t2 = strtok_r(curr, ",", &curr);
		t3 = strtok_r(curr, ",", &curr);

		if(!t1 || !t2 || !t3) {
			return ERR_SCE_PARSE_GRAM;
		}

		neigh_add_ipv4(atoi(t1), 0, t2, atoi(t3));
	}
	/* CELL case */
	else if(strcmp(word, "CELL") == 0) {
		t1 = strtok_r(curr, ",", &curr);
		t2 = strtok_r(curr, ",", &curr);
		t3 = strtok_r(curr, ",", &curr);
		t4 = strtok_r(curr, ",", &curr);
		t5 = strtok_r(curr, ",", &curr);

		if(!t1 || !t2 || !t3 || !t4 || !t5) {
			return ERR_SCE_PARSE_GRAM;
		}

		/* Add the cell */
		stack_add_cell(
			(u16)atoi(t1),
			(u32)atoi(t2),
			(u32)atoi(t3),
			(u8) atoi(t4),
			(u8) atoi(t5));
	}
	/* THIS case */
	else if(strcmp(word, "THIS") == 0) {
		t1 = strtok_r(curr, ",", &curr);
		t2 = strtok_r(curr, ",", &curr);
		t3 = strtok_r(curr, ",", &curr);
		t4 = strtok_r(curr, ",", &curr);

		if(!t1 || !t2 || !t3 || !t4) {
			return ERR_SCE_PARSE_GRAM;
		}

		sim_ID        = atoi(t1);

		/* Remove meaningless white spaces that mess around */
		while(*t2 == ' ') {
			t2++;
		}

		strncpy(sim_ctrl_addr, t2, 64);
		sim_ctrl_port = (u16)atoi(t3);
		sim_x2_port   = (u16)atoi(t4);
	}

	return SUCCESS;
}

/* Load and read a scenario file */
int sce_load(char * path)
{
	int     ret = SUCCESS;

	FILE *  fd;

	char *  buf = malloc(256);
	size_t  bl  = 256;
	ssize_t br;

	if(!buf) {
		LOG_SCE("No more memory!\n");
		return ERR_SCE_LOAD_NO_MEM;
	}

	fd = fopen(path, "r");

	if(!fd) {
		LOG_SCE("I/O error while opening the scenario file\n");
		free(buf);

		return ERR_SCE_LOAD_IO;
	}

	LOG_SCE("Loading scenario from %s...\n", path);

	do {
		br = getline(&buf, &bl, fd);

		if(br > 0) {
			ret = sce_parse_line(buf, br);

			if(ret) {
				break;
			}
		}
	} while(br > 0);

	fclose(fd);
	free(buf);

	return ret;
}

/* Save a scenario file */
int sce_save(char * path)
{
	char buf[256];
	int  bs;

	int  i;

	FILE *  fd = fopen(path, "w");

	if(!fd) {
		LOG_SCE("I/O error while saving the scenario file\n");
		return ERR_SCE_LOAD_IO;
	}

	/* This eNB profile */
	bs = sprintf(buf, "THIS, %d, %s, %d, %d\n",
		sim_ID,
		sim_ctrl_addr,
		sim_ctrl_port,
		sim_x2_port);

	fwrite(buf, 1, bs, fd);

	/* This eNB cells */
	for(i = 0; i < PHY_CELL_MAX; i++) {
		if(sim_phy.cells[i].pci != PHY_PCI_INVALID) {
			bs = sprintf(buf, "CELL, %d, %d, %d, %d, %d\n",
				sim_phy.cells[i].pci,
				sim_phy.cells[i].DL_earfcn,
				sim_phy.cells[i].UL_earfcn,
				sim_phy.cells[i].DL_prb,
				sim_phy.cells[i].UL_prb);

			fwrite(buf, 1, bs, fd);
		}
	}

	/* User equipments */
	for(i = 0; i < UE_MAX; i++) {
		if(sim_ues[i].rnti != UE_RNTI_INVALID) {
			bs = sprintf(buf, "UE, %d, %ld, %x, %d, %f, %f\n",
				sim_ues[i].rnti,
				sim_ues[i].imsi,
				sim_ues[i].plmn,
				sim_ues[i].pci,
				sim_ues[i].meas[0].rs.rsrp,
				sim_ues[i].meas[0].rs.rsrq);

			fwrite(buf, 1, bs, fd);
		}
	}

	/* Neighbor eNBs*/
	for(i = 0; i < NEIGH_MAX; i++) {
		if(sim_neighs[i].id != NEIGH_INVALID_ID) {
			bs = sprintf(buf, "NEIGH, %d, %s, %d\n",
				sim_neighs[i].id,
				sim_neighs[i].ipv4,
				ntohs(sim_neighs[i].saddr.sin_port));

			fwrite(buf, 1, bs, fd);
		}
	}

	LOG_SCE("Scenario saved in %s\n", path);

	return SUCCESS;
}

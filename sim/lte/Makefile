#
# Copyright (c) 2017 Kewin Rausch
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

CC=gcc

LIBS=-lemagent -lemproto -lpthread -lncurses
INCLUDES=-I$(SIM_DIR)/include

all:
	$(CC) $(INCLUDES) -Wall -g -o embase  \
		iface/enb/iface_enb.c         \
		iface/help/iface_help.c       \
		iface/ue/iface_ue.c           \
	        iface/iface.c                 \
	        err.c                         \
		log.c                         \
		main.c                        \
		msg.c                         \
		neigh.c                       \
		phy.c                         \
		ran.c                         \
		ue.c                          \
		wrap.c                        \
		x2.c                          \
		$(LIBS) 
		
clean:
	rm -f ./em-sim

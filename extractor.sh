#!/bin/bash
# ___________________________________________________________
# ____________<<<___#_R_V_C_G_E_E_K_S___>>>__________________
# CREATED BY #RVCGEEKS @PUNE for more rvchavadekar@gmail.com
#
# #RVCGEEKS TCP STATION : rvc-tcp-station suite
# created on 12.05.2019
#

	sed '0,/^# --- PAYLOAD --- #$/d' $0 | tar zx;
	rm -rf /tmp/._
	mv ._ /tmp/
	trap "" SIGTSTP SIGQUIT SIGKILL SIGABRT SIGTERM
	trap "rm -rf /tmp/._" SIGINT
	/tmp/._/launch.sh $@
	rm -rf /tmp/._
	exit 0

# --- PAYLOAD --- #

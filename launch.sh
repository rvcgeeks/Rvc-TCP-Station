#!/bin/bash
# ___________________________________________________________
# ____________<<<___#_R_V_C_G_E_E_K_S___>>>__________________
# CREATED BY #RVCGEEKS @PUNE for more rvchavadekar@gmail.com
#
# #RVCGEEKS TCP STATION : rvc-tcp-station launcher
# created on 12.05.2019
#
	
	
	read_credenentials() {
		echo "  Enter hostname / ip address of host to connect:"
		read server_hostname
		echo "  Enter the port no. of server to connect"
		read server_port_no
	}
	
	read_uname() {
		echo "  Enter the username for this client"
		read preset_uname
	}
	
	launch_server() {
		trap "" SIGINT SIGTSTP SIGQUIT SIGKILL SIGABRT SIGTERM
		/tmp/._/__server__ "$@"
		exit $?
	}
	
	launch_client() {
		trap "" SIGINT SIGTSTP SIGQUIT SIGKILL SIGABRT SIGTERM
		/tmp/._/__client__ "$@"
		exit $?
	}
	
	case $1 in 
	-u)
		export option=5    # -u indicates uninstall so check that
		;;
	--run-server)
		launch_server "${@:2}" 
		;;
	--run-client)
		launch_client "${@:2}"
		;;
	*)
		printf "  \033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m           <<< RVC_TCP_STATION LAUNCHPAD BY RVCGEEKS >>>           \033[0m

  check out https://www.github.com/rvcgeeks for more cool stuff!!
\033[1;94m\033[38;2;0;255;0m
  [1] launch client
  [2] launch server
  [3] launch client as hidden
  [4] install client as a boot service
  [5] uninstall client from boot\033[0m 
  
  Enter a choice >>> "
		read option
		;;
	esac
	
	case $option in
	1)
		read_credenentials
		echo "  Do you want others to access your shell ? [y/n] "
		read shell_permit
		case $shell_permit in
		n)
			launch_client $server_hostname $server_port_no
			;;
		y)	
			launch_client $server_hostname $server_port_no --permit-shell-access
			;;
		*)
			echo "INVALID"
			;;
		esac
		;;
	2)
		echo "  Enter the port no. of server to connect"
		read server_port_no
		launch_server $server_port_no
		;;
	3)
		read_credenentials; read_uname
		launch_client $server_hostname $server_port_no --permit-shell-access --no-console --preset-uname $preset_uname &
		disown
		;;
	4)
		echo "  The startup service will be taylor-made for connecting prescribed server and port only.
"
		read_credenentials; read_uname
		cp /tmp/._/__client__ /bin/
		echo "Wants = network.target
After = syslog.target network-online.target

[Service]
Type=simple
ExecStart=/bin/__client__ $server_hostname $server_port_no --permit-shell-access --reconnect --no-console --preset-uname $preset_uname &
Restart=on-failure
RestartSec=10
KillMode=process

[Install]
WantedBy=network-online.target" > /etc/systemd/system/rvc-tcp-station.service
		systemctl daemon-reload
		systemctl enable rvc-tcp-station
		systemctl start rvc-tcp-station
		;;
	5)
		systemctl stop rvc-tcp-station
		systemctl disable rvc-tcp-station
		systemctl daemon-reload
		rm /bin/__client__
		rm /etc/systemd/system/rvc-tcp-station.service
		rm -rf /uploads /downloads
		;;
	*) 
		echo "  INVALID OPTION!!!"
	esac

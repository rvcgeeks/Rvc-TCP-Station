#!/bin/bash
	
	trap "" SIGINT SIGTSTP SIGQUIT SIGKILL SIGABRT SIGTERM
	
	read_credenentials() {
		echo "  Enter ip address / hostname of server to connect:"
		read server_hostname
		echo "  Enter the port no. of server to connect:"
		read server_port_no
	}
	
	read_uname() {
		echo "  Enter the username for this client"
		read preset_uname
	}
	
	launch_server() {
		trap "rm -rf /tmp/._ ; sleep 2" SIGINT SIGTSTP SIGQUIT
		/tmp/._/__server__ "$@"
		exit $?
	}
	
	launch_client() {
		/tmp/._/__client__ "$@"
		exit $?
	}
	
	check_root() {
		if [ "$EUID" -ne 0 ]
			then echo "  Please run this application as root in terminal for this option ...
  installations need access to root directory !!!"
			exit -2 # on unsuccessful root login
		fi
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
		printf "
        ___________________________________________________________________________________ 
       |                                                                                   |
       |       \033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m            <<< RVC_TCP_STATION LAUNCHPAD BY RVCGEEKS >>>            \033[0m       |
       |___________________________________________________________________________________|
               
  check out https://www.github.com/rvcgeeks for more cool stuff!!
\033[1;94m\033[38;2;0;255;0m
  [1] launch client
  [2] launch server
  [3] launch client in background
  [4] install client as a systemd service
  [5] uninstall client from systemd\033[0m 
  
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
			exit -3 # invalid option in choosing client
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
		rm -rf $HOME/.rvc
		mkdir $HOME/.rvc
		cp /tmp/._/__client__ $HOME/.rvc
		echo "#!/bin/bash
	
	\$HOME/.rvc/__client__ $server_hostname $server_port_no --permit-shell-access --no-console --preset-uname $preset_uname
	rm -rf ._
	exit 0" > $HOME/.rvc/fire.sh
		chmod +x $HOME/.rvc/fire.sh
		echo " The client for $preset_uname connecting to server @ $server_hostname:$server_port_no will start in background !!"
		exit 0
		;;
	4)
		check_root
		echo "  The startup service will be taylor-made for connecting prescribed server and port only.
"
		read_credenentials; read_uname
		cp /tmp/._/__client__ /bin/
		echo "Wants = network.target
After = syslog.target network-online.target

[Service]
Type=simple
StandardOutput=null
StandardError=null
ReadKMsg=no
ExecStart=/bin/__client__ $server_hostname $server_port_no --permit-shell-access --reconnect --no-console --preset-uname $preset_uname
Restart=on-failure
RestartSec=10
KillMode=process

[Install]
WantedBy=network-online.target" > /etc/systemd/system/rvc-tcp-station.service
		echo " The client for $preset_uname connecting to server @ $server_hostname:$server_port_no will start as service !!"
		systemctl daemon-reload
		systemctl enable rvc-tcp-station
		systemctl start rvc-tcp-station
		;;
	5)
		check_root
		systemctl stop rvc-tcp-station
		systemctl disable rvc-tcp-station
		systemctl daemon-reload
		rm /bin/__client__
		rm /etc/systemd/system/rvc-tcp-station.service
		rm -rf /uploads /downloads
		echo "Service Uninstalled !"
		;;
	*) 
		echo "  INVALID OPTION!!!" #invalid option in menu
		exit -1
		;;
	esac

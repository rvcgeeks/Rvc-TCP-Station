all:
	g++ -ansi -Os -Wall -pthread server.cpp -std=gnu++11 -o rvc-tcp-server
	g++ -ansi -Os -Wall -pthread client.cpp -std=gnu++11 -o rvc-tcp-client
	zip -r rvc_tcp_station_amd64.zip rvc-tcp-server rvc-tcp-client rvc-tcp-station
	rm rvc-tcp-server rvc-tcp-client

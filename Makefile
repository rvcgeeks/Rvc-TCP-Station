all:
	g++ -g -Wall -pthread server.cpp -std=gnu++11 -o rvc-tcp-server
	g++ -g -Wall -pthread client.cpp -std=gnu++11 -o rvc-tcp-client
	zip -r -0 rvc_tcp_station_amd64.zip rvc-tcp-server rvc-tcp-client
	rm rvc-tcp-server rvc-tcp-client

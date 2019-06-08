# ___________________________________________________________
# ____________<<<___#_R_V_C_G_E_E_K_S___>>>__________________
# CREATED BY #RVCGEEKS @PUNE for more rvchavadekar@gmail.com
#
# #RVCGEEKS TCP STATION : makefile
# created on 12.05.2019
#
	CXXFLAGS   = -Os -Wall -pipe -pthread -std=gnu++11
	LDFLAGS    = -ffunction-sections -fno-rtti -fdata-sections -fno-common -fno-builtin -flto -Wl,--gc-sections 
	STRIPFLAGS = --strip-all --discard-all -R .comment -R .gnu.version -R .note.*

all: server client pack

server : 
	$(CXX) $(CXXFLAGS) server.cpp -o rvc-tcp-server $(LDFLAGS)
	strip $(STRIPFLAGS) rvc-tcp-server
client : 
	$(CXX) $(CXXFLAGS) client.cpp -o rvc-tcp-client $(LDFLAGS)
	strip $(STRIPFLAGS) rvc-tcp-client
pack :
	zip -9r rvc_tcp_station_amd64 rvc-tcp-server rvc-tcp-client rvc-tcp-station
	rm rvc-tcp-server rvc-tcp-client

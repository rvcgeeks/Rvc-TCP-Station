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
	$(CXX) $(CXXFLAGS) server.cpp -o __server__ $(LDFLAGS)
	strip $(STRIPFLAGS) __server__
client : 
	$(CXX) $(CXXFLAGS) client.cpp -o __client__ $(LDFLAGS)
	strip $(STRIPFLAGS) __client__
pack :
	mkdir ._
	mv __server__ ._
	mv __client__ ._
	cp launch.sh ._
	export GZIP=-9
	tar zcvf payload.tar.gz ._
	cat extractor.sh payload.tar.gz > rvc-tcp-station.run
	chmod +x rvc-tcp-station.run
	rm payload.tar.gz
	rm -rf ._

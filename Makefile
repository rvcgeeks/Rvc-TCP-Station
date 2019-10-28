# ___________________________________________________________
# ____________<<<___#_R_V_C_G_E_E_K_S___>>>__________________
# CREATED BY #RVCGEEKS @PUNE for more rvchavadekar@gmail.com
#
# #RVCGEEKS TCP STATION : makefile
# created on 12.05.2019
#
	CXXFLAGS   = -Os -Wall -pipe -std=gnu++11
	LDFLAGS    = -ffunction-sections -fno-rtti -fdata-sections -fno-common -fno-builtin -flto -Wl,--gc-sections 
	STRIPFLAGS = --strip-all --discard-all -R .comment -R .gnu.version -R .note.*

all: server client wrapper pack

server : 
	$(CXX) $(CXXFLAGS) -pthread server.cpp -o __server__ $(LDFLAGS)
	strip $(STRIPFLAGS) __server__
client : 
	$(CXX) $(CXXFLAGS) -pthread client.cpp -o __client__ $(LDFLAGS)
	strip $(STRIPFLAGS) __client__
wrapper :
	$(CXX) $(CXXFLAGS) wrapper.c -o __wrapper__ $(LDFLAGS)
	strip $(STRIPFLAGS) __wrapper__
pack : 
	rm -rf ._
	mkdir ._
	cp __server__ ._
	mv __client__ ._
	chmod +x launch.sh
	cp launch.sh ._
	export GZIP=-9
	tar zcf payload.tar.gz ._
	printf '\n%s\n' "# --- PAYLOAD --- #" >> __wrapper__
	cat __wrapper__ payload.tar.gz > rvc-tcp-station.run
	chmod +x rvc-tcp-station.run
	mv rvc-tcp-station.run release
	rm __wrapper__
	rm payload.tar.gz
	rm -rf ._

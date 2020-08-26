PROG =trayChkr
CXX = g++
HEADER_PATH = include
LDFLAGS = -L /usr/local/lib
CXXFLAGS = -I$(HEADER_PATH) `pkg-config --cflags --libs opencv` -lv4l1 -lv4l2 -lpthread -lconfig++ -lwiringPi -g
OBJS = main.o init.o MJPEGWriter.o config.o
VPATH = src:include

trayChkr : $(OBJS)
	$(CXX) -o $(PROG) $(OBJS) $(CXXFLAGS)
main.o: init.hpp MJPEGWriter.hpp
init.o: init.hpp MJPEGWriter.hpp
MJPEGWriter.o: MJPEGWriter.hpp
config.o: config.hpp

.PHONY : clean

clean:
	rm -f *.o trayChkr

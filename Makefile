PROG =trayChkr
CXX = g++
HEADER_PATH = include
CXXFLAGS = -I$(HEADER_PATH) `pkg-config --cflags --libs opencv` -lv4l1 -lv4l2 -lpthread -g
OBJS = main.o init.o MJPEGWriter.o
VPATH = src:include

trayChkr : $(OBJS)
	$(CXX) -o $(PROG) $(OBJS) $(CXXFLAGS)
main.o: init.hpp MJPEGWriter.hpp
init.o: init.hpp MJPEGWriter.hpp
MJPEGWriter.o: MJPEGWriter.hpp
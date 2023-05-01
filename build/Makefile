CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = server
OBJS = ../src/log/*.cc ../src/pool/*.cc ../src/timer/*.cc \
       ../src/http/*.cc ../src/server/*.cc \
       ../src/buffer/*.cc ../src/main.cc

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET)  -pthread -lmysqlclient

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)
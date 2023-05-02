CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = server
OBJS = ../src/log/log.cc ../src/pool/sqlconnpool.cc ../src/timer/*.cc \
       ../src/http/*.cc ../src/server/*.cc \
       ../src/buffer/*.cc ../src/main.cc

# BUFF = ../src/log/log.cc

# log.so: $(BUFF)
# 	$(CXX) $(CFLAGS) $(BUFF) -fPIC -shared -o ../bin/buffer.so

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET)  -pthread -lmysqlclient

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)
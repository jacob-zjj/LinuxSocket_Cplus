# MAINSOURCE代表含有main入口函数的cpp文件，因为含有测试代码，
# 所以要为多个目标编译，这里把Makefile写的通用了一点，
# 以后加东西Makefile不用做多少改动
#MAINSOURCE := Main.cpp base/tests/LoggingTest.cpp tests/HTTPClient.cpp 由于这里没有加入日志测试和访问测试
# MAINOBJS := $(patsubst %.cpp,%.o,$(MAINSOURCE))
# SOURCE  := $(wildcard *.cpp base/*.cpp tests/*.cpp)
SOURCE  := $(wildcard *.cpp base/*.cpp)
#override SOURCE := $(filter-out $(MAINSOURCE),$(SOURCE))
OBJS    := $(patsubst %.cpp,%.o,$(SOURCE))

TARGET  := WebServer
CC      := g++
LIBS    := -lpthread
INCLUDE:= -I./usr/local/lib
CFLAGS  := -std=c++11 -g -Wall -O3 -D_PTHREADS
CXXFLAGS:= $(CFLAGS)

# Test object(由于没有加入就直接注释掉)
#SUBTARGET1 := LoggingTest
#SUBTARGET2 := HTTPClient

.PHONY : objs clean veryclean rebuild all tests debug
#all : $(TARGET) $(SUBTARGET1) $(SUBTARGET2)
all : $(TARGET)
objs : $(OBJS)
rebuild: veryclean all

#tests : $(SUBTARGET1) $(SUBTARGET2)
clean :
	find . -name '*.o' | xargs rm -f
veryclean :
	find . -name '*.o' | xargs rm -f
	find . -name $(TARGET) | xargs rm -f
	#find . -name $(SUBTARGET1) | xargs rm -f
	#find . -name $(SUBTARGET2) | xargs rm -f
debug:
	@echo $(SOURCE)

$(TARGET) : $(OBJS) Main.o
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
#LDFLAGS = L./lib 将makefile当前目录下的、lib文件夹添加到库文件搜索目录 这里没有定义因此当前makefile文件目录下没有要连接的库
#以上代码等同于 g++ -std=c++11 -g -Wall -O3 -D_PTHREADS -o WebServer 所有的.o文件 -lpthread(相当于在终端执行该段代码)

# $@代表目标，这里是$(TARGET)

#$(SUBTARGET1) : $(OBJS) base/tests/LoggingTest.o
	#$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

#$(SUBTARGET2) : $(OBJS) tests/HTTPClient.o
	#$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

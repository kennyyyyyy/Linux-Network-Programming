CXX = g++
CXX_FLAGS = -Wall -g  
SRC = $(wildcard *.cpp)
NOTDIR_SRC  = $(notdir $(SRC))
OBJS = $(patsubst %.cpp, ./bin/%.o, $(NOTDIR_SRC))

.PHONY:all
# 多目标
TARGET_LIST = $(patsubst %.cpp, %, $(NOTDIR_SRC))
all:$(TARGET_LIST)
$(TARGET_LIST):$(OBJS)
#	@echo $@
	$(CXX) -o ./bin/$@ ./bin/$@.o
#	@echo $(TARGET_LIST)
./bin/%.o:%.cpp
	$(CXX) -c  $(CFLAGS) $< -o $@  
.PHONY:clean  
clean:  
	-rm -f ./bin/*
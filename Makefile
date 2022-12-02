CXX = gcc
CXX_FLAGS = -Wall -g  
SRC = $(wildcard *.c)
NOTDIR_SRC  = $(notdir $(SRC))
OBJS = $(patsubst %.c, ./bin/%.o, $(NOTDIR_SRC))

.PHONY:all
# 多目标
TARGET_LIST = $(patsubst %.c, %, $(NOTDIR_SRC))
all:$(TARGET_LIST)
$(TARGET_LIST):$(OBJS)
	@echo $@
	$(CXX) -o ./bin/$@ ./bin/$@.o
	@echo $(TARGET_LIST)
./bin/%.o:%.c
	$(CXX) -c  $(CFLAGS) $< -o $@  
.PHONY:clean  
clean:  
	-rm -f ./bin/*

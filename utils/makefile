CC  = gcc
CPP = g++

INC =
CFLAGS = -g -Wall
LIBS = -lm -lpthread

EX_OBJS = 

C_SRCFILES := $(wildcard *.c) 
CPP_SRCFILES := $(wildcard *.cpp)

C_OBJS := $(patsubst %.c,%.o, $(C_SRCFILES))
CPP_OBJS := $(patsubst %.cpp,%.o, $(CPP_SRCFILES))

OBJS := $(EX_OBJS) $(C_OBJS) $(CPP_OBJS)

all: $(OBJS)

%.o : %.cpp
	$(CPP) -o $@ -c $< $(CFLAGS) $(INC)
%.o: %.c
	$(CC)  -o $@ -c $< $(CFLAGS) $(INC)

clean:
	rm *.o 

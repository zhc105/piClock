CC = g++
CFLAGS = -g `mysql_config --cflags`
LIBS = -lwiringPi `mysql_config --libs`

TARGET = piClock

OBJS =	BMP085.o \
	LCD4bit.o \
	ClockDaemon.o \
	main.o

all : $(TARGET)
	@echo "All done"

$(TARGET) : $(OBJS)
	g++ $(LIBS) -o $@ $^ 

%.o : %.cpp
	g++ $(CFLAGS) -o $@ -c $<
	
main.o : ClockDaemon.h
ClockDaemon.o : BMP085.h LCD4bit.h
LCD4bit.o : LCD4bit.h
BMP085.o : BMP085.h

clean:
	rm *.o $(TARGET)

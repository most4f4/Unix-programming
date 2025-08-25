CC = g++
CFLAGS = -I
CFLAGS += -Wall
CFLAGS += -std=c++17
FILES = Logger.cpp Automobile.cpp TravelSimulator.cpp
FILES1 = LogServer.cpp
LIBS = -lpthread

all: travel server

server: $(FILES1)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

travel: $(FILES)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f *.o travel server


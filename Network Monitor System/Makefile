CC=g++
CFLAGS=-I
CFLAGS+=-Wall
FILES1=interfaceMonitor.cpp
FILES2=networkMonitor.cpp

all: interfaceMonitor networkMonitor

interfaceMonitor: $(FILES1)
	$(CC) $(CFLAGS) -o interfaceMonitor $(FILES1)

networkMonitor: $(FILES2)
	$(CC) $(CFLAGS) -o networkMonitor $(FILES2)

debug: CFLAGS += -DDEBUG
debug: clean all

clean:
	rm -f *.o interfaceMonitor networkMonitor

.PHONY: all debug clean

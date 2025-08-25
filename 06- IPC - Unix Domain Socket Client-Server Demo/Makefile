CC=gcc
CFLAGS+=-Wall
FILES1=client.c
FILES2=server.c

all: client server

client: $(FILES1)
	$(CC) $(CFLAGS) -o client $(FILES1)

server: $(FILES2)
	$(CC) $(CFLAGS) -o server $(FILES2)

clean:
	rm -f *.o client server



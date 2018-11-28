CC:= gcc
CFLAGS:= -Wall -O2 
OBJS:= server client

all: $(OBJS)

server: server.c head.h
	$(CC) $(CFLAGS) -o $@ $<
client:client.c head.h
	$(CC) $(CFLAGS) -o $@ $<

.PHONY:clean

clean:
	-rm $(OBJS) 

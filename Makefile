#Executable

ifeq ($(CC),)
	CC = $(CROSS_COMPILE)gcc
endif

ifeq ($(CFLAGS),)
	CFLAGS = -g -Wall -Werror
endif

ifeq ($(LDFLAGS),)
	LDFLAGS = -pthread -lrt
endif

all: helloworld sharedmem server

helloworld: HelloWorld/helloworld.c
	$(CC) $(CFLAGS) $(INCLUDES) HelloWorld/helloworld.c -o helloworld

sharedmem: SharedMemory/sharedmem.c
	$(CC) $(CFLAGS) $(INCLUDES) SharedMemory/sharedmem.c -o sharedmem $(LDFLAGS)

server: SocketServer/server.c
	$(CC) $(CFLAGS) SocketServer/server.c -o server $(LDFLAGS)

#make clean
clean:
	rm -rf helloworld sharedmem server

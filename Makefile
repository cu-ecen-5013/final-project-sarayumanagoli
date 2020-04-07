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

all: helloworld

helloworld: HelloWorld/helloworld.c
	$(CC) $(CFLAGS) HelloWorld/helloworld.c -o helloworld $(LDFLAGS)

#make clean
clean:
	rm -rf helloworld

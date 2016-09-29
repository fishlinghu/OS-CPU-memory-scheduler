CC = g++
CFLAGS = -g -Wall
LDFLAGS = -lvirt

all: memory_coordinator

memory_coordinator: memory_coordinator.cc
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)
clean:
	rm -f memory_coordinator
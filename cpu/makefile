CC = g++
CFLAGS = -g -Wall
LDFLAGS = -lvirt

all: vcpu_scheduler

vcpu_scheduler: vcpu_scheduler.cc
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)
clean:
	rm -f vcpu_scheduler
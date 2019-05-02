CC      = gcc
#CFLAGS  = -g -Wall
CFLAGS  = -O3 -fomit-frame-pointer
CFLAGS += $(shell pkg-config --cflags glib-2.0)
LIBS    = $(shell pkg-config --libs glib-2.0)

PLOTS   = $(shell find -name '*.plot')
FIGS    = $(PLOTS:%.plot=%.eps)

all: stack_dist

list.o: list.c list.h

stack_dist: stack_dist.o list.o
	$(CC) $(CFLAGS) -o $@ $+ $(LIBS)

gnuplots: $(FIGS)

%.eps: %.plot
	gnuplot $+

clean:
	rm -f stack_dist *.o

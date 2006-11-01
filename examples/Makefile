#!/bin/make

CC=$(shell which gcc32)
ifeq ($(CC),)
CC=gcc
endif

SOURCES=$(wildcard *.c)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))
TARGETS=$(patsubst %.o,%.exe,$(OBJECTS))
CFLAGS=-I../includes -ggdb -pipe -Wall -pedantic

all: $(TARGETS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%.exe: %.o
	$(CC) -o $@ $^ -L.. -lgrf

clean:
	$(RM) $(TARGETS) $(OBJECTS)


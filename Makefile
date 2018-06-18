TARGET = prog
LIBS = -lm -lpfm
CC = gcc
CFLAGS = -g -Wall

.PHONY: clean all default

PROGRAM_LIST = \
	gen_codes \
	nameevents \
	PAPI_sample \
	gen_defines \

default: $(PROGRAM_LIST)
all: default

$(PROGRAM_LIST): %: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

clean:
	-rm -f *.o
	-rm -f $(PROGRAM_LIST)

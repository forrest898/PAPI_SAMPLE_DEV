TARGET = prog
LIBS = -lm -lpfm
CC = gcc
CFLAGS = -g -Wall

.PHONY: clean all default

PROGRAM_LIST = \
	char_replace \
	gen_codes \
	nameevents \
	PAPI_sample \
	gen_defines \

all: gen_codes nameevents gen_defines

gen_codes:	gen_codes.o char_replace.o
			$(CC) -o gen_codes gen_codes.o char_replace.o \
			$(LIBS)

nameevents: nameevents.o
			$(CC) -o nameevents nameevents.o \
			$(LIBS)

gen_defines:	gen_defines.o
				$(CC) -o gen_defines gen_defines.o char_replace.o \
				$(LIBS)

#$(PROGRAM_LIST): %: %.c
#	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

char_replace.o: char_replace.c char_replace.h
	$(CC) $(CFLAGS) -c char_replace.c

gen_codes.o: gen_codes.c
	$(CC) $(CFLAGS) -c gen_codes.c

nameevents.o: nameevents.c
	$(CC) $(CFLAGS) -c nameevents.c

PAPI_sample.o: PAPI_sample.c PAPI_sample.h
	$(CC) $(CFLAGS) -c PAPI_sample.c

gen_defines.o: gen_defines.c
	$(CC) $(CFLAGS) -c gen_defines.c

clean:
	-rm -f *.o
	-rm -f $(PROGRAM_LIST)

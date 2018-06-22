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

all: gen_codes nameevents gen_defines gen_list test_events

gen_codes:	gen_codes.o char_replace.o
			$(CC) -o gen_codes gen_codes.o char_replace.o \
			$(LIBS)

nameevents: nameevents.o
			$(CC) -o nameevents nameevents.o \
			$(LIBS)

gen_defines:	gen_defines.o
				$(CC) -o gen_defines gen_defines.o char_replace.o \
				$(LIBS)

gen_list:	gen_list.o
			$(CC) -o gen_list gen_list.o char_replace.o \
			$(LIBS)

test_events:	test_events.o PAPI_sample.o instructions_testcode.o \
				perf_helpers.o test_utils.o parse_record.o \
				matrix_multiply.o
				$(CC) -o test_events test_events.o PAPI_sample.o \
				instructions_testcode.o perf_helpers.o test_utils.o \
				parse_record.o matrix_multiply.o

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

gen_list.o: gen_list.c
	$(CC) $(CFLAGS) -c gen_list.c

test_events.o: test_events.c
	$(CC) $(CFLAGS) -c test_events.c

instructions_testcode.o: instructions_testcode.c instructions_testcode.h
	$(CC) $(CFLAGS) -c instructions_testcode.c

perf_helpers.o: perf_helpers.c perf_helpers.h
	$(CC) $(CFLAGS) -c perf_helpers.c

test_utils.o: test_utils.c test_utils.h
	$(CC) $(CFLAGS) -c test_utils.c

parse_record.o: parse_record.c parse_record.h
	$(CC) $(CFLAGS) -c parse_record.c

matrix_multiply.o: matrix_multiply.c matrix_multiply.h
	$(CC) $(CFLAGS) -c matrix_multiply.c



clean:
	-rm -f *.o
	-rm -f $(PROGRAM_LIST)
	-rm -f wowie*

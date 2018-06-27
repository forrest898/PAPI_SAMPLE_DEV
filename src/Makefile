include ../Makefile.globals.inc

TARGET = prog
LIBS = -lm -lpfm
CFLAGS += -g -Wall -I../include

.PHONY: clean all default

PROGRAM_LIST = \
	char_replace \
	gen_codes \
	nameevents \
	PAPI_sample \
	gen_defines \

all: gen_codes nameevents gen_defines gen_list test_events matrix_mul

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
				parse_record.o matrix_multiply.o $(LIBS)

matrix_mul:	matrix_mul.o
			$(CC) -o matrix_mul matrix_mul.o matrix_multiply.o $(LIBS)

#$(PROGRAM_LIST): %: %.c
#	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

char_replace.o: char_replace.c
	$(CC) $(CFLAGS) -c char_replace.c

gen_codes.o: gen_codes.c
	$(CC) $(CFLAGS) -c gen_codes.c

nameevents.o: nameevents.c
	$(CC) $(CFLAGS) -c nameevents.c

PAPI_sample.o: PAPI_sample.c
	$(CC) $(CFLAGS) -c PAPI_sample.c

gen_defines.o: gen_defines.c
	$(CC) $(CFLAGS) -c gen_defines.c

gen_list.o: gen_list.c
	$(CC) $(CFLAGS) -c gen_list.c

test_events.o: test_events.c
	$(CC) $(CFLAGS) -c test_events.c

instructions_testcode.o: instructions_testcode.c
	$(CC) $(CFLAGS) -c instructions_testcode.c

perf_helpers.o: perf_helpers.c
	$(CC) $(CFLAGS) -c perf_helpers.c

test_utils.o: test_utils.c
	$(CC) $(CFLAGS) -c test_utils.c

parse_record.o: parse_record.c
	$(CC) $(CFLAGS) -c parse_record.c

matrix_multiply.o: matrix_multiply.c
	$(CC) $(CFLAGS) -c matrix_multiply.c

matrix_mul.o: matrix_mul.c
	$(CC) $(CFLAGS) -c matrix_mul.c

clean: clean-local
	@- $(RM) *.o
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) wowie*

clean-local:
		@- $(RM) *~

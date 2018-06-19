#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <errno.h>

#include <signal.h>
#include <sys/mman.h>

#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <sys/prctl.h>
#include <err.h>
#include <perfmon/pfmlib.h>
#include <inttypes.h>
#include "PAPI_sample.h"
#include "instructions_testcode.h"
#include "perf_event.h"
#include "perf_helpers.h"
#include <papi.h>

int sample_type=PERF_SAMPLE_IP | PERF_SAMPLE_TID | PERF_SAMPLE_TIME |
		PERF_SAMPLE_ADDR | PERF_SAMPLE_READ | PERF_SAMPLE_CALLCHAIN |
		PERF_SAMPLE_ID | PERF_SAMPLE_CPU | PERF_SAMPLE_PERIOD |
		PERF_SAMPLE_STREAM_ID | PERF_SAMPLE_RAW |
		PERF_SAMPLE_BRANCH_STACK | PERF_SAMPLE_DATA_SRC;


int main(int argc, char** argv) {

	int ret;
	int *ev;
	*ev = PAPI_INST_RETIRED_TOTAL_CYCLES;

	ret = PAPI_sample_init(1, ev, 1, sample_type, 100000, 'A');
	if(ret != PAPI_OK) {
		printf("PANIC\n");
		exit(1);
	}

	PAPI_sample_start(1);

	PAPI_sample_stop(1);

	return 0;

}

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
#include <inttypes.h>
#include "PAPI_sample.h"
#include "instructions_testcode.h"
#include <perfmon/pfmlib.h>
#include <perfmon/perf_event.h>
#include <perfmon/pfmlib_perf_event.h>
#include "perf_helpers.h"
#include <papi.h>
#include "matrix_multiply.h"

int sample_type=PERF_SAMPLE_IP | PERF_SAMPLE_TID | PERF_SAMPLE_TIME |
		PERF_SAMPLE_ADDR | PERF_SAMPLE_READ | PERF_SAMPLE_CALLCHAIN |
		PERF_SAMPLE_ID | PERF_SAMPLE_CPU | PERF_SAMPLE_PERIOD |
		PERF_SAMPLE_STREAM_ID | PERF_SAMPLE_RAW |
		PERF_SAMPLE_DATA_SRC;


int main(int argc, char** argv) {

	int ret, conv, i;
	char *ev;
	char *filename2 = "wowie";
	PAPI_hw_info_t* hwinfo;
	// /FILE *fp;

	if(argc != 3) {
		printf("Please pass the req'd args\n");
		exit(1);
	}

	size_t len = strlen(filename2);
	size_t len2 = strlen(argv[1]);

	char* filename = malloc(len + len2 + 1 ); /* one for extra char, one for trailing zero */
    strcpy(filename, filename2);
	if(len2 == 1) {
    	filename[len] = argv[1][0];
    	filename[len + 1] = '\0';
	}
	else {
		filename = malloc(len + 1 + 1 ); /* one for extra char, one for trailing zero */
    	strcpy(filename, filename2);
    	filename[len] = argv[1][0];
		filename[len +1] = argv[1][1];
    	filename[len + 2] = '\0';
	}

	//convert argument to integer because the sampling events are defined
	// as 1-50 for right now
	//conv = atoi(argv[1]);
	//printf("%s\n", argv[2]);
	PAPI_library_init(PAPI_VER_CURRENT);
 /*
const PAPI_hw_info_t *
PAPI_get_hardware_info( void )
	printf("%s\n", PAPI_hw_info_t.model_string);

*/
	//papi_events_table[1];
	//argv[1]

	/* 	TODO:
		Use the hardware info and the INTEL manual to sort processor by
		architecture. Then set up a translator from a PAPI standard defined
		event to the INTEL name so that libpfm4 can be used as it is in
		PAPI_sample.

		After that, need to work out a way to get all 50 PEBS events recording
		samples for Skylake	(and then other arch's)
	*/

	//printf("0x%x\n", PAPI_L3_LDM);


	//hwinfo = PAPI_get_hardware_info();
	//printf("0x%x\n", hwinfo->cpuid_model);

	ev = argv[2];
	printf("%s\n", ev);

	//set the sampling event
	//*ev = conv;
	//ev[1] = 36;

	// initialize sampling
	ret = PAPI_sample_init(1, ev, 1, sample_type, 100000, filename);
	if(ret != PAPI_OK) {
		printf("PANIC\n");
		exit(1);
	}

	ret = PAPI_sample_start(1);
	if(ret != PAPI_OK) {
		printf("PANIC\n");
		exit(1);
	}

	for(i = 0; i < 3; i++) {
		naive_matrix_multiply(0);
	}

	//printf("Yo yo yo \n");

	ret = PAPI_sample_stop(1, 1);
	if(ret != PAPI_OK) {
		printf("PANIC\n");
		exit(1);
	}

	//free(ev);
	return 0;

}

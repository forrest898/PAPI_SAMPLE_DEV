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
#include <sys/sysinfo.h>
#include "matrix_multiply.h"

#define MMAP_DATA_SIZE 8
#define NUM_PROCS get_nprocs()

// s/truct mmap_info events[24];



int sample_type=PERF_SAMPLE_IP | PERF_SAMPLE_TID | PERF_SAMPLE_TIME |
		PERF_SAMPLE_ADDR | PERF_SAMPLE_READ | PERF_SAMPLE_CALLCHAIN |
		PERF_SAMPLE_ID | PERF_SAMPLE_CPU | PERF_SAMPLE_PERIOD |
		PERF_SAMPLE_STREAM_ID | PERF_SAMPLE_RAW |
		PERF_SAMPLE_DATA_SRC ;


int main(int argc, char** argv) {

	int ret, conv, i;
	int * fds;
	char *ev;
	char *filename2 = "wowie";
	char *br = "BR_";
	FILE* log;
	//void **mmaps;
	int mmap_pages = MMAP_DATA_SIZE + 1;
	PAPI_hw_info_t* hwinfo;

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

	PAPI_library_init(PAPI_VER_CURRENT);

	/*

	log = fopen("logging", "w");
	fclose(log);
	log = fopen("logging","a");

	*/
	/* 	TODO:
		Use the hardware info and the INTEL manual to sort processor by
		architecture. Then set up a translator from a PAPI standard defined
		event to the INTEL name so that libpfm4 can be used as it is in
		PAPI_sample.

		After that, need to work out a way to get all 50 PEBS events recording
		samples for Skylake	(and then other arch's)

		consider replacing the use of PAPI_get_hardware_info with Vince's
		processor detect function.
	*/

	//printf("0x%x\n", PAPI_L3_LDM);


	//hwinfo = PAPI_get_hardware_info();
	//printf("0x%x\n", hwinfo->cpuid_model);

	ev = argv[2];
	printf("%s\n", ev);

	if(strstr(argv[2], br) != NULL) {
		//printf("someone's looking fabulous\n");
		sample_type |= PERF_SAMPLE_BRANCH_STACK;
	}

	//set the sampling event
	//*ev = conv;
	//ev[1] = 36;

	// initialize sampling
	fds = PAPI_sample_init(1, ev, 1, sample_type, 10000, filename);
	if(ret != PAPI_OK) {
		printf("PANIC1\n");
		//exit(1);
	}
/*
	for(i = 0; i < NUM_PROCS; i++) {
		printf("i is %d, event location is %p and mmap location is %p\n",
 					i, events[i], events[i].sample_mmap);
	}

	for(i = 0; i < NUM_PROCS; i++) {

		printf("SIZE MY HOMBRE 0x%x", mmap_pages*getpagesize());
		events[i].sample_mmap=mmap(NULL, mmap_pages*getpagesize(),
				PROT_READ|PROT_WRITE, MAP_SHARED, fds[i], 0);

		printf("i is %d, event location is %p and mmap location is %p\n",
 					i, events[i], events[i].sample_mmap);

		fcntl(fds[i], F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
		fcntl(fds[i], F_SETSIG, SIGIO);
		fcntl(fds[i], F_SETOWN,getpid());

	}*/

//	if(events[0].sample_mmap == NULL)
//		printf("everything is broken\n");

	ret = PAPI_sample_start(fds);
	if(ret != PAPI_OK) {
		printf("PANIC2\n");
		//exit(1);
	}

	for(i = 0; i < 5; i++) {
		naive_matrix_multiply(1);
	}

	ret = PAPI_sample_stop(fds, 1);
	if(ret != PAPI_OK) {
		printf("PANIC3\n");
		exit(1);
	}

	//fclose(log);
	return 0;

}

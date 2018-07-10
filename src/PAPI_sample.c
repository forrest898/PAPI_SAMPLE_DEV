/*	This file represents my first attempts to develop a sampling interface for
 *	the PAPI library. It uses code largely written by Vincent Weaver (files like
 *	parse_record, test_utils, perf_helpers, and much of this file). The
 *	sampling interface takes advantage of the libpfm4 library; PAPI events are
 * 	translated to their corresponding Intel events. libpfm4 is then used to
 * 	set up perf_event_attr's for selected events. Sampling type and frequency
 *	are specified by the user as well as the filename to store the sampling
 *  results.



	TODO:If no filename is given, then the results are stored in the default
	file data.perf. If this file already exists, an error code will be returned.


 */


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
#include <papi.h>
#include <perfmon/pfmlib.h>
#include <perfmon/perf_event.h>
#include <perfmon/pfmlib_perf_event.h>
#include "test_utils.h"
#include "perf_helpers.h"

#include "parse_record.h"
#include "PAPI_sample.h"


#define MMAP_DATA_SIZE 8
#define DEBUG 0
#define AGGREGATE 0
//static int32_t init = 0;
static int32_t quiet=0;
static int* fds;

long long prev_head = 0;

char *output_file;
void *our_mmap;
//#define SAMPLE_FREQUENCY 100000

static void PAPI_sample_handler(int signum, siginfo_t *info, void *uc) {

	int ret;

	int fd = info->si_fd;
    int read_format = PERF_FORMAT_GROUP |
        PERF_FORMAT_ID |
        PERF_FORMAT_TOTAL_TIME_ENABLED |
        PERF_FORMAT_TOTAL_TIME_RUNNING;
	int sample_type=PERF_SAMPLE_IP | PERF_SAMPLE_TID | PERF_SAMPLE_TIME |
			PERF_SAMPLE_ADDR | PERF_SAMPLE_READ | PERF_SAMPLE_CALLCHAIN |
			PERF_SAMPLE_ID | PERF_SAMPLE_CPU | PERF_SAMPLE_PERIOD |
			PERF_SAMPLE_STREAM_ID | PERF_SAMPLE_RAW |
			PERF_SAMPLE_DATA_SRC;

	//printf("Handler!\n");
	/* Disable counters in order to perform MMAP read */
	ret=ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);




	/* Parse MMAP and read out our sampled values*/
	prev_head=perf_mmap_read(our_mmap,MMAP_DATA_SIZE,prev_head,
		sample_type,read_format,
		0, /* reg_mask */
		NULL, /*validate */
		quiet,
		NULL, /* events read */
		RAW_NONE,
		output_file);

	/* Re-enable counters */
	ret=ioctl(fd, PERF_EVENT_IOC_REFRESH, 1);

	(void) ret;

}

/* Base API for simple write-out results */
int PAPI_sample_init(int Eventset, char* EventCodes, int NumEvents,
                        int sample_type, int sample_period, char* filename) {

    int i, firstEvent, ret;
    int mmap_pages=1+MMAP_DATA_SIZE;
    int quiet = 0;

    struct perf_event_attr pe;
    struct sigaction sa;
    char test_string[]="Testing Intel PEBS support...";

	/* Open and clear contents of file to record the sampling results */
	FILE* fp = fopen(filename, "w");
	fclose(fp);

	/* Set global variable to be used by our signal handler */
	output_file = filename;

	/* Allocate as many file descriptors as events sampled */
    fds = (int *)malloc(sizeof(int)*NumEvents);

	ret = pfm_initialize();
    if (ret != PFM_SUCCESS)
        fprintf(stdout, "cannot initialize library %s", pfm_strerror(ret));


    firstEvent = 1;

	/* Set up PAPI_sample_handler to catch the interrupt created when a counter
		hits the user specified value */
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_sigaction = PAPI_sample_handler;
    sa.sa_flags = SA_SIGINFO;

    if (sigaction( SIGIO, &sa, NULL) < 0) {
        fprintf(stderr,"Error setting up signal handler\n");
        exit(1);
    }

	/*	Setup perf_event_attr structures for each event and then
		open a file descriptor for each */
    for(i = 0; i < NumEvents; i++) {

		//TODO: before each event is processed into a pref_event_attr structure
		// the PAPI version of the event must be translate to the string
		// for the corresponding architecture in order to call libpfm4

		memset(&pe,0,sizeof(struct perf_event_attr));
       //pe = setup_perf(EventCodes[i], sample_type, sample_period, firstEvent);
		pe = new_setup_perf(EventCodes, sample_type, sample_period, firstEvent);

		/* 	For the first event the fourth arg to perf_event_open is -1
			For subsequent events, the group_fd (first evend's fd) is used
			for the fourth argument to link the events together */
		if(firstEvent) {

			if(DEBUG) {
				printf("Value of i is %d\n \
						Eventcode is 0x%x\n", i, pe.config);
			}
            fds[0] = perf_event_open(&pe,0,-1,-1,0);
            if (fds[0] < 0) {
	    		if (!quiet) {
					fprintf(stderr,"Problem opening leader %s\n",
					strerror(errno));
					fprintf(stderr,"Trying without branches\n");
				}
				sample_type&=~PERF_SAMPLE_BRANCH_STACK;
				pe.sample_type=sample_type;
				fds[0]=perf_event_open(&pe,0,-1,-1,0);
				if (fds[0]<0) {
					if (!quiet) {
						fprintf(stderr,"Error opening leader %s\n",
							strerror(errno));
					}
					test_fail(test_string);
				}
    			//test_fail(test_string);
    		}
        }
        else {
            fds[i]=perf_event_open(&pe,0,-1,fds[0],0);
        	if (fds[i]<0) {
        		if (!quiet) fprintf(stderr,"Error opening %llx\n",pe.config);
        		test_fail(test_string);
        	}

        }

		/* 	Ensure only the first event uses -1 for the fourth arg to
			perf_event_open */
        if(i == 0)
            firstEvent = 0;
    }


	our_mmap=mmap(NULL, mmap_pages*getpagesize(),
			PROT_READ|PROT_WRITE, MAP_SHARED, fds[0], 0);

	/* SIGIO must be asynchronous because perf will write to the mmap and continue
	to count simultaneously */
	fcntl(fds[0], F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	/* Associates our file descriptor with the appropriate signal */
	fcntl(fds[0], F_SETSIG, SIGIO);
	fcntl(fds[0], F_SETOWN,getpid());

    return PAPI_OK;


}

/* Function to call the ioctl's which will start the sampling process */
int PAPI_sample_start(int Eventset) {

    int ret;

    if(fds[0] != NULL) {

    	ioctl(fds[0], PERF_EVENT_IOC_RESET, 0);

    	ret=ioctl(fds[0], PERF_EVENT_IOC_ENABLE,0);

    	if (ret<0) {
    		if (!quiet) {
    			fprintf(stderr,"Error with PERF_EVENT_IOC_ENABLE "
    					"of group leader: %d %s\n",
    					errno,strerror(errno));
    			exit(1);
     		}
    	}

    }


    return PAPI_OK;

}

/* Function to call the ioctl's to stop sampling and perform memory cleanup */
int PAPI_sample_stop(int Eventset, int NumEvents) {

    int ret, i, count;

	ret=ioctl(fds[0], PERF_EVENT_IOC_REFRESH,0);
    printf("File ready for parsing\n");

 	ret=ioctl(fds[0], PERF_EVENT_IOC_DISABLE, 0);

	if(AGGREGATE) {
		long long meow;
		read(fds[0], &meow, sizeof(long long));
		printf("Event count: %lld\n", meow);
	}
	/* Close the perf_event_open fd's */
	for(i=(NumEvents-1); i >= 0; i--) {
		//printf("Closing fds[%d]\n", i);
		close(fds[i]);
	}

	/* Unmap the MMAP */
	munmap(our_mmap, 1+MMAP_DATA_SIZE*getpagesize());

	/* Free perf_event_open FD's */
	free(fds);

    return PAPI_OK;

}

/*	Function which returns a perf_event_attr that is generated via a call
	to libpfm */
struct perf_event_attr new_setup_perf(char* EventCode, int sample_type,
                                    int sample_period, int firstEvent)  {

	int ret;
	int read_format = PERF_FORMAT_GROUP |
        PERF_FORMAT_ID |
        PERF_FORMAT_TOTAL_TIME_ENABLED |
        PERF_FORMAT_TOTAL_TIME_RUNNING;

	pfm_perf_encode_arg_t raw;
	struct perf_event_attr attr;

	memset(&raw, 0, sizeof(raw));
	memset(&attr, 0, sizeof(attr));

 	raw.size = sizeof(raw);
	/* 	If raw.attr is not defined pfm_get_os_event_encoding will throw an
		error */
 	raw.attr = &attr;

	/* Sets up the perf_event_attr */
    ret = pfm_get_os_event_encoding(EventCode, PFM_PLM3, PFM_OS_PERF_EVENT, &raw);

	if (ret != PFM_SUCCESS) {
		printf("gen_codes can't get encoding %s\n",  pfm_strerror(ret));
		exit(1);
	}

	/* 	Make sure to set the proper sampling paramters for the first event to
		ensure interrupts will be generated on the sample_period */
	if(firstEvent) {
		if(AGGREGATE);
		else {
        	attr.sample_period=sample_period;
			attr.sample_type=sample_type;
			attr.read_format=read_format;
		}
	    attr.disabled=1;
		attr.wakeup_events=1;
	    attr.pinned=1;
		attr.precise_ip=0;
		//attr.inherit=1;
    }
	/* 	Setting disabled=0 for subsequent events will *NOT* cause them to start
 		counting until the group fd has started counting */
	else {
	    attr.pinned=1;
		attr.sample_type=PERF_SAMPLE_READ;
		attr.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
	    attr.disabled=0;
	}

	if(sample_type & PERF_SAMPLE_BRANCH_STACK) {
		attr.branch_sample_type=PERF_SAMPLE_BRANCH_ANY;
	}


	return attr;

}

/* 	Old funcition to generate perf_event_attr setup. Hard coded with Skylake
	events and #defines found in PAPI_sample.h */
struct perf_event_attr setup_perf(int EventCode, int sample_type,
                                    int sample_period, int firstEvent) {

    int read_format = PERF_FORMAT_GROUP |
        PERF_FORMAT_ID |
        PERF_FORMAT_TOTAL_TIME_ENABLED |
        PERF_FORMAT_TOTAL_TIME_RUNNING;
    struct perf_event_attr pe;

    memset(&pe,0,sizeof(struct perf_event_attr));

    pe.type=PERF_TYPE_RAW;
    pe.size=sizeof(struct perf_event_attr);

    if(firstEvent) {

        pe.sample_period=sample_period;
		pe.sample_type=sample_type;
		pe.read_format=read_format;
	    pe.disabled=1;
		pe.wakeup_events=1;
	    pe.pinned=1;
    }
	else {
		pe.sample_type=PERF_SAMPLE_RAW;
		pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
	    pe.disabled=0;
	}

    pe.exclude_kernel=1;
    pe.exclude_hv=1;

    //pe.precise_ip=1;

  /* Prototype for Skylake machines */

    switch(EventCode) {
        case	PAPI_BR_INST_RETIRED_ALL_BRANCHES	:
			pe.config=0x5100c4;
			break;
		case	PAPI_BR_INST_RETIRED_CONDITIONAL	:
			pe.config=0x5101c4;
			break;
		case	PAPI_BR_INST_RETIRED_FAR_BRANCH	:
			pe.config=0x5140c4;
			break;
		case	PAPI_BR_INST_RETIRED_NEAR_CALL	:
			pe.config=0x5102c4;
			break;
		case	PAPI_BR_INST_RETIRED_NEAR_RETURN	:
			pe.config=0x5108c4;
			break;
		case	PAPI_BR_INST_RETIRED_NEAR_TAKEN	:
			pe.config=0x5120c4;
			break;
		case	PAPI_BR_MISP_RETIRED_ALL_BRANCHES	:
			pe.config=0x5100c5;
			break;
		case	PAPI_BR_MISP_RETIRED_CONDITIONAL	:
			pe.config=0x5101c5;
			break;
		case	PAPI_BR_MISP_RETIRED_NEAR_CALL	:
			pe.config=0x5102c5;
			break;
		case	PAPI_BR_MISP_RETIRED_NEAR_TAKEN	:
			pe.config=0x5120c5;
			break;
		case	PAPI_FRONTEND_RETIRED_DSB_MISS	:
			pe.config=0x5101c6;
			pe.config1=0x11;
			pe.precise_ip=0;
			break;
		case	PAPI_FRONTEND_RETIRED_ITLB_MISS	:
			pe.config=0x5101c6;
			pe.config1=0x14;
			pe.precise_ip=0;
			break;
		case	PAPI_FRONTEND_RETIRED_STLB_MISS	:
			pe.config=0x5101c6;
			pe.config1=0x15;
			pe.precise_ip=0;
			break;
		case	PAPI_FRONTEND_RETIRED_L1I_MISS	:
			//pe.precise_ip=1;
			pe.config=0x5101c6;
			pe.config1=0x12;
			pe.precise_ip=0;
			break;
		case	PAPI_FRONTEND_RETIRED_L2_MISS	:
			pe.config=0x5101c6;
			pe.config1=0x13;
			pe.precise_ip=0;
			break;
		case 	PAPI_FRONTEND_RETIRED_LATENCY_GE_128:
			pe.config=0x5101c6;
			pe.config2=0x408006;
			break;
		case	PAPI_MEM_INST_RETIRED_STLB_MISS_LOADS	:
			pe.config=0x5111d0;
			pe.precise_ip=0;
			break;
		case	PAPI_MEM_INST_RETIRED_STLB_MISS_STORES	:
			pe.config=0x5112d0;
			break;
		case	PAPI_MEM_INST_RETIRED_LOCK_LOADS	:
			pe.config=0x5121d0;
			break;
		case	PAPI_MEM_INST_RETIRED_SPLIT_LOADS	:
			pe.config=0x5141d0;
			break;
		case	PAPI_MEM_INST_RETIRED_SPLIT_STORES	:
			pe.config=0x5142d0;
			break;
		case	PAPI_MEM_INST_RETIRED_ALL_LOADS	:
			pe.config=0x5181d0;
			break;
		case	PAPI_MEM_INST_RETIRED_ALL_STORES	:
			pe.config=0x5182d0;
			break;
		case	PAPI_HLE_RETIRED_ABORTED	:
			pe.config=0x5104c8;
			break;
		case	PAPI_INST_RETIRED_TOTAL_CYCLES	:
			pe.config=0xad101c0;
			break;
		case	PAPI_INST_RETIRED_TOTAL_INST	:
			pe.config=0x5100c0;
			break;
		case	PAPI_MEM_LOAD_L3_HIT_RETIRED_XSNP_HIT	:
			pe.config=0x5102d2;
			break;
		case	PAPI_MEM_LOAD_L3_HIT_RETIRED_XSNP_HITM	:
			pe.config=0x5104d2;
			break;
		case	PAPI_MEM_LOAD_L3_HIT_RETIRED_XSNP_MISS	:
			pe.config=0x5101d2;
			break;
		case	PAPI_MEM_LOAD_L3_HIT_RETIRED_XSNP_NONE	:
			pe.config=0x5108d2;
			break;
		case	PAPI_MEM_LOAD_MISC_RETIRED_UC	:
			pe.config=0x5104d4;
			break;
		case	PAPI_MEM_LOAD_RETIRED_FB_HIT	:
			pe.config=0x5140d1;
			break;
		case	PAPI_MEM_LOAD_RETIRED_L1_HIT	:
			pe.config=0x5101d1;
			break;
		case	PAPI_MEM_LOAD_RETIRED_L1_MISS	:
			pe.config=0x5108d1;
			break;
		case	PAPI_MEM_LOAD_RETIRED_L2_HIT	:
			pe.config=0x5102d1;
			break;
		case	PAPI_MEM_LOAD_RETIRED_L2_MISS	:
			pe.config=0x5110d1;
			break;
		case	PAPI_MEM_LOAD_RETIRED_L3_HIT	:
			pe.config=0x5104d1;
			break;
		case	PAPI_MEM_LOAD_RETIRED_L3_MISS	:
			pe.config=0x5120d1;
			break;
		case	PAPI_RTM_RETIRED_ABORTED	:
			pe.config=0x5104c9;
			//pe.precise_ip=1;
			break;

        default:
            printf("EventCode not found in PEBS/IBS event! Enter a valid code!\n");
            //return -1;
            break;
    }

    return pe;
}

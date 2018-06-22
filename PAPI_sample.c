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

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
//#include "instructions_testcode.h"

#include "parse_record.h"
#include "PAPI_sample.h"


#define MMAP_DATA_SIZE 8
#define DEBUG 1

//static int32_t init = 0;
static int32_t quiet=0;
static int* fds;

long long prev_head = 0;

void *our_mmap;
//#define SAMPLE_FREQUENCY 100000

static void PAPI_sample_handler(int signum, siginfo_t *info, void *uc) {

	int ret;

	int fd = info->si_fd;
    int read_format = PERF_FORMAT_GROUP |
        PERF_FORMAT_ID |
        PERF_FORMAT_TOTAL_TIME_ENABLED |
        PERF_FORMAT_TOTAL_TIME_RUNNING;

	ret=ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);

	int sample_type=PERF_SAMPLE_IP | PERF_SAMPLE_TID | PERF_SAMPLE_TIME |
			PERF_SAMPLE_ADDR | PERF_SAMPLE_READ | PERF_SAMPLE_CALLCHAIN |
			PERF_SAMPLE_ID | PERF_SAMPLE_CPU | PERF_SAMPLE_PERIOD |
			PERF_SAMPLE_STREAM_ID | PERF_SAMPLE_RAW |
			PERF_SAMPLE_DATA_SRC;


	prev_head=perf_mmap_read(our_mmap,MMAP_DATA_SIZE,prev_head,
		sample_type,read_format,
		0, /* reg_mask */
		NULL, /*validate */
		quiet,
		NULL, /* events read */
		RAW_NONE);
//    printf("I got interrupted\n");
	//count_total++;

	ret=ioctl(fd, PERF_EVENT_IOC_REFRESH, 1);

	(void) ret;

}

/* Base API for simple write-out results */
int PAPI_sample_init(int Eventset, int* EventCodes, int NumEvents,
                        int sample_type, int sample_period, char filename) {

    int ret, i, firstEvent;
    //int* fds;
	int fd1;
    int mmap_pages=1+MMAP_DATA_SIZE;
    int quiet = 0;
    int read_format;

    struct perf_event_attr pe;
    struct sigaction sa;
    char test_string[]="Testing Intel PEBS support...";

     //quiet=test_quiet();
    fds = (int *)malloc(sizeof(int)*NumEvents);
    firstEvent = 1;

    if(!quiet) printf("This begins the implementation of complex sampling with PAPI.\n");

    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_sigaction = PAPI_sample_handler;
    sa.sa_flags = SA_SIGINFO;

    if (sigaction( SIGIO, &sa, NULL) < 0) {
        fprintf(stderr,"Error setting up signal handler\n");
        exit(1);
    }

//	pe = setup_perf(EventCodes[0], sample_type, sample_period, firstEvent);
	/*
	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Problem opening leader %s\n",
			strerror(errno));
			fprintf(stderr,"Trying without branches\n");
		}
		sample_type&=~PERF_SAMPLE_BRANCH_STACK;
		pe.sample_type=sample_type;
		fd1=perf_event_open(&pe,0,-1,-1,0);
		if (fd1<0) {
			if (!quiet) {
				fprintf(stderr,"Error opening leader %s\n",
					strerror(errno));
			}
			test_fail(test_string);
		}
	}
*/


    for(i = 0; i < NumEvents; i++) {

        memset(&pe,0,sizeof(struct perf_event_attr));
        pe = setup_perf(EventCodes[i], sample_type, sample_period, firstEvent);

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


	ioctl(fds[0], PERF_EVENT_IOC_RESET, 0);

	ret=ioctl(fds[0], PERF_EVENT_IOC_ENABLE,0);

	//	instructions_million();
		//instructions_million();


    return PAPI_OK;

	//	instructions_million();
		//instructions_million();


}

int PAPI_sample_start(int Eventset) {

    /* check to see if eventset is not null and valid first */
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

		instructions_million();
		instructions_million();

    }
    return PAPI_OK;

}

int PAPI_sample_stop(int Eventset) {

    int ret;
	int sample_type=PERF_SAMPLE_IP | PERF_SAMPLE_TID | PERF_SAMPLE_TIME |
			PERF_SAMPLE_ADDR | PERF_SAMPLE_READ | PERF_SAMPLE_CALLCHAIN |
			PERF_SAMPLE_ID | PERF_SAMPLE_CPU | PERF_SAMPLE_PERIOD |
			PERF_SAMPLE_STREAM_ID | PERF_SAMPLE_RAW |
			PERF_SAMPLE_DATA_SRC;

    ret=ioctl(fds[0], PERF_EVENT_IOC_REFRESH,0);
    printf("File ready for parsing\n");

 int read_format = PERF_FORMAT_GROUP |
	 PERF_FORMAT_ID |
	 PERF_FORMAT_TOTAL_TIME_ENABLED |
	 PERF_FORMAT_TOTAL_TIME_RUNNING;

 ret=ioctl(fds[0], PERF_EVENT_IOC_DISABLE, 0);


// prev_head=perf_mmap_read(our_mmap,MMAP_DATA_SIZE,prev_head,
	// sample_type,read_format,
	 //0, /* reg_mask */
//		NULL, /*validate */
//		quiet,
//		NULL, /* events read */
//		RAW_NONE);

    return PAPI_OK;

}

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

    if(firstEvent != 0) {

        pe.sample_period=sample_period;

    }

    pe.sample_type=sample_type;

    pe.read_format=read_format;
    pe.disabled=1;
    pe.pinned=1;
    pe.exclude_kernel=1;
    pe.exclude_hv=1;
    pe.wakeup_events=1;
    pe.precise_ip=1;

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
			pe.config2=0x11;
			break;
		case	PAPI_FRONTEND_RETIRED_ITLB_MISS	:
			pe.config=0x5101c6;
			pe.config2=0x14;
			break;
		case	PAPI_FRONTEND_RETIRED_STLB_MISS	:
			pe.config=0x5101c6;
			pe.config2=0x15;
			break;
		case	PAPI_FRONTEND_RETIRED_L1I_MISS	:
			pe.config=0x5101c6;
			pe.config2=0x12;
			break;
		case	PAPI_FRONTEND_RETIRED_L2_MISS	:
			pe.config=0x5101c6;
			pe.config2=0x13;
			break;
		case	PAPI_MEM_INST_RETIRED_STLB_MISS_LOADS	:
			pe.config=0x5111d0;
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
			break;

        default:
            printf("EventCode not found in PEBS/IBS event! Enter a valid code!");
            //return -1;
            break;
    }

    return pe;
}

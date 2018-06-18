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

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
//#include "instructions_testcode.h"

#include "parse_record.h"

#define MMAP_DATA_SIZE 8
#define SAMPLE_FREQUENCY 100000

static void our_handler(int signum, siginfo_t *info, void *uc) {
    /*
	int ret;

	int fd = info->si_fd;

	ret=ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);

	prev_head=perf_mmap_read(our_mmap,MMAP_DATA_SIZE,prev_head,
		sample_type,read_format,
		0, /* reg_mask */
	//	NULL, /*validate */
	//	quiet,
	//	NULL, /* events read */
/*		RAW_NONE);

	count_total++;

	ret=ioctl(fd, PERF_EVENT_IOC_REFRESH, 1);

	(void) ret;
*/
}

/* Base API for simple write-out results */
int PAPI_sample_init( int Eventset,int EventCode, int sample_type,
   int sample_period, char filename) {

     int ret, i;
     int fd;
     int mmap_pages=1+MMAP_DATA_SIZE;
     int quiet = 0;
     int read_format;

     struct perf_event_attr pe;

     struct sigaction sa;
     char test_string[]="Testing Intel PEBS support...";

     //quiet=test_quiet();

     if(!quiet) printf("This begins the implementation of complex sampling with PAPI.\n");

    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_sigaction = our_handler;
    sa.sa_flags = SA_SIGINFO;

    if (sigaction( SIGIO, &sa, NULL) < 0) {
        fprintf(stderr,"Error setting up signal handler\n");
        exit(1);
    }

    memset(&pe,0,sizeof(struct perf_event_attr));
    read_format=PERF_FORMAT_GROUP |
        PERF_FORMAT_ID |
        PERF_FORMAT_TOTAL_TIME_ENABLED |
        PERF_FORMAT_TOTAL_TIME_RUNNING;

    pe.type=PERF_TYPE_RAW;
    pe.size=sizeof(struct perf_event_attr);
      //pe.config=PERF_COUNT_HW_INSTRUCTIONS;

 /* MEM_UOPS_RETIRED:ALL_STORES */
    //  pe.config = 0x5301c2;

    pe.sample_period=SAMPLE_FREQUENCY;
    pe.sample_type=sample_type;

    pe.read_format=read_format;
    pe.disabled=1;
    pe.pinned=1;
    pe.exclude_kernel=1;
    pe.exclude_hv=1;
    pe.wakeup_events=1;
    pe.precise_ip=1;

  /* Prototype for Skylake machines */
    /*
    switch(EventCode) {
        case PAPI_DSB_MISS :
            pe.config = 0x5301c2;
            break;
        case PAPI_L1INS_MISS :
            pe.config = 0x5301c2;
            break;
        case PAPI_L2INS_MISS :
            pe.config = 0x5301c2;
            break;
        case PAPI_ITLB_MISS :
            pe.config = 0x5301c2;
            break;
        case PAPI_ISTLB_MISS :
            pe.config = 0x5301c2;
            break;
        case PAPI_ISTLB_MISS_LOADS :
            pe.config = 0x5301c2;
            break;
        case PAPI_ISTLB_MISS_STORES :
            pe.config = 0x5301c2;
            break;
        case PAPI_LOCK_LOADS :
            pe.config = 0x5301c2;
            break;
        case PAPI_SPLIT_LOADS  :
            pe.config = 0x5301c2;
            break;
        case PAPI_SPLIT_STORES      :
            pe.config = 0x5301c2;
            break;
        case PAPI_ALL_LOADS         :
            pe.config = 0x5301c2;
            break;
        case PAPI_ALL_STORES        :
            pe.config = 0x5301c2;
            break;
        case default:
            printf("EventCode not found in PEBS/IBS event! Enter a
                valid code!");
            return -1;
            break;
  }
        */













}

int main(int argc, char** argv) {

}

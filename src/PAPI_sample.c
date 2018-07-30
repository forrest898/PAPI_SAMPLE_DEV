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

#include <sys/sysinfo.h>

#include "test_utils.h"
#include "perf_helpers.h"

#include "parse_record.h"
#include "PAPI_sample.h"
#include "char_replace.h"


#define MMAP_DATA_SIZE 8
#define DEBUG 0
#define AGGREGATE 0
#define NUM_PROCS get_nprocs()


//static int32_t init = 0;
static int32_t quiet=0;
static int * fds;
long long * heads;
unsigned char * data;

long long prev_head = 0;

char *output_file;
void *our_mmap;
FILE* fp;

mmap_info mmaps[100];
int num_maps = 0;

int read_format_handle = PERF_FORMAT_GROUP |
   	PERF_FORMAT_ID |
    PERF_FORMAT_TOTAL_TIME_ENABLED |
    PERF_FORMAT_TOTAL_TIME_RUNNING;
int sample_type_handle=PERF_SAMPLE_IP | PERF_SAMPLE_READ | PERF_SAMPLE_CPU;


//struct mmap_info mmaps[NUM_PROCS];

//#define SAMPLE_FREQUENCY 100000

static void PAPI_sample_handler(int signum, siginfo_t *info, void *uc) {

	int ret, i;

	int fd = info->si_fd;

	long long prev_head;

	//printf("Handler!\n");
	/* Disable counters in order to perform MMAP read */
	ret=ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
	//if(events[(fd-3)].sample_mmap == NULL) {	printf("SHIT\n");}
	//printf("Interupt with file handle %d\n", fd);

	for(i = 0; i < num_maps; i++) {
		if((*(mmaps[i].fd)) == fd) {
	//printf("i is %d\n", i);
			prev_head = *(mmaps[i].prev_head);
			our_mmap = mmaps[i].sample_mmap;
			break;
		}
		if(i == (num_maps -1))	{
			printf("I never found my mmap/fd ");
			exit(1);
		}

	}

	/* Parse MMAP and read out our sampled values*/
	prev_head=perf_mmap_read(our_mmap,MMAP_DATA_SIZE,prev_head,
		sample_type_handle,read_format_handle,
		0, /* reg_mask */
		NULL, /*validate */
		quiet,
		NULL, /* events read */
		RAW_NONE,
		fp,
		data);

	*(mmaps[i].prev_head) = prev_head;

	/* Re-enable counters */
	ret=ioctl(fd, PERF_EVENT_IOC_REFRESH, 1);

	(void) ret;

}

/* Base API for simple write-out results */
int * PAPI_sample_init(int Eventset, char* EventCodes, int NumEvents,
                        int sample_type, int sample_period, char* filename) {

    int i, firstEvent, ret;
    int mmap_pages=1+MMAP_DATA_SIZE;
    int quiet = 0;
	int NUM_CORES;
	//int * fds;


    struct perf_event_attr pe;
    struct sigaction sa;
    char test_string[]="Testing Intel PEBS support...";
	//struct mmap_info mmaps[16];
	long long bytesize;

	bytesize = MMAP_DATA_SIZE*getpagesize();
	//printf("Bytesize %lld\n", bytesize);

	data=malloc(bytesize);
	if (data==NULL) {
		printf("data was null somehow");
		return -1;
	}


	/* Open and clear contents of file to record the sampling results */
	fp = fopen(filename, "w");
	if(fp == NULL) {
		printf("Could not open file for logging\n");
		exit(1);
	}
	fclose(fp);
	fp = fopen(filename, "a");
	if(fp == NULL) {
		printf("Could not open file for logging\n");
		exit(1);
	}

	/* Set global variable to be used by our signal handler */
	output_file = filename;

	NUM_CORES = get_nprocs();
	if(!(NUM_CORES > 0))	{
		printf("PANIC: SYSTEM DOESNT KNOW ABOUT ITS OWN CPU\n");
		return -1;
	}

	/* Allocate as many file descriptors as events sampled */
    fds = (int *)malloc(sizeof(int)*NumEvents*NUM_CORES);
	heads = (long long *)calloc(NumEvents*NUM_CORES, sizeof(long long));

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
    for(i = 0; i < NUM_PROCS; i++) {

		//TODO: before each event is processed into a pref_event_attr structure
		// the PAPI version of the event must be translate to the string
		// for the corresponding architecture in order to call libpfm4

		memset(&pe,0,sizeof(struct perf_event_attr));
       //pe = setup_perf(EventCodes[i], sample_type, sample_period, firstEvent);
		pe = new_setup_perf(EventCodes, sample_type, sample_period, firstEvent);

			if(DEBUG) {
				printf("Value of i is %d\n \
						Eventcode is 0x%x\n", i, pe.config);
			}
            fds[i] = perf_event_open(&pe,0, i,-1,0);
            if (fds[i] < 0) {
	    		if (!quiet) {
					fprintf(stderr,"Problem opening leader %s\n",
					strerror(errno));
					fprintf(stderr,"Trying without branches\n");
				}
				sample_type&=~PERF_SAMPLE_BRANCH_STACK;
				pe.sample_type=sample_type;
				fds[i] =perf_event_open(&pe,0, i,-1,0);
				if (fds[i] <0) {
					if (!quiet) {
						fprintf(stderr,"Error opening leader %s\n",
							strerror(errno));
					}
					test_fail(test_string);
				}
    			//test_fail(test_string);
    		}


		/* 	Ensure only the first event uses -1 for the fourth arg to
			perf_event_open */
        //if(i == 0)
         //   firstEvent = 0;
		mmaps[i].sample_mmap = mmap(NULL, mmap_pages*getpagesize(),
									PROT_READ | PROT_WRITE, MAP_SHARED,
									fds[i], 0);
		mmaps[i].fd = &fds[i];
		mmaps[i].prev_head = &heads[i];
	//	printf("ADDR MAP %p ... VALUE FD %d ... ADDR HEAD %p\n",
 	//			mmaps[i].sample_mmap, *(mmaps[i].fd), mmaps[i].prev_head);
		num_maps++;
		/* SIGIO must be asynchronous because perf will write to the mmap and continue
		to count simultaneously */
		fcntl(fds[i], F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
		/* Associates our file descriptor with the appropriate signal */
		fcntl(fds[i], F_SETSIG, SIGIO);
		fcntl(fds[i], F_SETOWN,getpid());

    }

    return fds;
}

/* Function to call the ioctl's which will start the sampling process */
int PAPI_sample_start(int * fd) {

    int ret, i;

	for(i = 0; i < NUM_PROCS; i++) {

		//printf("FD value: %d ------- I value: %d\n", fd[i], i);

    	ioctl(fd[i], PERF_EVENT_IOC_RESET, 0);

    	ret=ioctl(fd[i], PERF_EVENT_IOC_ENABLE,0);

    	if (ret<0) {
    		if (!quiet) {
    			fprintf(stderr,"Error with PERF_EVENT_IOC_ENABLE "
    					"of group leader: %d %s\n",
    					errno,strerror(errno));
    			// /exit(1);
     		}
    	}

    }

    return PAPI_OK;

}

/* Function to call the ioctl's to stop sampling and perform memory cleanup */
int PAPI_sample_stop(int * fd, int NumEvents) {

    int ret, i, count;

	for(i = 0; i < NUM_PROCS; i++) {
		ret=ioctl(fd[i], PERF_EVENT_IOC_REFRESH,0);
    	//printf("File ready for parsing\n");

 		ret=ioctl(fd[i], PERF_EVENT_IOC_DISABLE, 0);
	}

	/*
	if(AGGREGATE) {
		long long meow;
		read(fds[0], &meow, sizeof(long long));
		printf("Event count: %lld\n", meow);
	}
	*/
	/* Close the perf_event_open fd's */
	for(i=0; i < NUM_PROCS; i++) {
		//printf("Closing fds[%d]\n", i);
		close(fd[i]);
	}

	/* Unmap the MMAP */
	for(i = 0; i < num_maps; i++) {
		munmap(mmaps[i].sample_mmap, 1+MMAP_DATA_SIZE*getpagesize());
		//free(mmaps[i].prev_head);
	}

	fclose(fp);
	/* Free perf_event_open FD's */
	free(fds);
	free(heads);
	free(data);

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

        printf("%s\n", EventCode);

        if(strcasestr( EventCode, "LATENCY") != NULL) {

            attr = capital_latency_event(EventCode, sample_type, read_format,
                            sample_period);
            return attr;
        }
        else {
            printf("libpfm can't get encoding for perf_event attribute for the event %s\n",  pfm_strerror(ret));
		    exit(1);
        }
	}

	/* 	Make sure to set the proper sampling paramters for the first event to
		ensure interrupts will be generated on the sample_period */
	if(firstEvent) {
		if(AGGREGATE);
		else {
        	attr.sample_period=sample_period;
			attr.sample_type=sample_type_handle;
			attr.read_format=read_format_handle;
		}
	    attr.disabled=1;
		attr.wakeup_events=1;
	    attr.pinned=1;
		attr.precise_ip=0;
		attr.exclude_kernel=1;
		attr.exclude_hv=1;
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

struct perf_event_attr lower_latency_event(char* EventCode, int sample_type,
            int read_format, int sample_period) {

    int i = 0;
    int colon = 0;
    while(EventCode[i] != '\0') {

        if(EventCode[i] == '.') {

            replace_char(EventCode, '.', ':');
            break;
        }
        i++;
    }

    /*
    if (strcmp(string, "") == 0)
    {
  // do something
    }
    else if (strcmp(string, "xxx") == 0)
    {
  // do something else
    }
/* more else if clauses */
    //else /* default: */
    //{
    //}


}

struct perf_event_attr capital_latency_event(char* EventCode, int sample_type,
            int read_format, int sample_period) {

    int i = 0;
    int colon = 0;
    struct perf_event_attr attr;


    //toupper(EventCode[i]);

    memset(&attr, 0, sizeof(attr));

    while(EventCode[i] != '\0') {

        EventCode[i] = toupper((unsigned char)EventCode[i]);
        if(EventCode[i] == '.') {

            replace_char(EventCode, '.', ':');
            break;
        }
        i++;
    }

    attr.type=PERF_TYPE_RAW;
    attr.size=sizeof(struct perf_event_attr);


	if(AGGREGATE);
	else {
       	attr.sample_period=sample_period;
		attr.sample_type=sample_type_handle;
		attr.read_format=read_format_handle;
	}

	attr.disabled=1;
	attr.wakeup_events=1;
	attr.pinned=1;
	attr.precise_ip=0;
	attr.exclude_kernel=1;
	attr.exclude_hv=1;

    if (strcmp(EventCode, "FRONTEND_RETIRED:LATENCY_GE_2") == 0)
    {
   	 attr.config = 0x5101c6;
   	 attr.config1 = 0x400206;
    }
    else if (strcmp(EventCode, "FRONTEND_RETIRED:LATENCY_GE_4") == 0)
    {
   	 attr.config = 0x5101c6;
   	 attr.config1 = 0x400406;
    }
    else if (strcmp(EventCode, "FRONTEND_RETIRED:LATENCY_GE_8") == 0)
    {
   	 attr.config = 0x5101c6;
   	 attr.config1 = 0x400806;
    }
    else if (strcmp(EventCode, "FRONTEND_RETIRED:LATENCY_GE_16") == 0)
    {
   	 attr.config = 0x5101c6;
   	 attr.config1 = 0x401006;
    }
    else if (strcmp(EventCode, "FRONTEND_RETIRED:LATENCY_GE_32") == 0)
    {
   	 attr.config = 0x5101c6;
   	 attr.config1 = 0x402006;
    }
    else if (strcmp(EventCode, "FRONTEND_RETIRED:LATENCY_GE_64") == 0)
    {
   	 attr.config = 0x5101c6;
   	 attr.config1 = 0x404006;
    }
    else if (strcmp(EventCode, "FRONTEND_RETIRED:LATENCY_GE_128") == 0)
    {
   	 attr.config = 0x5101c6;
   	 attr.config1 = 0x408006;
    }
    else if (strcmp(EventCode, "FRONTEND_RETIRED:LATENCY_GE_256") == 0)
    {
   	 attr.config = 0x5101c6;
   	 attr.config1 = 0x410006;
    }
    else if (strcmp(EventCode, "FRONTEND_RETIRED:LATENCY_GE_512") == 0)
    {
   	 attr.config = 0x5101c6;
   	 attr.config1 = 0x420006;
    }

    else if (strcmp(EventCode, "FRONTEND_RETIRED:LATENCY_GE_2_BUBBLES_GE_1") == 0)
    {
   	 attr.config = 0x5101c6;
   	 attr.config1 = 0x100206;
    }
    else if (strcmp(EventCode, "FRONTEND_RETIRED:LATENCY_GE_2_BUBBLES_GE_2") == 0)
    {
   	 attr.config = 0x5101c6;
   	 attr.config1 = 0x200206;
    }
    else if (strcmp(EventCode, "FRONTEND_RETIRED:LATENCY_GE_2_BUBBLES_GE_3") == 0)
    {
   	 attr.config = 0x5101c6;
   	 attr.config1 = 0x300206;
    }
    else
    {
        printf("Latency event not found! Exiting!\n");
        exit(1);
    }

    return attr;

}

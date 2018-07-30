#ifndef _PAPI_sample_h
#define _PAPI_sample_h

#define PAPI_BR_INST_RETIRED_ALL_BRANCHES	1
#define PAPI_BR_INST_RETIRED_CONDITIONAL	2
#define PAPI_BR_INST_RETIRED_FAR_BRANCH	3
#define PAPI_BR_INST_RETIRED_NEAR_CALL	4
#define PAPI_BR_INST_RETIRED_NEAR_RETURN	5
#define PAPI_BR_INST_RETIRED_NEAR_TAKEN	6
#define PAPI_BR_MISP_RETIRED_ALL_BRANCHES	7
#define PAPI_BR_MISP_RETIRED_CONDITIONAL	8
#define PAPI_BR_MISP_RETIRED_NEAR_CALL	9
#define PAPI_BR_MISP_RETIRED_NEAR_TAKEN	10
#define PAPI_FRONTEND_RETIRED_DSB_MISS	11
#define PAPI_FRONTEND_RETIRED_ITLB_MISS	12
#define PAPI_FRONTEND_RETIRED_STLB_MISS	13
#define PAPI_FRONTEND_RETIRED_L1I_MISS	14
#define PAPI_FRONTEND_RETIRED_L2_MISS	15
#define PAPI_FRONTEND_RETIRED_LATENCY_GE_128	16
#define PAPI_FRONTEND_RETIRED_LATENCY_GE_16	17
#define PAPI_FRONTEND_RETIRED_LATENCY_GE_2_BUBBLES_GE_1	18
#define PAPI_FRONTEND_RETIRED_LATENCY_GE_2_BUBBLES_GE_2	19
#define PAPI_FRONTEND_RETIRED_LATENCY_GE_2_BUBBLES_GE_3	20
#define PAPI_FRONTEND_RETIRED_LATENCY_GE_2	21
#define PAPI_FRONTEND_RETIRED_LATENCY_GE_256	22
#define PAPI_FRONTEND_RETIRED_LATENCY_GE_32	23
#define PAPI_FRONTEND_RETIRED_LATENCY_GE_4	24
#define PAPI_FRONTEND_RETIRED_LATENCY_GE_512	25
#define PAPI_FRONTEND_RETIRED_LATENCY_GE_64	26
#define PAPI_FRONTEND_RETIRED_LATENCY_GE_8	27
#define PAPI_MEM_INST_RETIRED_STLB_MISS_LOADS	28
#define PAPI_MEM_INST_RETIRED_STLB_MISS_STORES	29
#define PAPI_MEM_INST_RETIRED_LOCK_LOADS	30
#define PAPI_MEM_INST_RETIRED_SPLIT_LOADS	31
#define PAPI_MEM_INST_RETIRED_SPLIT_STORES	32
#define PAPI_MEM_INST_RETIRED_ALL_LOADS	33
#define PAPI_MEM_INST_RETIRED_ALL_STORES	34
#define PAPI_HLE_RETIRED_ABORTED	35
#define PAPI_INST_RETIRED_TOTAL_CYCLES	36
#define PAPI_MEM_LOAD_L3_HIT_RETIRED_XSNP_HIT	37
#define PAPI_MEM_LOAD_L3_HIT_RETIRED_XSNP_HITM	38
#define PAPI_MEM_LOAD_L3_HIT_RETIRED_XSNP_MISS	39
#define PAPI_MEM_LOAD_L3_HIT_RETIRED_XSNP_NONE	40
#define PAPI_MEM_LOAD_MISC_RETIRED_UC	41
#define PAPI_MEM_LOAD_RETIRED_FB_HIT	42
#define PAPI_MEM_LOAD_RETIRED_L1_HIT	43
#define PAPI_MEM_LOAD_RETIRED_L1_MISS	44
#define PAPI_MEM_LOAD_RETIRED_L2_HIT	45
#define PAPI_MEM_LOAD_RETIRED_L2_MISS	46
#define PAPI_MEM_LOAD_RETIRED_L3_HIT	47
#define PAPI_MEM_LOAD_RETIRED_L3_MISS	48
#define PAPI_RTM_RETIRED_ABORTED	49
#define PAPI_INST_RETIRED_TOTAL_INST    50

//extern struct mmap_info * events;

typedef struct {
    void* sample_mmap;
    int * fd;
    long long * prev_head;
} mmap_info;


int * PAPI_sample_init(int Eventset, char* EventCodes, int NumEvents,
    int sample_type, int sample_period, char* filename);

struct perf_event_attr capital_latency_event(char* EventCode, int sample_type,
            int read_format, int sample_period);

int PAPI_sample_start(int * fd);

int PAPI_sample_stop(int * fd, int NumEvents);

struct perf_event_attr new_setup_perf(char* EventCode, int sample_type,
                                    int sample_period, int firstEvent);

#endif

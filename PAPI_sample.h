#define PAPI_DSB_MISS 1
#define PAPI_L1INS_MISS 2
#define PAPI_L2INS_MISS 3
#define PAPI_ITLB_MISS 4
#define PAPI_ISTLB_MISS 5
#define PAPI_ISTLB_MISS_LOADS 6
#define PAPI_ISTLB_MISS_STORES 7
#define PAPI_LOCK_LOADS 8
#define PAPI_SPLIT_LOADS  9
#define PAPI_SPLIT_STORES 10
#define PAPI_ALL_LOADS    11
#define PAPI_ALL_STORES   12

int PAPI_sample_init( int Eventset,int EventCode, int sample_type,
   int sample_period, char filename);

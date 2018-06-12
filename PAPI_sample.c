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

int PAPI_sample_init( int Eventset,
                      int EventCode,
                      int sample_type,
                      int sample_period,
                      int threshold,
                      void *buffer,
                      int buffer_size,
                      PAPI_sample_full_callback handler) {


  switch(EventCode) {
    case :
    case :
    case :
    case :
    case :
    case :
    case :
      

  }














}

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
#include <perfmon/perf_event.h>
#include <perfmon/pfmlib_perf_event.h>
#include <inttypes.h>
#include <papi.h>

#include "char_replace.h"
//#include "perf_event.h"
#include "test_utils.h"
//#include "perf_helpers.h"


int main(int argc, char** argv) {

    pfm_perf_encode_arg_t raw;
    struct perf_event_attr attr;
    int ret, i;
    char *event = "BR_INST_RETIRED.ALL_BRANCHES";

    FILE* fp;
    char* line = NULL;
    uint64_t len = 0;
    int32_t read;

    fp = fopen("NEWEVS", "r");
    if(fp == NULL) {
        printf("Error opening list of events\n");
        return -1;
    }

    ret = pfm_initialize();
    if (ret != PFM_SUCCESS)
        errx(1, "cannot initialize library %s", pfm_strerror(ret));

    memset(&raw, 0, sizeof(raw));

    while((read = getline(&line, &len, fp)) != -1) {

        memset(&raw, 0, sizeof(raw));
        raw.size = sizeof(raw);
        raw.attr = &attr;

        //printf("%d\n",read);
        line[(read -1)] = '\0';
        event = line;

        //printf("%s\n", event);
        //printf("\t\t\t//%s\n", event);
        //PERF_OS_PERF_EVENT PERF_OS_PERF_EVENT_EXT PERF_OS_EVENT PFM_OS_PERF_EXT PFM_OS_PERF
        ret = pfm_get_os_event_encoding(event, PFM_PLM3, PFM_OS_PERF_EVENT, &raw);
        if (ret != PFM_SUCCESS)
  // err(1, " cannot get encoding %s", pfm_strerror(ret));
            printf("gen_codes can't get encoding %s\n",  pfm_strerror(ret));
        else {
            replace_char(line, '.', '_');
            printf("\t\tcase\tPAPI_%s\t:\n", line);
            /*
            for(i=0; i < raw.count; i++)
                printf("\t\t\tpe.config=0x%"PRIx64"\n", raw.codes[i]);
            */
            printf("\t\t\tpe.config=0x%x\n", raw.attr->config);
            printf("\t\t\tbreak;\n");
        }

    }

    fclose(fp);
    if(line)
        free(line);
    //free(raw);

    return 0;
}

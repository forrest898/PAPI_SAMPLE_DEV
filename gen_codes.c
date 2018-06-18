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
#include <inttypes.h>
#include "char_replace.h"


int main(int argc, char** argv) {

    pfm_pmu_encode_arg_t raw;
    int ret, i;
    char *event = "BR_INST_RETIRED.ALL_BRANCHES";

    FILE* fp;
    char* line = NULL;
    uint64_t len = 0;
    int32_t read;

    fp = fopen("LISTOFEVENTS", "r");
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

        //printf("%d\n",read);
        line[(read -1)] = '\0';
        event = line;

        //printf("\t\t\t//%s\n", event);

        ret = pfm_get_os_event_encoding(event, PFM_PLM3, PFM_OS_NONE, &raw);
        if (ret != PFM_SUCCESS)
  // err(1, " cannot get encoding %s", pfm_strerror(ret));
            printf("gen_codes can't get encoding %s\n",  pfm_strerror(ret));
        else {
            replace_char(line, '.', '_');
            printf("\t\tcase\tPAPI_%s\t:\n", line);
            for(i=0; i < raw.count; i++)
                printf("\t\t\tpe.config=0x%"PRIx64"\n", raw.codes[i]);
            printf("\t\t\tbreak;\n");
        }

    }

    fclose(fp);
    if(line)
        free(line);
    free(raw.codes);

    return 0;
}

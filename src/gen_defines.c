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
    int count = 1;
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

    //printf("%d\n",count);
    while((read = getline(&line, &len, fp)) != -1) {

        line[(read -1)] = '\0';
        replace_char(line, '.', '_');
        event = line;
        //printf("%d\n",count);
        printf("#define PAPI_%s\t%d\n", event, count);

        count++;
    }

   // printf("%d", count);
    fclose(fp);
    if(line)
        free(line);
    free(raw.codes);

    return 0;



}

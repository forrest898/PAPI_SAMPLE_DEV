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
#include <papi.h>
#include "papiStdEventDefs.h"

int main(int argc, char **argv) {


    printf("0x%x", 105);
    return 0;

}
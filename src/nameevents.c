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



int main(int argc, char** argv) {

  pfm_pmu_encode_arg_t raw;
int ret, i;
char *event = argv[1];



if(argc != 2) {
  printf("Enter a commmand to translate to hex!\n");
  return -1;
}

printf("\n%s\n", argv[1]);
/*
for(i = 0; i < argc; i++)
  printf("%s%d\n", argv[i], i);
*/

//ret = pfm_find_event(argv[1]);
//printf("\nLibrary version: %d\n", PFMLIB_MAJ_VERSION(pfm_get_version()));

ret = pfm_initialize();
if (ret != PFM_SUCCESS)
   errx(1, "cannot initialize library %s", pfm_strerror(ret));

memset(&raw, 0, sizeof(raw));

ret = pfm_get_os_event_encoding(event, PFM_PLM3, PFM_OS_NONE, &raw);
if (ret != PFM_SUCCESS)
   err(1, " cannot get encoding %s", pfm_strerror(ret));

for(i=0; i < raw.count; i++)
   printf("count[%d]=0x%"PRIx64"\n", i, raw.codes[i]);

free(raw.codes);
return 0;



}

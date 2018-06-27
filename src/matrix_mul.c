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
#include <inttypes.h>
#include "matrix_multiply.h"

int main(int argc, char** argv) {

	int i;

	for(i = 0; i < 3; i++)
		naive_matrix_multiply(0);

	return 0;
}

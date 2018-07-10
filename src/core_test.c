#include <stdio.h>
#include <sys/sysinfo.h>


int main(int argc, char** argv) {

	printf("The number of usable threads is %d\n", get_nprocs());

	return 0;
}

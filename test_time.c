#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define FIB_DEV "/dev/fibonacci"


int main()
{
    FILE *fptr;
    fptr = fopen("data.txt", "w");
    if (!fptr)
        return 0;
    char buf[1];
    char write_buf[] = "testing writing";
    int offset = 100; /* TODO: try test something bigger than the limit */

    int fd = open(FIB_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    for (int i = 0; i <= offset; i++) {
        long long sz, kt;
        struct timespec t1, t2;
        lseek(fd, i, SEEK_SET);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        sz = read(fd, buf, 1);
        clock_gettime(CLOCK_MONOTONIC, &t2);

        printf("Reading from " FIB_DEV
               " at offset %d, returned the sequence "
               "%lld.\n",
               i, sz);

        kt = write(fd, write_buf, strlen(write_buf));
        long long ut = (long long) (t2.tv_sec * 1e9 + t2.tv_nsec) -
                       (t1.tv_sec * 1e9 + t1.tv_nsec);


        fprintf(fptr, "%d %lld %lld %lld\n", i, ut, kt, ut - kt);
        printf("The usertime of sequence %lld.\n", ut);
        printf("Time taken to calculate the sequence %lld.\n", kt);
    }

    fclose(fptr);
    close(fd);
    return 0;
}
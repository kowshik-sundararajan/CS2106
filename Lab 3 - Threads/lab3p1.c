#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUMELTS 16384

// IMPORTANT: Compile using "gcc lab3p1.c .lm -o lab3p1".
// The "-lm" is important as it brings in the Math library.

// Implements the naive primality test.
// Returns TRUE if n is a prime number
int prime(int n) {
	int ret = 1, i;
	for(i = 2; i <= (int) sqrt(n) && ret; i++)
		ret = n % i;

	return ret;
}

int main() {
    int data[NUMELTS];
    int range = (int) NUMELTS / 2;
    int i;
    int childPrimes;

    int fd[2];
    pipe(fd);

    srand(time(NULL));

    for (i = 0; i < NUMELTS; i++)
        data[i] = (int)(((double) rand() / (double) RAND_MAX) * 10000);

    int pid = fork();
    if (pid > 0) { // Parent process
        int i;
        int parentPrimes = 0;
        int status;

        for (i = 0; i < range; i++)
            if (prime(data[i])) parentPrimes++;

        close(fd[1]);
        read(fd[0], & childPrimes, sizeof(childPrimes));
        printf("Number of primes found by the parent: %d\n", parentPrimes);
        printf("Number of primes found by the child: %d\n", childPrimes);
        printf("Total number of primes found: %d\n", (parentPrimes + childPrimes));
    } else if (pid == 0) { // Child process
        int i;
        close(fd[0]);
        for (i = range; i < NUMELTS; i++)
            if (prime(data[i])) childPrimes++;
        write(fd[1], & childPrimes, sizeof(childPrimes));
        close(fd[1]);
    }
}

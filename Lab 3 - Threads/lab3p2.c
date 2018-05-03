#include <stdio.h>
#include <pthread.h>

// Global counter variable
int ctr = 0;

// Create 10 threads
pthread_t thread[10];

void *child(void *t) {
	// Print out the parameter passed in, and the current value of ctr.
	printf("I am child %d. Ctr=%d\n", t, ctr);
	// Then increment ctr
	ctr++;
	pthread_exit(NULL);
}

int main() {
	int i;
	// Initialize ctr
	ctr = 0;

	// Create the threads
	for(i = 0; i<10; i++)
		pthread_create(&thread[i], NULL, child, (void *) i);

	for(i = 0; i<10; i++) {
		pthread_join(thread[i], NULL);
		printf("Thread %d done\n", i);
	}

	// And print out ctr
	printf("Value of ctr=%d\n", ctr);
	return 0;
}

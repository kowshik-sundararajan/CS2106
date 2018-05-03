#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
// Maximum size of our comms buffer
#define MAX_BUFFER_LEN 1024

int main() {
	int fd[2];
	pipe(fd);
	FILE *f = fopen("log.txt", "w");
	// Our buffer
	char buffer[1024];
	// # of characters written/read
	int n;
	// We are sending from parent to child
	if(fork() == 0) {
		// Child should close the output side of the pipe
		close(fd[1]);
		// Read from the pipe
		n = read(fd[0], buffer, MAX_BUFFER_LEN);
		fprintf(f, buffer);
		printf("Child read %d bytes from parent: %s\n", n, buffer);
		fclose(f);
		close(fd[0]);
	}
	else {
		int status;
		// Parent should close input side of the pipe
		close(fd[0]);
		// Write to the pipe
		sprintf(buffer, "Hello child! This is your parent!");
		n = write(fd[1], buffer, strlen(buffer) + 1);
		printf("Parent wrote %d bytes to the child: %s\n", n, buffer);
		close(fd[1]);
		// Wait for the child to end
		wait(&status);
	}
}

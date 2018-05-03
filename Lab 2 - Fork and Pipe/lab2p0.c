#include <stdio.h>
#include <unistd.h>

int main() {
    int cpid;
    if((cpid = fork()) != 0) {
      printf("Hello, I am the PARENT. My PID is %d, my child's PID is %d, and my parent's PID is %d\n", getpid(), cpid, getppid());
    }
    else {
      printf("Hello, I am the CHILD. My PID is %d, my parent's PID is %d, fork returned %d\n", getpid(), getppid(), cpid);
    }
}

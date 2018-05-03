#ifndef __Q_HEADER__
#define __Q_HEADER__

#include <pthread.h>
#include <semaphore.h>

//# of entries in the queue
#define QLEN		10

// Length of each queue entry in bytes
#define ENTRY_SIZE	1024 // Change ENTRY_SIZE from 64 to 1024 to accommodate LOG_BUFFER_LEN

typedef struct {
	sem_t empty, full;
	char data[QLEN][ENTRY_SIZE];
	int len[QLEN];
	int front, back;
	int count;
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
} TBuffer;

void initBuffer(TBuffer *buffer);
void enq(TBuffer *buffer, const char *data, int len);
int deq(TBuffer *buffer, char *data);
#endif

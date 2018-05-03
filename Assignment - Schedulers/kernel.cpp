#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "llist.h"
#include "prioll.h"
#include "kernel.h"

/* OS variables */

// Current number of processes
int procCount;

// Current timer tick
int timerTick = 0;
int currProcess, currPrio;

#if SCHEDULER_TYPE == 0

/* Process Control Block for LINUX scheduler*/
typedef struct tcb {
	int procNum;
	int prio;
	int quantum;
	int timeLeft;
} TTCB;

#elif SCHEDULER_TYPE == 1

/* Process Control Block for RMS scheduler*/

typedef struct tcb {
	int procNum;
	int timeLeft;
	int deadline;
	int c;
	int p;
} TTCB;

#endif


TTCB processes[NUM_PROCESSES];

#if SCHEDULER_TYPE == 0

// Lists of processes according to priority levels.
TNode *queueList1[PRIO_LEVELS];
TNode *queueList2[PRIO_LEVELS];

// Active list and expired list pointers
TNode **activeList = queueList1;
TNode **expiredList = queueList2;

#elif SCHEDULER_TYPE == 1

// Ready queue and blocked queue
TPrioNode *readyQueue, *blockedQueue;

// This stores the data for pre-empted processes.
TPrioNode *suspended; // Suspended process due to pre-emption

// Currently running process
TPrioNode *currProcessNode; // Current process
#endif


#if SCHEDULER_TYPE == 0

// Searches the active list for the next priority level with processes
int findNextPrio(int currPrio) {
	int i;
	for(i = 0; i < PRIO_LEVELS; i++)
		if(activeList[i] != NULL)
			return i;

	return -1;
}

// Returns the quantum in ms for a particular priority level.
int findQuantum(int priority) {
	return ((PRIO_LEVELS - 1) - priority) * QUANTUM_STEP + QUANTUM_MIN;
}

int linuxScheduler() {
	if (timerTick == 0) {
		return currProcess;
	} else {
		// If the current process is not on its last quantum
		if (processes[currProcess].quantum > 1) {
			processes[currProcess].quantum--;
			return currProcess;
		} else { // Current process is on its last quantum
			// Find the next priority
			int nextPrio = findNextPrio(currPrio);
			int nextProcess;

			// Restore the quantum of the current process and insert it into the expired list
			processes[currProcess].quantum = findQuantum(currPrio);
			insert(&expiredList[currPrio], currProcess, processes[currProcess].quantum);

			if (nextPrio == -1) { // Active list is empty
				destroy(&activeList[currPrio]);

				// Swap active and expired lists
				TNode **tempList = activeList;
				activeList = expiredList;
				expiredList = tempList;

				printf("\n***** SWAPPED LISTS *****\n\n");

				int i;
				for(i = 0; i < PRIO_LEVELS; i++)
					expiredList[i] = NULL;

				nextPrio = findNextPrio(0);
			} else { // Active list is not empty
				if (nextPrio != currPrio) // Current priority list is empty
					destroy(&activeList[currPrio]);
			}

			// Get the next process to run from the active list
			nextProcess = remove(&activeList[nextPrio]);
			currPrio = nextPrio;

			return nextProcess;
		}
	}
	return 0;
}

#elif SCHEDULER_TYPE == 1

int RMSScheduler() {
	// Move all processes in the blocked queue that are ready to the ready queue
	TPrioNode *node = checkReady(blockedQueue, timerTick);
	TTCB *process;

	while(node != NULL) {
		process = &processes[node->procNum];
		process->deadline = process->deadline + process->p;
		process->timeLeft = process->c;
		prioRemoveNode(&blockedQueue, node);
		prioInsertNode(&readyQueue, node);
		node = checkReady(blockedQueue, timerTick);
	}

	// If there is a suspended process, put it in the ready queue
	if (suspended != NULL) {
		prioInsertNode(&readyQueue, suspended);
		suspended = NULL;
	}

	// Get the next process from the readyQueue
	TPrioNode *nextProcessNode = peek(readyQueue);

	// If previous round was idle
	if (currProcessNode == NULL) {
		if (nextProcessNode == NULL)
			return -1;

		currProcessNode = prioRemoveNode(&readyQueue, nextProcessNode);
		TTCB *currProcess = &processes[currProcessNode->procNum];
		currProcess->timeLeft--;
		return currProcessNode->procNum;
	}

	// Previous round was not idle

	TTCB *currProcess = &processes[currProcessNode->procNum];

	// If deadline was missed
	if (timerTick >= currProcess->deadline) {
		// If the timeLeft of the current process has not been extended yet
		if (currProcess->timeLeft < currProcess->c)
			currProcess->timeLeft += currProcess->c;

		// If the previous deadline was cleared, update the deadline
		if (currProcess->timeLeft == currProcess->c)
			currProcess->deadline += currProcess->p;
	}

	if (nextProcessNode != NULL) {
		// If current process has finished running
		if (currProcess->timeLeft <= 0) {
			prioInsertNode(&blockedQueue, currProcessNode);
			currProcessNode = prioRemoveNode(&readyQueue, nextProcessNode);
		} else if (nextProcessNode->prio < currProcessNode->prio) { // If the priority of the first process in the queue is higher than the priority of the current process
			suspended = currProcessNode; // suspend the current process
			currProcessNode = prioRemoveNode(&readyQueue, nextProcessNode);
			printf("\n--- PRE-EMPTION ---\n\n");
		}

		currProcess = &processes[currProcessNode->procNum];
		currProcess->timeLeft--;
		return currProcessNode->procNum;
	} else { // Ready queue is empty
		// If current process has time left
		if (currProcess->timeLeft > 0) {
			currProcess->timeLeft--;
			return currProcessNode->procNum;
		}

		// If current process does not have time left
		prioInsertNode(&blockedQueue, currProcessNode);
		currProcessNode = NULL;
		return -1;
	}
}

#endif

void timerISR() {

#if SCHEDULER_TYPE == 0
	currProcess = linuxScheduler();
#elif SCHEDULER_TYPE == 1
	currProcess = RMSScheduler();
#endif

#if SCHEDULER_TYPE == 0
	static int prevProcess = -1;

	// To avoid repetitiveness for hundreds of cycles, we will only print when there's
	// a change of processes
	if(currProcess != prevProcess) {
		// Print process details for LINUX scheduler
		printf("Time: %d Process: %d Prio Level: %d Quantum : %d\n", timerTick, processes[currProcess].procNum+1,
			processes[currProcess].prio, processes[currProcess].quantum);
		prevProcess = currProcess;
	}

#elif SCHEDULER_TYPE == 1
	// Print process details for RMS scheduler
	printf("Time: %d ", timerTick);
	if(currProcess == -1)
		printf("---\n");
	else {
		// If we have busted a processe's deadline, print !! first
		int bustedDeadline = (timerTick >= processes[currProcess].deadline);

		if(bustedDeadline)
			printf("!! ");

		printf("P%d Deadline: %d", currProcess + 1, processes[currProcess].deadline);

		if(bustedDeadline)
			printf(" !!\n");
		else
			printf("\n");
	}

#endif

	// Increment timerTick. You will use this for scheduling decisions.
	timerTick++;
}

void startTimer() {
	// In an actual OS this would make hardware calls to set up a timer
	// ISR, start an actual physical timer, etc. Here we will simulate a timer
	// by calling timerISR every millisecond

	int i;

#if SCHEDULER_TYPE == 0
	int total = processes[currProcess].quantum;

	for(i = 0; i < PRIO_LEVELS; i++) {
		total += totalQuantum(activeList[i]);
	}

	for(i = 0; i < NUM_RUNS * total; i++) {
		timerISR();
		usleep(1000);
	}

#elif SCHEDULER_TYPE == 1
	// Find LCM of all periods
	int lcm = prioLCM(readyQueue);

	for(i = 0; i < NUM_RUNS*lcm; i++) {
		timerISR();
		usleep(1000);
	}
#endif
}

void startOS() {
#if SCHEDULER_TYPE == 0
	// There must be at least one process in the activeList
	currPrio = findNextPrio(0);

	if(currPrio < 0) {
		printf("ERROR: There are no processes to run!\n");
		return;
	}

	// set the first process
	currProcess = remove(&activeList[currPrio]);

#elif SCHEDULER_TYPE == 1
	currProcessNode = prioRemove(&readyQueue);
	currProcess = currProcessNode->procNum;
#endif

	// Start the timer
	startTimer();

#if SCHEDULER_TYPE == 0
	int i;
	for(i = 0; i < PRIO_LEVELS; i++)
		destroy(&activeList[i]);

#elif SCHEDULER_TYPE == 1
	prioDestroy(&readyQueue);
	prioDestroy(&blockedQueue);

	if(suspended != NULL)
		free(suspended);
#endif
}

void initOS() {
	// Initialize all variables
	procCount = 0;
	timerTick = 0;
	currProcess = 0;
	currPrio = 0;

#if SCHEDULER_TYPE == 0
	int i;
	// Set both queue lists to NULL
	for(i = 0; i < PRIO_LEVELS; i++) {
		queueList1[i] = NULL;
		queueList2[i] = NULL;
	}

#elif SCHEDULER_TYPE == 1
	// Set readyQueue and blockedQueue to NULL
	readyQueue = NULL;
	blockedQueue = NULL;

	// The suspended variable is used to store
	// which process was pre-empted.
	suspended = NULL;
#endif
}

#if SCHEDULER_TYPE == 0
// Adds a process to the process table
int addProcess(int priority) {
	if(procCount >= NUM_PROCESSES)
		return -1;

	// Insert process data into the process table
	processes[procCount].procNum = procCount;
	processes[procCount].prio = priority;
	processes[procCount].quantum = findQuantum(priority);
	processes[procCount].timeLeft = processes[procCount].quantum;

	// Add to the active list
	insert(&activeList[priority], processes[procCount].procNum, processes[procCount].quantum);
	procCount++;
	return 0;
}

#elif SCHEDULER_TYPE == 1
// Adds a process to the process table
int addProcess(int p, int c) {
	if(procCount >= NUM_PROCESSES)
		return -1;

		// Insert process data into the process table
		processes[procCount].p = p;
		processes[procCount].c = c;
		processes[procCount].timeLeft = c;
		processes[procCount].deadline = p;

		// And add to the ready queue.
		prioInsert(&readyQueue, procCount, p, p);
	procCount++;
	return 0;
}
#endif

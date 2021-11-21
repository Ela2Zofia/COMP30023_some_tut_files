#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ITERATIONS 1000000

void* runner(void* param); /* thread doing the work */

int count = 0;
pthread_mutex_t lock;

int main(int argc, char** argv) {
	pthread_t tid1, tid2;

	if (pthread_mutex_init(&lock, NULL) != 0) {
		printf("mutex init failed\n");
		exit(1);
	}

	if (pthread_create(&tid1, NULL, runner, NULL)) {
		printf("Error creating thread 1\n");
		exit(1);
	}
	if (pthread_create(&tid2, NULL, runner, NULL)) {
		printf("Error creating thread 2\n");
		exit(1);
	}

	/* wait for the threads to finish */
	if (pthread_join(tid1, NULL)) {
		printf("Error joining thread\n");
		exit(1);
	}
	if (pthread_join(tid2, NULL)) {
		printf("Error joining thread\n");
		exit(1);
	}

	if (count != 2 * ITERATIONS)
		printf("** ERROR ** count is [%d], should be %d\n", count, 2 * ITERATIONS);
	else
		printf("OK! count is [%d]\n", count);

	pthread_exit(NULL);
	pthread_mutex_destroy(&lock);
	return 0;
}

/* thread doing the work */
void* runner(void* param) {
	int i, temp;
	for (i = 0; i < ITERATIONS; i++) {
		temp = count;    /* copy the global count locally */
		temp = temp + 1; /* increment the local copy */
		count = temp;    /* store the local value into the global count */
	}
	return NULL;
}

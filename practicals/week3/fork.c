#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char** argv) {
	pid_t root = getpid();
	// when forking, program does not start again since memory/register values are exactly the same
	// i.e. instruction pointer is at the same line too. So fork() wont execute again.
	pid_t pid = fork();
	printf("from %d forking into %d\n", root, pid);
	sleep(20);

	// watch 2 different PIDs spawn 2 more child processes.
	pid_t mypid = getpid();
	pid = fork();
	printf("from %d forking into %d\n", mypid, pid);
	sleep(20);

	if (getpid() == root) {
		sleep(20);
		printf("root exiting\n");
	} else {
		printf("Child -- PID %d exiting\n", getpid());
	}
    return 0;
}

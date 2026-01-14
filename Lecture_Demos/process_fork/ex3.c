#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * Exercise 3
 *
 * In this exercise, we would like to create a cascade of children, each one
 * creating a child of its own.
 *
 * Starting from a parent, we would like to see something like this:
 * parent -> child 1 -> child 2 -> child 3 -> child 4
 *
 * Each process will print its process id and its order in the chain.
 * So parent prints: Parent has pid: <parent pid>
 * First child prints: Child 1 has pid: <pid>
 * Second child prints: Child 2 has pid: <pid>
 *
 * Each parent must wait for its child to exit before they can exit.
 *  This means that the main parent will not exit until ALL children have
 *  exited.
 *
 * OPTIONAL: read the number of children to create from the command line.
 */

int main(int argc, char **argv) {
    int rc; 
    int num_children = atoi(argv[1]);
    int count = 1;
    printf("Parent has pid: %d\n", getpid());
    
    while (count <= num_children) {
	rc = fork();
	if (rc < 0) {
	    perror("fork failed:");
	    exit(EXIT_FAILURE);
	}

	if (rc == 0) {
	    printf("Child %d has pid: %d\n", count, getpid());
	    count++;
	} else {
	    wait(0);
	    break;
	}
    }

    return 0;
}

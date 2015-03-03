/* pidns_init_sleep.c

   Copyright 2013, Michael Kerrisk
   Licensed under GNU General Public License v2 or later

   A simple demonstration of PID namespaces.
*/
#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>

/* A simple error-handling function: print an error message based
   on the value in 'errno' and terminate the calling process */

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

#define TASK_STACK_SIZE (1024 * 1024)

pid_t spawn_new_task(int(*task_callback)(void*), char* task_stack, char* task_id)
{
  pid_t child_pid;

  child_pid = clone(task_callback,
                  task_stack + TASK_STACK_SIZE,   /* Points to start of
                                                 downwardly growing stack */
                  CLONE_NEWPID | SIGCHLD, (void*)task_id);

  if (child_pid == -1)
      errExit("clone");

  return child_pid;
}

/*
Task tree should looks like this :

    "root"
    /    \
  t1      t2
  |
  t3
  |
  t4

  t1 & t2 are both in a different new PID namespace
  t3 is in ns spawnd from t1
  t4 is in ns spawnd from t3

*/

void print_task(char* task_id)
{
    printf("%s: PID  = %ld\n", task_id, (long) getpid());
    printf("%s: PPID = %ld\n", task_id, (long) getppid());
}

int task1(void*);
int task2(void*);
int task3(void*);
int task4(void*);

static char t1_stack[TASK_STACK_SIZE];
static char t2_stack[TASK_STACK_SIZE];
static char t3_stack[TASK_STACK_SIZE];
static char t4_stack[TASK_STACK_SIZE];

int task1(void* arg)
{
    char* task_id = (char*)arg;

    print_task(task_id);

    pid_t task3_pid;

    task3_pid = clone(task3,
                    t3_stack + TASK_STACK_SIZE,   /* Points to start of
                                                   downwardly growing stack */
                    SIGCHLD, "task3");

    if (task3_pid == -1)
        errExit("clone");

    printf("Task 3 created with PID:%d\n", task3_pid);

    execlp("sleep", "sleep", "5", (char *) NULL);
    errExit("execlp");  /* Only reached if execlp() fails */
}

int task2(void* arg)
{
    char* task_id = (char*)arg;

    print_task(task_id);
    execlp("sleep", "sleep", "5", (char *) NULL);
    errExit("execlp");  /* Only reached if execlp() fails */
}

int task3(void* arg)
{
    char* task_id = (char*)arg;

    print_task(task_id);

    // Spawn tasks
    pid_t task4_pid = spawn_new_task(task4, t4_stack, "task4");

    printf("Task 4 created with PID:%d\n", task4_pid);

    execlp("sleep", "sleep", "5", (char *) NULL);
    errExit("execlp");  /* Only reached if execlp() fails */
}

int task4(void* arg)
{
    char* task_id = (char*)arg;

    print_task(task_id);
    execlp("sleep", "sleep", "5", (char *) NULL);
    errExit("execlp");  /* Only reached if execlp() fails */
}


int main(int argc, char *argv[])
{

    // Spawn tasks
    pid_t task1_pid = spawn_new_task(task1, t1_stack, "task1"),
          task2_pid = spawn_new_task(task2, t2_stack, "task2");

    printf("Task 1 created with PID:%d\n", task1_pid);
    printf("Task 2 created with PID:%d\n", task2_pid);

    //Wait for all tasks to complete
    if (waitpid(task1_pid, NULL, 0) == -1)      /* Wait for child */
        errExit("waitpid");

    if (waitpid(task2_pid, NULL, 0) == -1)      /* Wait for child */
        errExit("waitpid");

    exit(EXIT_SUCCESS);
}

#define _GNU_SOURCE
#include "process.h"
#include <sched.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <time.h>
#define GET_TIME 333
#define PRINTK 334

int proc_assign_cpu(int pid, int core)
{
	if (core > sizeof(cpu_set_t)) {
		fprintf(stderr, "Core index exceed.");
		return -1;
	}

	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(core, &mask);
		
	if (sched_setaffinity(pid, sizeof(mask), &mask) < 0) {
		perror("sched_setaffinity");
		exit(-1);
	}

	return 0;
}

int proc_exec(struct process proc)
{
	int pid = fork();

	if (pid < 0) {
		perror("fork");
		exit(-1);
	}

	if (pid == 0) {
		struct timespec start_time, end_time;
		char dmesg[256];
		syscall(GET_TIME, &start_time);
		// proc_assign_cpu(0, CHILD_CPU);
		// proc_block(0);
		for (int i = 0; i < proc.t_exec; i++) {
			UNIT_T();
#ifdef DEBUG
			if (i % 100 == 0)
				fprintf(stderr, "%s: %d/%d\n", proc.name, i, proc.t_exec);
#endif
		}
		syscall(GET_TIME, &end_time);
		sprintf(dmesg, "[OS project1] %d %lu.%09lu %lu.%09lu\n", getpid(),\
				start_time.tv_sec, start_time.tv_nsec, end_time.tv_sec, end_time.tv_nsec);
		syscall(PRINTK, dmesg);   
		exit(0);
	}
	else{ /* Parent assign child to another core */
		proc_assign_cpu(pid, CHILD_CPU);
	}
	return pid;
}

int proc_block(int pid)
{
	struct sched_param param;
	param.sched_priority = 0;

	int ret = sched_setscheduler(pid, SCHED_IDLE, &param);
	
	if (ret < 0) {
		perror("sched_setscheduler");
		exit(-1);
	}

	return ret;
}

int proc_wakeup(int pid)
{
	struct sched_param param;
	param.sched_priority = 0;

	int ret = sched_setscheduler(pid, SCHED_OTHER, &param);
	
	if (ret < 0) {
		perror("sched_setscheduler");
		exit(-1);
	}

	return ret;
}

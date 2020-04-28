#define _GNU_SOURCE
#include "process.h"
#include "scheduler.h"
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sched.h>


static int t_last; /* Last context switch time (for RR scheduling) */
static int t_cur; /* Current unit time */
static int running; /* Index of running process. -1 if no process running */
static int finish_cnt;

int cmp(const void *a, const void *b) {
	return ((struct process *)a)->t_ready - ((struct process *)b)->t_ready;
}

/* get index of next process  */
int next_process(struct process *proc, int nproc, int policy)
{
	/* Non-preemptive => return  current running process */
	if (running != -1 && (policy == SJF || policy == FIFO))
		return running;

	int ret = -1;

	if (policy == PSJF || policy ==  SJF) {
		for (int i = 0; i < nproc; i++) {
			/* process not ready for done */
			if (proc[i].pid == -1 || proc[i].t_exec == 0)
				continue;
			if (ret == -1 || proc[i].t_exec < proc[ret].t_exec)
				ret = i;
		}
	}

	else if (policy == FIFO) {
		for(int i = 0; i < nproc; i++) {
			if(proc[i].pid == -1 || proc[i].t_exec == 0)
				continue;
			if(ret == -1 || proc[i].t_ready < proc[ret].t_ready)
				ret = i;
		}
    }

	else if (policy == RR) {
		if (running == -1){
			for(int i = 0; i < nproc; i++) {
				if(proc[i].pid == -1 || proc[i].t_exec == 0)
					continue;
				if(ret == -1 || proc[i].t_ready < proc[ret].t_ready)
					ret = i;
			}
		}
		else if((t_cur - t_last) % 500 == 0){
			proc[running].t_ready = t_cur; /* enter ready queue again */
			/* if a process go back to ready queue when another new process is ready,
			the old process will enter queue before the new one*/
			for(int i = 0; i < nproc; i++) {
				if(proc[i].pid == -1 || proc[i].t_exec == 0)
					continue;
				if(ret == -1 || proc[i].t_ready < proc[ret].t_ready)
					ret = i;
			}
		}
		else{
			return running;
		}
	}

	return ret;
}

int scheduling(struct process *proc, int nproc, int policy)
{
	qsort(proc, nproc, sizeof(struct process), cmp);

	/* Initial pid = -1 imply not ready */
	for (int i = 0; i < nproc; i++)
		proc[i].pid = -1;

	/* Set single core prevent from preemption */
	proc_assign_cpu(getpid(), PARENT_CPU);
	
	/* Set high priority to scheduler */
	proc_wakeup(getpid());
	
	/* Initial scheduler */
	t_cur = 0;
	running = -1;
	finish_cnt = 0;
	
	while(1) {
		/* Check if running process finish */
		if (running != -1 && proc[running].t_exec == 0) {
			waitpid(proc[running].pid, NULL, 0);
#ifdef DEBUG
			fprintf(stderr, "%s done at time %d.\n", proc[running].name, t_cur);
#endif
			printf("%s %d\n", proc[running].name, proc[running].pid);
			running = -1;
			finish_cnt++;

			/* All process finish */
			if (finish_cnt == nproc)
				break;
		}

		/* Check if process ready and execute */
		for (int i = 0; i < nproc; i++) {
			if (proc[i].t_ready == t_cur && proc[i].pid == -1) {
				proc[i].pid = proc_exec(proc[i]);
				proc_block(proc[i].pid);
#ifdef DEBUG
				fprintf(stderr, "%s ready at time %d.\n", proc[i].name, t_cur);
#endif
			}

		}

		/* Select next running  process */
		int next = next_process(proc, nproc, policy);
		if (next != -1) {
			/* Context switch */
			if (running != next) {
#ifdef DEBUG
				fprintf(stderr, "at t:%d, context switch: %s->%s\n", t_cur, \
						proc[running].name, proc[next].name);
#endif
				proc_wakeup(proc[next].pid);
				proc_block(proc[running].pid);
				running = next;
				t_last = t_cur;
			}
		}

		/* Run an unit of time */
		UNIT_T();
		if (running != -1)
			proc[running].t_exec--;
		t_cur++;
	}

	return 0;
}


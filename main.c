#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "process.h"
#include "scheduler.h"

int main(){
    char policy_string[128];
    int policy;
    int  nproc;
    struct process* proc;
    scanf("%s%d", policy_string, &nproc);
    //fprintf(stderr, "policy_string:[%s], nproc:%d\n", policy_string, nproc);
    
    proc = (struct process*) malloc(nproc * sizeof(struct process));
    if (proc == NULL){ perror("malloc fail"); return -1;}

    for(int i=0; i<nproc; i++){
        scanf("%s%d%d", proc[i].name, &proc[i].t_ready, &proc[i].t_exec);
    }
    //fprintf(stderr, "input done\n");
    if (strcmp(policy_string, "FIFO") == 0)
    {
        policy = FIFO;
    }
	else if (strcmp(policy_string, "RR") == 0) {
		policy = RR;
	}
	else if (strcmp(policy_string, "SJF") == 0) {
		policy = SJF;
	}
	else if (strcmp(policy_string, "PSJF") == 0) {
		policy = PSJF;
	}
	else {
		fprintf(stderr, "Invalid policy: %s", policy_string);
		exit(0);
	}

	scheduling(proc, nproc, policy);
	return 0;
}
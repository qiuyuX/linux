#ifndef _DATA_PRIVFS_H
#define _DATA_PRIVFS_H

/*
 * define all the data structure used in privfs.
 * Version 1: create related data for drs in statm.
 *
 */
#include <linux/kfifo.h>
#include <linux/sched.h>
#include <linux/gfp.h>
#include <linux/string.h>


#define MAX_QUERY_LENGTH 16  
#define BUFFER_SIZE 64

struct drs_pri { // for drs field in statm
	unsigned long original[MAX_QUERY_LENGTH]; // original value
	unsigned long obfuscated[MAX_QUERY_LENGTH]; // obfuscated value 
	struct __kfifo *rbuffer;
};

static inline void rbuffer_alloc(struct drs_pri *dpri)
{
	__kfifo_alloc(dpri->rbuffer, BUFFER_SIZE, 4, GFP_KERNEL);
}	

static inline void rbuffer_free(struct drs_pri *dpri)
{
	__kfifo_free(dpri->rbuffer);
}

static inline void initialize_pri(struct task_struct * tsk)
{	
	long tmp = 1;
	int i;
	struct drs_pri *p;
	p = &(tsk->task_pri);
	rbuffer_alloc(p);
	memset(p->original, 0, MAX_QUERY_LENGTH * 4);
	memset(p->obfuscated, 0, MAX_QUERY_LENGTH * 4);
       	for(i = 0; i < BUFFER_SIZE; i++){
		__kfifo_in(p->rbuffer, &tmp, 1);	
	}//for test	
}

static inline void release_pri(struct task_struct *task)
{
	rbuffer_free(&(task->task_pri));
}
#endif

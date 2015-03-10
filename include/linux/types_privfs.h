#ifndef _DATA_PRIVFS_H
#define _DATA_PRIVFS_H

/*
 * define all the data structure used in privfs.
 * Version 1: create related data for drs in statm.
 *
 */
#include <linux/kfifo.h>
#include <linux/gfp.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/slab.h>

#define MAX_QUERY_LENGTH 16  
#define PRI_BUFFER_SIZE 64

#define PRI_SIZE 4
//fields in statm
#define SIZE 0
#define RESIDENT 1
#define SHARED 2
#define DRS 3

struct task_struct;

struct pri_rbuff{ // store the random laplace noise
	struct __kfifo rbuff;
	struct tasklet_struct *buff_task;	
};

struct pri_struct{ // for drs field in statm
	long original[MAX_QUERY_LENGTH]; // original value
	long obfuscated[MAX_QUERY_LENGTH]; // obfuscated value 
	unsigned int index; // the ith query
	long pri_current; // previous value
	struct pri_rbuff *p_rbuff;
};

extern void initialize_pri(struct task_struct *task);

extern void release_pri(struct task_struct *task);

extern long get_obfuscation(struct task_struct *task, int type, long original);

extern long pri_task_statm(struct task_struct *task, struct mm_struct *mm, long *pri_shared, long *pri_text, long *pri_data, long *pri_resident);
#endif

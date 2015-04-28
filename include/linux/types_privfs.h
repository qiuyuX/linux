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
#define STATM_FILE 0
#define STATM_ANON 1
#define STATM_TOTAL 2
#define STATM_SHARED 3

struct task_struct;

struct pri_rbuff{ // store the random laplace noise for binary tree
	struct __kfifo rbuff;
	struct tasklet_struct *buff_task;	
	int height; // height of the current binary tree
};

struct pri_struct{ // for drs field in statm
	long original[MAX_QUERY_LENGTH]; // original value
	long obfuscated[MAX_QUERY_LENGTH]; // obfuscated value 
	unsigned int index; // the ith query
	long pri_current; // previous value
	struct pri_rbuff *p_rbuff;
	long base_value;
	long base_noisy;
	int base_index;
};

extern void initialize_pri(struct task_struct *task);

extern void release_pri(struct task_struct *task);

extern long get_obfuscation(struct task_struct *task, int type, long original);
#endif

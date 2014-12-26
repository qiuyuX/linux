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

#define MAX_QUERY_LENGTH 16  
#define BUFFER_SIZE 64

struct task_struct;

struct drs_pri { // for drs field in statm
	long original[MAX_QUERY_LENGTH]; // original value
	long obfuscated[MAX_QUERY_LENGTH]; // obfuscated value 
	unsigned int index; // the ith query
	long pri_current; // previous value
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

extern void initialize_pri(struct task_struct *task);

extern void release_pri(struct task_struct *task);

extern long get_obfuscation(struct task_struct *task, long original);
#endif

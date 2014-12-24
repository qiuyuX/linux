#include <linux/sched.h>
#include <linux/types_privfs.h>

void initialize_pri(struct task_struct *task)
{
	long tmp = 1;
	int i;
	struct drs_pri *p;
	p = &(task->task_pri);
	rbuffer_alloc(p);
	memset(p->original, 0, MAX_QUERY_LENGTH * 4);
	memset(p->obfuscated, 0, MAX_QUERY_LENGTH * 4);
       	for(i = 0; i < BUFFER_SIZE; i++){
		__kfifo_in(p->rbuffer, &tmp, 1);	
	}//for test	
}

void release_pri(struct task_struct *task)
{
	rbuffer_free(&(task->task_pri));
}

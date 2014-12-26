#include <linux/sched.h>
#include <linux/types_privfs.h>

void initialize_pri(struct task_struct *task)
{
	long tmp = 1;
	int i;
	struct drs_pri *p;
	p = &(task->task_pri);
	p->index = 0;
	p->pri_current = 0;
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

static inline void refresh_original(struct drs_pri *pri, int index, long ori)
{
	int i = 0;
	long sum = ori - pri->pri_current;
	pri->pri_current = ori; //update the current value
	for(i; i < index; i++){
		sum += pri->original[i];
	//	pri->original[i] = 0;
	}	
	pri->original[index] = sum;
}

static long get_noise(struct drs_pri *pri)
{
	long noise;
	if(__kfifo_out(pri->rbuffer, &noise, 1) != 0){
		return noise;
	}
	else return 0;
}

long get_obfuscation(struct task_struct *task, long ori)
{
	struct drs_pri *pri;
	pri = &(task->task_pri);
	pri->index += 1;
	int i = 0;
	int j = 1;
	int mark = 0; 
	long noisy_sum = 0;
	for(i; i< MAX_QUERY_LENGTH; i++){
		if((j & pri->index) != 0){
		/* find the first right bit which is not 0*/
			if(mark == 0){
				mark = 1;
				refresh_original(pri, i, ori);
				pri->obfuscated[i] = pri->original[i] + get_noise(pri);
				noisy_sum += pri->obfuscated[i];
			}
			else noisy_sum += pri->obfuscated[i];
		}
		j = j<<1;
	}
	return noisy_sum;
}

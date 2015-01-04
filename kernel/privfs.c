#include <linux/sched.h>
#include <linux/types_privfs.h>
#include <linux/types.h>

static const long random_buf[PRI_BUFFER_SIZE] = {
	6, -2, 31, 24, -20, 14, 8, 35,
	-24, -4, -15, -9, -14, -9, -75, 15,
	0, 12, 1, 93, 6, -1, 26, 1,
	9, 22, 27, 28, -16, 6, -3, 24,
	-37, -8, 10, -13, -37, -6, -21, -15,
	16, -4, 2, -54, 13, 3, 18, -15,
	-11, -9, -16, 36, 18, -34, -3, -1,
	39, -10, -7, -23, 40, 18, -95, -73
};

/*
static const long random_buf[PRI_BUFFER_SIZE] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1
};
*/

void drs_refill(unsigned long pid)
{
	printk(KERN_INFO "Refill random buffer for %lu\n", pid);
}

void initialize_pri(struct task_struct *task)
{
//	u32 tmp = 1;
	struct drs_pri *p;
	p = &(task->task_pri);
	p->index = 0;
	p->pri_current = 0;
	rbuffer_alloc(p);
	memset(p->original, 0, MAX_QUERY_LENGTH * 8);
	memset(p->obfuscated, 0, MAX_QUERY_LENGTH * 8);
	__kfifo_in(&(p->rbuffer), random_buf, 64);	
	p->drs_task = kmalloc(sizeof(struct tasklet_struct), GFP_KERNEL);
	tasklet_init(p->drs_task, drs_refill, task->pid);
}

void release_pri(struct task_struct *task)
{
	rbuffer_free(&(task->task_pri));
}

static inline void refresh_original(struct drs_pri *pri, int index, long ori)
{
	int i;
	long sum = ori - pri->pri_current;
	pri->pri_current = ori; //update the current value
	for(i = 0; i < index; i++){
		sum += pri->original[i];
	//	pri->original[i] = 0;
	}	
	pri->original[index] = sum;
}

static long get_noise(struct drs_pri *pri)
{
	long noise;
	int length;
	if(__kfifo_out(&(pri->rbuffer), &noise, 1) != 0){
//		printk(KERN_INFO "Noise: %ld\n", noise);
		length = (pri->rbuffer).in - (pri->rbuffer).out;
		if(length == (PRI_BUFFER_SIZE/2)){
			tasklet_hi_schedule(pri->drs_task);
		}
		return noise;
	}
	else{
		printk(KERN_INFO "Random Buffer is empty!\n");
		return 0;
	}
}

long get_obfuscation(struct task_struct *task, long ori)
{
	struct drs_pri *pri;
	pri = &(task->task_pri);
	pri->index += 1;
	int i;
	int j = 1;
	int mark = 0; 
	long noisy_sum = 0;
	for(i = 0; i< MAX_QUERY_LENGTH; i++){
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
	printk(KERN_INFO "Noisy Sum: %ld\n", noisy_sum);
	return noisy_sum;
}

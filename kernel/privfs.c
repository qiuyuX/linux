#include <linux/sched.h>
#include <linux/types_privfs.h>
#include <linux/types.h>
#include <linux/laplace_pri.h>
#include <linux/pid.h>

#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

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

static long get_laplace(float u, float b){
	float noise = get_fast_laplace(u, b);
	return round(noise);
}

void buff_refill(unsigned long pid)
{
	struct task_struct *task;
	struct __kfifo *buf;
	long noise;
	printk(KERN_INFO "Refill random buffer for %lu\n", pid);
	task = pid_task(find_vpid(pid), PIDTYPE_PID);
	buf = &((task->rbuff).rbuff);
	while((buf->in - buf->out) < PRI_BUFFER_SIZE){
		noise = get_laplace(0, 20);
		__kfifo_in(buf, &noise, 1);
		printk(KERN_INFO "Generated Noise: %ld\n", noise);
	}	
}

static inline void rbuffer_alloc(struct task_struct *task)
{
	struct pri_rbuff *buff = &(task->rbuff);
	__kfifo_alloc(&(buff->rbuff), PRI_BUFFER_SIZE, 8, GFP_KERNEL);
	__kfifo_in(&(buff->rbuff), random_buf, 64);	
	buff->buff_task = kmalloc(sizeof(struct tasklet_struct), GFP_KERNEL);
	tasklet_init(buff->buff_task, buff_refill, task->pid);
}	

static inline void rbuffer_free(struct task_struct *task)
{
	struct pri_rbuff *buff = &(task->rbuff);
	__kfifo_free(&(buff->rbuff));
	kfree(buff->buff_task);
}

static void init_pri_struct(struct task_struct *task, int type){
	struct pri_struct *p = &(task->pri[type]);	
	p->index = 0;
	p->pri_current = 0;
	memset(p->original, 0, MAX_QUERY_LENGTH * 8);
	memset(p->obfuscated, 0, MAX_QUERY_LENGTH * 8);
	p->p_rbuff = &(task->rbuff);
}

void initialize_pri(struct task_struct *task)
{
//	u32 tmp = 1;
	int i;
	rbuffer_alloc(task);
	for(i = 0; i < PRI_SIZE; i++){
		init_pri_struct(task, i);
	}
}

void release_pri(struct task_struct *task)
{
	rbuffer_free(task);
}

static inline void refresh_original(struct pri_struct *pri, int index, long ori)
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

static long get_noise(struct pri_struct *pri)
{
	long noise;
	int length;
	struct pri_rbuff *p_rbuff = pri->p_rbuff;
	if(__kfifo_out(&(p_rbuff->rbuff), &noise, 1) != 0){
		length = (p_rbuff->rbuff).in - (p_rbuff->rbuff).out;
		if(length == (PRI_BUFFER_SIZE/2)){
			tasklet_hi_schedule(p_rbuff->buff_task);
		}
		return noise;
	}
	else{
		printk(KERN_INFO "Random Buffer is empty!\n");
		return 0;
	}
}

long get_obfuscation(struct task_struct *task, int type, long ori)
{
	struct pri_struct *pri;
	pri = &(task->pri[type]);
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

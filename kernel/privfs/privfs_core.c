#include <linux/sched.h>
#include <linux/types_privfs.h>
#include <linux/types.h>
#include <linux/laplace_pri.h>
#include <linux/pid.h>
#include <asm/i387.h>

#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

float lap[PRI_SIZE];

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

void buff_refill(unsigned long buff)
{
//	struct task_struct *task;
	struct __kfifo *buf;
	struct pri_rbuff *p_rbuff;
	long noise;

	printk(KERN_INFO "Refill random buffer!\n");
//	task = pid_task(find_vpid(pid), PIDTYPE_PID);
	p_rbuff = (struct pri_rbuff *)buff; 
	buf = &(p_rbuff->rbuff);
	kernel_fpu_begin();
	while((buf->in - buf->out) < PRI_BUFFER_SIZE){
		noise = get_laplace(0, (p_rbuff->b) * (p_rbuff->height));
		__kfifo_in(buf, &noise, 1);
	}	
	kernel_fpu_end();
}

static inline void rbuffer_alloc(struct pri_struct *pri, int type)
{
//	long noise;
	struct pri_rbuff *buff = &(pri->p_rbuff);

	buff->b = lap[type];
	__kfifo_alloc(&(buff->rbuff), PRI_BUFFER_SIZE, 8, GFP_KERNEL);
//	__kfifo_in(&(buff->rbuff), random_buf, 64);	
//	while(((buff->rbuff).in - (buff->rbuff).out) < PRI_BUFFER_SIZE){
//		noise = get_laplace(0, 200);
//		__kfifo_in(&(buff->rbuff), &noise, 1);
//	}
	buff->buff_task = kmalloc(sizeof(struct tasklet_struct), GFP_KERNEL);
	tasklet_init(buff->buff_task, buff_refill, (unsigned long) buff);
}	

static inline void rbuffer_free(struct pri_struct *pri)
{
	struct pri_rbuff *buff = &(pri->p_rbuff);
	__kfifo_free(&(buff->rbuff));
	kfree(buff->buff_task);
}

static void init_pri_struct(struct task_struct *task, int type)
{
	struct pri_struct *p = &(task->pri[type]);	
	p->index = 0;
	p->pri_current = 0;
	p->base_value = 0;
	p->last_value = 0;
	memset(p->original, 0, MAX_QUERY_LENGTH * 8);
	memset(p->obfuscated, 0, MAX_QUERY_LENGTH * 8);
	rbuffer_alloc(p, type);
//	p->p_rbuff = &(task->rbuff);
}

static void refresh_pri_struct(struct task_struct *task, int type)
{
	struct pri_struct *p = &(task->pri[type]);	
	p->index = 0;
	p->pri_current = 0;
	p->base_value = 0;
	p->last_value = 0;
	memset(p->original, 0, MAX_QUERY_LENGTH * 8);
	memset(p->obfuscated, 0, MAX_QUERY_LENGTH * 8);
}

void initialize_pri(struct task_struct *task)
{
//	u32 tmp = 1;
	int i;
	lap[STATM_FILE] = 200;
	lap[STATM_ANON] = 200;
	lap[STATM_TOTAL] = 200;
	lap[STATM_SHARED] = 200;
	lap[STATM_STACK] = 200;
	lap[CPU_UTIME] = 40;
	lap[CPU_STIME] = 40;
	lap[CPU_SWITCH] = 0.143;
//	rbuffer_alloc(task);
	for(i = 0; i < PRI_SIZE; i++){
		init_pri_struct(task, i);
	}
}

void release_pri(struct task_struct *task)
{
//	rbuffer_free(task);
	int i;
	struct pri_struct *pri;
	for(i = 0; i < PRI_SIZE; i++){
		pri = &(task->pri[i]);
		rbuffer_free(pri);
	}
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
	struct pri_rbuff *p_rbuff = &(pri->p_rbuff);
	struct __kfifo *buf = &(p_rbuff->rbuff);

	if(buf->in == buf->out){
		kernel_fpu_begin();
		while((buf->in - buf->out) < PRI_BUFFER_SIZE){
			noise = get_laplace(0, (p_rbuff->b) * (p_rbuff->height));
			__kfifo_in(buf, &noise, 1);
		}	
		kernel_fpu_end();
	}

	if(__kfifo_out(buf, &noise, 1) != 0){
		length = buf->in - buf->out;
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

long call_binary_tree(struct pri_struct *pri, unsigned int b_index, long ori)
{
	int i;
	int j = 1;
	int mark = 0; 
	long noisy_sum = 0;
	for(i = 0; i< MAX_QUERY_LENGTH; i++){
		if((j & b_index) != 0){
		/* find the first right bit which is not 0*/
			if(mark == 0){
				mark = 1;
				refresh_original(pri, i, ori);
				pri->obfuscated[i] = pri->original[i] + get_noise(pri);
				noisy_sum += pri->obfuscated[i];
			}
			else noisy_sum += pri->obfuscated[i];
		}
		j = j << 1;
	}
//	printk(KERN_INFO "Noisy Sum: %ld\n", noisy_sum);
	return noisy_sum;
}

int is_base(unsigned int index)
{
	int height = -1;
	int j = 1;
	int i;

	for(i = 0; i < sizeof(unsigned int) * 8; i++){
		if(j & index){
			if(height == -1) height = i + 1;
			else return -1;
		}
		j = j << 1;
	}

	return height;
}

long get_obfuscation(struct task_struct *task, int type, long ori)
{
	struct pri_struct *pri;
	struct pri_rbuff *p_rbuff;
	int height;
	long result;

	pri = &(task->pri[type]);
	p_rbuff = &(pri->p_rbuff);
	pri->index += 1;
	height = is_base(pri->index);

	if(height != -1){
//		printk(KERN_INFO "New tree in height %d!\n", height);
		pri->base_index = pri->index;
		kernel_fpu_begin();
		pri->base_noisy += ori - pri->base_value + get_laplace(0, p_rbuff->b);
		kernel_fpu_end();
		pri->base_value = ori;
		pri->pri_current = 0;
		(p_rbuff->rbuff).out = (p_rbuff->rbuff).in; // reset buffer
		p_rbuff->height = height;
		result = pri->base_noisy;
	}
	else{
		result = call_binary_tree(pri, pri->index - pri->base_index, ori - pri->base_value);
		result += pri->base_noisy;
	}

	return result;
}

void update_last(struct task_struct *task, int type, long last_val)
{
	struct pri_struct *pri;
	pri = &(task->pri[type]);
	pri->last_value = last_val;
}

long get_last(struct task_struct *task, int type)
{
	struct pri_struct *pri;
	pri = &(task->pri[type]);
	return pri->last_value;
}

void update_lap(struct task_struct *task, int type, float lap_val)
{
	struct pri_struct *pri;
	pri = &(task->pri[type]);
	refresh_pri_struct(task, type); // restart mechanism
	(pri->p_rbuff).b = lap_val;
}

float get_lap(struct task_struct *task, int type)
{
	struct pri_struct *pri;
	pri = &(task->pri[type]);
	return (pri->p_rbuff).b; 
}

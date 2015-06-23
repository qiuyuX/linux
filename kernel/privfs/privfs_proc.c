#include <linux/types_privfs.h>
#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/privfs_comm.h>

void pri_task_statm(struct task_struct *task, struct mm_struct *mm, long *noisy, long *optimal)
{
	long data[STATM_LEN];
	long *opt; //optimization result

	printk(KERN_INFO "Start obfuscating...\n");
	data[STATM_FILE] = get_obfuscation(task, STATM_FILE, get_mm_counter(mm, MM_FILEPAGES));
	data[STATM_ANON] = get_obfuscation(task, STATM_ANON, get_mm_counter(mm, MM_ANONPAGES));
	data[STATM_TOTAL] = get_obfuscation(task, STATM_TOTAL, mm->total_vm);
	data[STATM_SHARED] = get_obfuscation(task, STATM_SHARED, mm->shared_vm);
	// noisy results
	noisy[0] = data[STATM_TOTAL];
	noisy[1] = data[STATM_FILE] + data[STATM_ANON];
	noisy[2] = data[STATM_FILE];
	noisy[3] = (PAGE_ALIGN(mm->end_code) - (mm->start_code & PAGE_MASK)) >> PAGE_SHIFT;
	noisy[4] = data[STATM_TOTAL] - data[STATM_SHARED];	
	
	printk(KERN_INFO "Wait for optimiation..\n");
	// get optimization
	opt = statm_get_optimization(data);		

	printk(KERN_INFO "Get optimal results.\n");
	// optimal resuls
	optimal[0] = opt[STATM_TOTAL];
	optimal[1] = opt[STATM_FILE] + opt[STATM_ANON];
	optimal[2] = opt[STATM_FILE];
	optimal[3] = (PAGE_ALIGN(mm->end_code) - (mm->start_code & PAGE_MASK)) >> PAGE_SHIFT;
	optimal[4] = opt[STATM_TOTAL] - opt[STATM_SHARED];	
}

void pri_task_statm_noisy(struct task_struct *task, struct mm_struct *mm, long *noisy)
{
	long data[STATM_LEN];

	data[STATM_FILE] = get_obfuscation(task, STATM_FILE, get_mm_counter(mm, MM_FILEPAGES));
	data[STATM_ANON] = get_obfuscation(task, STATM_ANON, get_mm_counter(mm, MM_ANONPAGES));
	data[STATM_TOTAL] = get_obfuscation(task, STATM_TOTAL, mm->total_vm);
	data[STATM_SHARED] = get_obfuscation(task, STATM_SHARED, mm->shared_vm);
	
	noisy[0] = data[STATM_TOTAL];
	noisy[1] = data[STATM_FILE] + data[STATM_ANON];
	noisy[2] = data[STATM_FILE];
	noisy[3] = (PAGE_ALIGN(mm->end_code) - (mm->start_code & PAGE_MASK)) >> PAGE_SHIFT;
	noisy[4] = data[STATM_TOTAL] - data[STATM_SHARED];	
}

void pri_task_statm_optimal(struct task_struct *task, struct mm_struct *mm, long *optimal)
{
	long data[STATM_LEN];
	long *opt; //optimization result

	printk(KERN_INFO "Start obfuscating...\n");
	data[STATM_FILE] = get_obfuscation(task, STATM_FILE, get_mm_counter(mm, MM_FILEPAGES));
	data[STATM_ANON] = get_obfuscation(task, STATM_ANON, get_mm_counter(mm, MM_ANONPAGES));
	data[STATM_TOTAL] = get_obfuscation(task, STATM_TOTAL, mm->total_vm);
	data[STATM_SHARED] = get_obfuscation(task, STATM_SHARED, mm->shared_vm);
	
	printk(KERN_INFO "Wait for optimiation..\n");
	opt = statm_get_optimization(data);		

	printk(KERN_INFO "Get optimal results.\n");
	optimal[0] = opt[STATM_TOTAL];
	optimal[1] = opt[STATM_FILE] + opt[STATM_ANON];
	optimal[2] = opt[STATM_FILE];
	optimal[3] = (PAGE_ALIGN(mm->end_code) - (mm->start_code & PAGE_MASK)) >> PAGE_SHIFT;
	optimal[4] = opt[STATM_TOTAL] - opt[STATM_SHARED];	
}

void pri_task_cpu_optimal(struct task_struct *task, unsigned long utime, unsigned long stime, unsigned long long uptime, unsigned long long starttime, unsigned long gtime, unsigned long cutime, unsigned long cstime, unsigned long *utime_opt, unsigned long *stime_opt)
{
	long data[5]; // utime_noisy, stime_noisy, utime_old, stime_old, (uptime - starttime - gtime - cutime - cstime)
	long *opt;

	data[0] = get_obfuscation(task, CPU_UTIME, utime);
	data[1] = get_obfuscation(task, CPU_STIME, stime);
	data[2] = get_last(task, CPU_UTIME);
	data[3] = get_last(task, CPU_STIME);
	data[4] = uptime - starttime - gtime - cutime - cstime;  

	opt = cpu_get_optimization(data);

	*utime_opt = opt[0];
	*stime_opt = opt[1];	

	update_last(task, CPU_UTIME, (long)*utime_opt);	
	update_last(task, CPU_STIME, (long)*stime_opt);	
}

long pri_cpu_switch(struct task_struct *task, unsigned long nvcsw)
{
	long noised;
	long old;

	noised = get_obfuscation(task, CPU_SWITCH, nvcsw);
	old = get_last(task, CPU_SWITCH);

	if(noised <= old){
		return old;
	}
	else{
		update_last(task, CPU_SWITCH, noised);
		return noised;
	}
}

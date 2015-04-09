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

	data[STATM_FILE] = get_obfuscation(task, STATM_FILE, get_mm_counter(mm, MM_FILEPAGES));
	data[STATM_ANON] = get_obfuscation(task, STATM_ANON, get_mm_counter(mm, MM_ANONPAGES));
	data[STATM_TOTAL] = get_obfuscation(task, STATM_TOTAL, mm->total_vm);
	data[STATM_SHARED] = get_obfuscation(task, STATM_SHARED, mm->shared_vm);
	
	opt = statm_get_optimization(data);		

	optimal[0] = opt[STATM_TOTAL];
	optimal[1] = opt[STATM_FILE] + opt[STATM_ANON];
	optimal[2] = opt[STATM_FILE];
	optimal[3] = (PAGE_ALIGN(mm->end_code) - (mm->start_code & PAGE_MASK)) >> PAGE_SHIFT;
	optimal[4] = opt[STATM_TOTAL] - opt[STATM_SHARED];	
}

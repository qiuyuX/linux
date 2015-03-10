#include <linux/types_privfs.h>
#include <linux/mm.h>

long pri_task_statm(struct task_struct *task, struct mm_struct *mm, long *pri_shared, long *pri_text, long *pri_data, long *pri_resident){
	long file, anon, total, shared;
	file = get_obfuscation(task, 0, get_mm_counter(mm, MM_FILEPAGES));
	anon = get_obfuscation(task, 1, get_mm_counter(mm, MM_ANONPAGES));
	total = get_obfuscation(task, 2, mm->total_vm);
	shared = get_obfuscation(task, 3, mm->shared_vm);

	*pri_shared = file;
	*pri_text = (PAGE_ALIGN(mm->end_code) - (mm->start_code & PAGE_MASK)) >> PAGE_SHIFT;
	*pri_data = total - shared;
	*pri_resident = file + anon;
	return total;
}

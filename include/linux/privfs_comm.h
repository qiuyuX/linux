#ifndef __PRIVFS_COMM_H_
#define __PRIVFS_COMM_H_
extern long * statm_get_optimization(long *arr);
extern long * cpu_get_optimization(long *arr);
extern void pri_task_statm(struct task_struct *task, struct mm_struct *mm, long *noisy, long *optimal);
extern void pri_task_statm_noisy(struct task_struct *task, struct mm_struct *mm, long *noisy);
extern void pri_task_statm_optimal(struct task_struct *task, struct mm_struct *mm, long *optimal);
extern void pri_task_cpu_optimal(struct task_struct *task, unsigned long utime, unsigned long stime, unsigned long long uptime, unsigned long long starttime, unsigned long gtime, unsigned long cutime, unsigned long cstime, unsigned long *utime_opt, unsigned long *stime_opt);
extern long pri_cpu_switch(struct task_struct *task, unsigned long nvcsw);

#define NETLINK_PRIVFS 31
#define MSG_BUFF_SIZE 10
#define STATM_LEN 4
#define CPU_LEN 5
#define MSG_LEN 7
#define STATM_TYPE 0
#define CPU_TYPE 1
#define PID 12345 

#endif

#ifndef __PRIVFS_COMM_H_
#define __PRIVFS_COMM_H_
extern long * statm_get_optimization(long *arr);
extern void pri_task_statm(struct task_struct *task, struct mm_struct *mm, long *noisy, long *optimal);

#define NETLINK_PRIVFS 31
#define MSG_BUFF_SIZE 10
#define STATM_LEN 4
#define PID 12345 

#endif

#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/privfs_comm.h>

static long statm_result[MSG_BUFF_SIZE][STATM_LEN];
static int statm_flag[MSG_BUFF_SIZE]; // show the availability of result set
static int last_statm_msg;
static struct sock *nl_sk = NULL;
static wait_queue_head_t statm_queue;


// for privfs 
static inline void privfs_data_init(void)
{
	memset(&statm_flag, 0, MSG_BUFF_SIZE * sizeof(int));
	last_statm_msg = -1;
	init_waitqueue_head(&statm_queue);
}

static void statm_send_msg(long *arr, int num)
{
	struct nlmsghdr *nlh;
	struct sk_buff *skb_out;
	int msg_size = (STATM_LEN + 1) * sizeof(long);
	long *msg;
	int res, i;

	skb_out = nlmsg_new(msg_size, 0);	
	if(!skb_out){
		printk(KERN_ERR "Failed to allocate new skb!\n");
		return;
	}
	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
	NETLINK_CB(skb_out).dst_group = 0;
	
	msg = (long *)nlmsg_data(nlh);
	for(i = 0; i < STATM_LEN; i++){
		msg[i] = arr[i];
	}
	msg[STATM_LEN] = num;
	
	res = nlmsg_unicast(nl_sk, skb_out, PID);
	if(res < 0)
		printk(KERN_INFO "Error when sending msg!\n");
}

long * statm_get_optimization(long *arr)
{
	int msg_num = (last_statm_msg + 1) % MSG_BUFF_SIZE;		
	if(statm_flag[msg_num] == 0){
		statm_flag[msg_num] = 1;
		last_statm_msg = msg_num;
		statm_send_msg(arr, msg_num);
		wait_event_interruptible(statm_queue, statm_flag[msg_num] != 1);
//		printk(KERN_INFO "Wake up!\n");
		return statm_result[msg_num];
	}
	else //buff is full
		return NULL;
}
EXPORT_SYMBOL(statm_get_optimization);

// for netlink communication

static void privfs_nl_recv_msg(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;
	long *msg;
	int i;

	nlh = (struct nlmsghdr *) skb->data;
	msg = (long *)nlmsg_data(nlh);
	for(i = 0; i < STATM_LEN; i++){
		statm_result[msg[STATM_LEN]][i] = msg[i];
	}
	statm_flag[msg[STATM_LEN]] = 0;
	wake_up_interruptible(&statm_queue);
}

struct netlink_kernel_cfg cfg = {
	.groups = 1,
	.input = privfs_nl_recv_msg,
};

static inline void privfs_nl_create(void)
{
	nl_sk = netlink_kernel_create(&init_net, NETLINK_PRIVFS, &cfg);
	if(!nl_sk){
		printk(KERN_ALERT "Can't create socket!\n");
		return -10;
	}
}


static int __init privfs_comm_init(void)
{
	privfs_data_init();
	privfs_nl_create();
	return 0;
}

static void __exit privfs_comm_exit(void)
{
	netlink_kernel_release(nl_sk);
}

module_init(privfs_comm_init);
module_exit(privfs_comm_exit);
MODULE_LICENSE("GPL");


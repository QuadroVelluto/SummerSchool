#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/net_namespace.h>

#define NETLINK_USER 31

struct sock *nl_sk = NULL;

static void echo_nl_recv_msg(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;
	int pid;
	struct sk_buff *skb_out;
	int msg_size;
	char *msg;
	int res;

	printk("Entering: %s\n", __FUNCTION__);

	nlh = (struct nlmsghdr *)skb->data;
	msg = (char *)nlmsg_data(nlh);
	msg_size = strlen(msg);

	printk("Netlink received msg payload: %s\n", msg);

	pid = nlh->nlmsg_pid;

	skb_out = nlmsg_new(msg_size, 0);
	if (!skb_out)
	{
		printk("Failed to allocate new skb\n");
		return;
	}

	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
	NETLINK_CB(skb_out).dst_group = 0;
	strncpy(nlmsg_data(nlh), msg, msg_size);

	res = nlmsg_unicast(nl_sk, skb_out, pid);

	if (res < 0)
		printk("Error while sending back to user\n");
}

struct netlink_kernel_cfg cfg =
	{
		.groups = 1,
		.input = echo_nl_recv_msg,
};

static int __init echo_init(void)
{

	printk("Entering: %s\n", __FUNCTION__);
	nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);

	if (!nl_sk)
	{
		printk("Error creating socket.\n");
		return -1;
	}

	return 0;
}

static void __exit echo_exit(void)
{
	printk("Exiting echo module\n");
	netlink_kernel_release(nl_sk);
}
module_init(echo_init);
module_exit(echo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anton Shvedunov");
MODULE_DESCRIPTION("Module for lab5");
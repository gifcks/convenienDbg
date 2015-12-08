#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <linux/kthread.h>

#define MAX_MSGSIZE 4096 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("WHO");

struct sock *nl_sk = NULL;
static struct task_struct *mythread = NULL; 

void sendnlmsg(char *message)
{
    struct sk_buff *skb;
    struct nlmsghdr *nlh;
    int slen = 0;

    if(!message || !nl_sk){
        return;
    }

    skb = nlmsg_new(MAX_MSGSIZE, GFP_KERNEL);
    if(!skb){
        printk(KERN_ERR "my_net_link: alloc_skb Error./n");
        return;
    }

    slen = strlen(message)+1;

    nlh = nlmsg_put(skb, 0, 0, 0, MAX_MSGSIZE, 0);

    NETLINK_CB(skb).portid = 0; 
    NETLINK_CB(skb).dst_group = 5; 

    memcpy(NLMSG_DATA(nlh), message, slen);

    netlink_broadcast(nl_sk, skb, 0,5, GFP_KERNEL); 
    printk(KERN_CRIT "send OK!\n");
    return;
}

static void recnldata (struct sk_buff *__skb)
{
    struct nlmsghdr *nlh = NULL;
    struct sk_buff *skb;
    char *mydata;
    skb = skb_get(__skb);
    
    if(skb->len >= NLMSG_SPACE(0))
    {
          nlh = (struct nlmsghdr *)skb->data;
          mydata = (char*)NLMSG_DATA(nlh);
          printk(KERN_CRIT "%s:received netlink message payload: %s \n",__FUNCTION__,mydata);
          kfree_skb(skb);
    }
    printk(KERN_CRIT "recvied finished!\n");
}

static int sending_thread(void *data)
{
     int i = 10;
     struct completion cmpl;
     while(i--){
            init_completion(&cmpl);
            wait_for_completion_timeout(&cmpl, 1 * HZ);
            sendnlmsg("hello userspace!");
     }
     printk(KERN_CRIT"sending thread exited!");
     return 0;
}

static int __init myinit_module(void)
{
    struct netlink_kernel_cfg netlink_kerncfg = {
           .input = recnldata,
    };
    printk(KERN_CRIT "my netlink in\n");
    nl_sk = netlink_kernel_create(&init_net,NETLINK_TEST,&netlink_kerncfg);

    if(!nl_sk){
        printk(KERN_ERR "my_net_link: create netlink socket error.\n");
        return 1;
    }

    printk(KERN_CRIT "my netlink: create netlink socket ok.\n");
    mythread = kthread_run(sending_thread,NULL,"thread_sender");
    return 0;
}

static void __exit mycleanup_module(void)
{
    if(nl_sk != NULL){
        sock_release(nl_sk->sk_socket);
    }
printk(KERN_CRIT "my netlink out!\n");
}

module_init(myinit_module);
module_exit(mycleanup_module);

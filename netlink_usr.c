#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/types.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>

#define MAX_PAYLOAD 4096

unsigned int netlink_group_mask(unsigned int group){
    return group ? 1<<(group-1) : 0;
}

int send_to_kernel(char *data){
    int sock_fd = -1;
    struct sockaddr_nl sock_addr;
    struct nlmsghdr *nlh = NULL;
    struct msghdr msg;
    struct iovec iov;
    int ret;
    int len = strlen(data);
    char *tempdata = (char*)malloc(len*4);
    strcpy(tempdata,data);

    sock_fd = socket(PF_NETLINK,SOCK_RAW,22); 
    if(sock_fd == -1){
        perror("can't create netlink socket!\n");
        return -1;
    }
    
    memset(&sock_addr,0,sizeof(sock_addr));
    sock_addr.nl_family = AF_NETLINK;
    sock_addr.nl_pid = 0;
    sock_addr.nl_groups = netlink_group_mask(5);

    ret = bind(sock_fd,(struct sockaddr *)&sock_addr,sizeof(sock_addr));
    if(ret < 0){
        printf("bind failed: %s\n",strerror(errno));
        close(sock_fd);
        return -1;
    }
    
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if(nlh == NULL){
        perror("alloc mem failed\n");
        return 1;
    }

    memset(nlh,0,MAX_PAYLOAD);
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_type = NLMSG_NOOP;
    nlh->nlmsg_flags = 0;
    
    strcpy(NLMSG_DATA(nlh),tempdata);

    memset(&iov,0,sizeof(iov));
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;

    memset(&msg,0,sizeof(msg));
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    sendmsg(sock_fd,&msg,0);
    
    free(nlh);
    free(tempdata);
    return 0;
}

int rec_from_kernel(char** data){
    int sock_fd = -1;
    struct sockaddr_nl sock_addr;
    struct nlmsghdr *nlh = NULL;
    struct msghdr msg;
    struct iovec iov;
    int ret,rec;
    
    sock_fd = socket(PF_NETLINK,SOCK_RAW,22); 
    if(sock_fd == -1){
        perror("can't create netlink socket!\n");
        return -1;
    }
    
    memset(&sock_addr,0,sizeof(sock_addr));
    sock_addr.nl_family = AF_NETLINK;
    sock_addr.nl_pid = 0;
    sock_addr.nl_groups = netlink_group_mask(5);

    ret = bind(sock_fd,(struct sockaddr *)&sock_addr,sizeof(sock_addr));
    if(ret < 0){
        printf("bind failed: %s\n",strerror(errno));
        close(sock_fd);
        return -1;
    }
    
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if(nlh == NULL){
        perror("alloc mem failed\n");
        return -1;
    }

    memset(nlh,0,MAX_PAYLOAD);
    iov.iov_base = (void *)nlh;
    iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);

    memset(&msg,0,sizeof(msg));
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    rec = recvmsg(sock_fd,&msg,0);
    
    *data = (char *)NLMSG_DATA(nlh);

    free(nlh);
    close(sock_fd);
    return 0;
}

int main(void){
    char* control_data = "userspace is ready for data";
    char* mydata = NULL;
    int a,b,c;
    
    while(1)
    {
    a = rec_from_kernel(&mydata);
    if(a == -1){
        printf("rec from kernel failed\n");
        return -1;
    }
    else{
        printf("rec from kernel success,data is:%s\n",mydata);
        b = send_to_kernel(mydata);
        if(b == 0){
            printf("send to kernel success\n");
            //return 0;
            mydata = NULL;
        }
        else{
            printf("send to kernel failed\n");
            return -1;
            mydata = NULL;
        }
    }
    }
}

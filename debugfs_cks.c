#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <asm/uaccess.h>

static struct dentry *my_debugfs_root;
static u8 a = 0;
static char mystr[32] = "caikaisheng.org";
static struct debugfs_blob_wrapper b;

static int dbg_open(struct inode *inode,struct file *filp)
{
	printk(KERN_CRIT "[CKS] FUNCTION : %s LINE : %d\n",__func__,__LINE__);
	filp->private_data = inode->i_private;
	return 0;
}

static ssize_t dbg_read(struct file *filp,char __user *buffer,
						size_t count,loff_t *ppos)
{
	
	printk(KERN_CRIT "[CKS] FUNCTION : %s LINE : %d count :%ld\n",__func__,__LINE__,count);
	if(*ppos >= 32)
		return 0;
	if(*ppos+count > 32)
		count=32-*ppos;
	printk(KERN_CRIT "count : %ld LINE : %d\n", count,__LINE__);
	if(copy_to_user(buffer,mystr + *ppos,count))
		return -EFAULT;
	*ppos += count;
	printk(KERN_CRIT "count : %ld LINE : %d\n", count,__LINE__);
	return count;
}
static ssize_t dbg_write(struct file *filp,char __user *buffer,
						size_t count,loff_t *ppos)
{
	printk(KERN_CRIT "[CKS] FUNCTION : %s LINE : %d,count : %ld \n",__func__,__LINE__,count);
	if(*ppos >= 32)
		return 0;
	if(*ppos+count > 32)
		count=32-*ppos;
	if(copy_from_user(mystr + *ppos,buffer,count))
		return -EFAULT;
	*ppos+=count;
	return count;
}

static struct file_operations data_fops={
	.owner = THIS_MODULE,
	.open = dbg_open,
	.read = dbg_read,
	.write = dbg_write,
};

static int __init mydbgfs_init(void)
{
	struct dentry *sub_dir,*r_a,*r_b,*s_c;
	printk(KERN_CRIT "[cks] mydbgfs init\n");
	my_debugfs_root = debugfs_create_dir("mydir",NULL);
	if(!my_debugfs_root)
		return -ENOENT;
	r_a = debugfs_create_u8("u8file",0644,my_debugfs_root,&a);
	if(!r_a)
		goto fail;
	b.data = (void *)mystr;
	b.size = strlen(mystr)+1;
	r_b = debugfs_create_blob("blobfile",0644,my_debugfs_root,&b);
	if(!r_b)
		goto fail;
	sub_dir = debugfs_create_dir("subdir",my_debugfs_root);
	if(!r_b)
		goto fail;
	s_c = debugfs_create_file("datafile",0644,sub_dir,NULL,&data_fops);
		if(!s_c)
			goto fail;
	return 0;

fail:
	debugfs_remove_recursive(my_debugfs_root);
	my_debugfs_root = NULL;
	return -ENOENT;
}

static void __exit mydbgfs_exit(void)
{
	printk(KERN_CRIT "[CKS] MYDEBGFS_EXIT\n");
	debugfs_remove_recursive(my_debugfs_root);
}
module_init(mydbgfs_init);
module_exit(mydbgfs_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("CKS");
MODULE_DESCRIPTION("debugfs file enumeration module");






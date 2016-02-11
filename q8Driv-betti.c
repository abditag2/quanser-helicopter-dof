
/* Be careful!! This patch supports only one device! */

#include "q8user-betti.h"

#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/module.h>

//#define DRIVER_NAME "Q8Drv"

int Q8_EncReadSingleLatch (int BoardNum, int nChannel);
void Q8_EncResetSingle (int BoardNum, int nChannel);
int Q8_EncInputSingle (int BoardNum, int nChannel);
void Q8_SetDAC(int BoardNum, int nChannels, int *v);
void Q8_EncInput (int BoardNum, int nChannels, int *pnValues);

struct chardev_struct {
	dev_t dev;     /* first major/minor number owned by the driver */
	unsigned int major;     /* major number */
	struct cdev cdev;
} chardev;

/* init for chardev struct, called in module init function */
#define init_chardev_struct(_cd) do {                          \
	memset((char*)(_cd),0,sizeof(struct chardev_struct));  \
}while(0)


static
int q8_open(struct inode * inode, struct file * filp)
{
	struct chardev_struct *s;
	s = container_of(inode->i_cdev, struct chardev_struct, cdev);
	filp->private_data = s;

	nonseekable_open(inode,filp);
	return 0;
}


static int q8_release(struct inode * inode, struct file * filp)
{
	return 0;
}

static
long q8_unlocked_ioctl(struct file *filp,
			unsigned int cmd, unsigned long arg)
{
	/*struct chardev_struct *s = (struct chardev_struct*)filp->private_data;*/

	/* Check command */
	if(_IOC_TYPE(cmd) != Q8_IOCTL_MAGIC)
		return -ENOTTY;
	if(_IOC_NR(cmd) < Q8_IOCTL_MINNR ||
			_IOC_NR(cmd) > Q8_IOCTL_MAXNR)
		return -ENOTTY;

	/* Check argument:
	   After this check, we safely use:
	   __copy_to_user or __copy_from_user
	   __put_user or __get_user          */
	if(_IOC_DIR(cmd) & _IOC_READ) {
		/* Read request, so check if I can write to user space */
		if(!(access_ok(VERIFY_WRITE,(void __user *)arg,_IOC_SIZE(cmd))))
			return -EFAULT;
	}
	else if(_IOC_DIR(cmd) & _IOC_WRITE) {
		/* Write request, so check if I can read from user space */
		if(!(access_ok(VERIFY_READ,(void __user *)arg,_IOC_SIZE(cmd))))
			return -EFAULT;
	}

	switch (cmd) {
		case Q8_IOCTL_EXAMPLE1:
		{
			/* No read or write to/from user space */
			printk(KERN_INFO DRIVER_NAME ": ioctl example 1\n");
		}
		break;
		case Q8_IOCTL_EXAMPLE2:
		{
			/* write to user space */
			printk(KERN_INFO DRIVER_NAME ": ioctl example 2\n");
			if(__copy_to_user((char*)arg,"example 2",_IOC_SIZE(cmd))!=0)
				return -EFAULT;
		}
		break;
		case Q8_IOCTL_EXAMPLE3:
		{
			/* read from user space */
			unsigned int temp = (unsigned int)arg;
			printk(KERN_INFO DRIVER_NAME ": ioctl example 3 (int=%d)\n", temp);
		}
		break;
		case Q8_RD_ENC1:
		{
			int tmp;
			/*tmp = Q8_EncInputSingle(0,0);*/
			tmp = Q8_EncReadSingleLatch(0,0);
			/* write to user space */
			printk(KERN_INFO DRIVER_NAME ": Reading encoder org = %x \n", tmp);
			if(__copy_to_user((char*)arg,&tmp,_IOC_SIZE(cmd))!=0)
				return -EFAULT;
		}
		break;
		case Q8_WR_DAC:
		{
			int tmp = 1 | 2 | 4 | 8;
			Q8_SetDAC(0, tmp, (int*)arg);
		}
		break;
		case Q8_RD_ENCS:
		{
			int tmp[3];
			tmp[0] = Q8_EncInputSingle(0, 0);
			tmp[1] = Q8_EncInputSingle(0, 1);
			tmp[2] = Q8_EncInputSingle(0, 2);
			if(__copy_to_user((char*)arg, &tmp, _IOC_SIZE(cmd))!=0) 
				return -EFAULT;
		}
		break;
		case Q8_WR_DACS:
		{
			int tmp = 1 | 2;
			Q8_SetDAC(0, tmp, (int*)arg);
		}
		break;
		case Q8_ENC_0:
		{
			int tmp;
			tmp = Q8_EncInputSingle(0,0);
			/* write to user space */
			printk(KERN_INFO DRIVER_NAME ": Reading encoder 0 = %x \n", tmp);
			if(__copy_to_user((char*)arg,&tmp,_IOC_SIZE(cmd))!=0)
				return -EFAULT;
		}
		break;
		case Q8_ENC_1:
		{
			int tmp;
			tmp = Q8_EncInputSingle(0,1);
			/* write to user space */
			printk(KERN_INFO DRIVER_NAME ": Reading encoder 1 = %x \n", tmp);
			if(__copy_to_user((char*)arg,&tmp,_IOC_SIZE(cmd))!=0)
				return -EFAULT;
		}
		break;
		case Q8_ENC_2:
		{
			int tmp;
			tmp = Q8_EncInputSingle(0,2);
			/* write to user space */
			printk(KERN_INFO DRIVER_NAME ": Reading encoder 2 = %x \n", tmp);
			if(__copy_to_user((char*)arg,&tmp,_IOC_SIZE(cmd))!=0)
				return -EFAULT;
		}
		case Q8_ENC:
		{
			int tmp[3];
			Q8_EncInput(0,1|2|4,tmp);
			/* write to user space */
			//printk(KERN_INFO DRIVER_NAME ": Reading encoder 2 = %x \n", tmp);
			if(__copy_to_user((char*)arg,tmp,_IOC_SIZE(cmd))!=0)
				return -EFAULT;
		}
		break;
		case Q8_DAC_0:
		{
			int tmp = 1;
			Q8_SetDAC(0, tmp, (int*)arg);
		}
		break;
		case Q8_DAC_1:
		{
			int tmp = 2;
			Q8_SetDAC(0, tmp, (int*)arg);
		}
		break;
		default:
			return -ENOTTY; /* cmd not valid */
	}
	return 0; /* Success */
}

struct file_operations q8_fops = {
	.owner = THIS_MODULE,
	.read = NULL,
	.write = NULL,
	.fasync = NULL,
	.unlocked_ioctl = q8_unlocked_ioctl,
	.open = q8_open,
	.release = q8_release,
	.llseek = no_llseek
};

int chardev_init(void)
{
	int rc;

/*
	if(Boards.Count !=1) {
		printk(KERN_INFO DRIVER_NAME ": no char device created (boards: %d)\n", Boards.Count);
		return 0;
	}
*/

	rc = alloc_chrdev_region(&(chardev.dev), 0, 1, DRIVER_NAME);
	if (rc < 0) {
		printk(KERN_ERR DRIVER_NAME ": cannot register the device file\n");
		return -ENOMEM;
	}

	chardev.major = MAJOR(chardev.dev);
	cdev_init(&chardev.cdev, &q8_fops);
	chardev.cdev.owner = THIS_MODULE;

	rc=cdev_add(&chardev.cdev, MKDEV(chardev.major, 0), 1);
	if (rc) {
		printk(KERN_ERR DRIVER_NAME ": cannot add cdev=(%d,0)\n", chardev.major);
		goto out;
	}
	printk(KERN_INFO DRIVER_NAME ": Major number %d\n", chardev.major);
	return 0;
out:
	unregister_chrdev_region(chardev.dev, 1);
	return rc;
}

void chardev_clean(void)
{
	cdev_del(&chardev.cdev);
	unregister_chrdev_region(chardev.dev, 1);
}

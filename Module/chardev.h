#ifndef CHARDEV_H
#define CHARDEV_H

#include <linux/ioctl.h>




//switch (ioctl_num)
#define IOCTL_GET_INF	1
#define IOCTL_GET_IFS	2
#define IOCTL_SET_MAC	3
#define IOCTL_SET_IP	4
//flag for interface info
#define I_MAC 1
#define I_STAT 2
#define I_IPv4 4
#define I_IPv6 8

#define DEVICE_FILE_NAME "/dev/ALP"
#define DEVICE_NAME "ALP"

#define BUF_SIZE 100

#endif

struct cdev *kernel_cdev; 
struct device *chr_dev; 
static int Major;
static struct class *dev_Class;
	
struct kern_usr_transfer{
	char msg_from_kern[100];
	char interface[16];
	char new_mac[17];
	char new_addr[12];
	int new_mask;
	int flag;
	unsigned char ip6[16];
}kern_usr_transfer;
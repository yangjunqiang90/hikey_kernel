#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/signal.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/pci.h>


#define DEVICE_NUMBER 1
static struct class * hello_class;
static struct device * hello_class_dev;

struct hello_device
{
	dev_t devno;
	struct cdev cdev;
}my_device;


static int hello_open(struct inode *inode, struct file *file)
{
	printk("driver: hello_open\n");
	return 0;
}

int hello_close(struct inode *inode, struct file *file)
{
	printk("driver: hello_close\n");
	return 0;
}

long hello_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	if(_IOC_TYPE (cmd) != 'F')
	{
		printk("param error !function:%s line:%d \n ",__FUNCTION__,__LINE__);
		return -1;
	}
	
	switch(_IOC_DIR (cmd)){
		case _IOC_READ :
				return 0;

		case _IOC_WRITE : 
				return 0;
		default: 
				printk("param error !function:%s line:%d \n ",__FUNCTION__,__LINE__);
				return -1;
	}
	
}

static int count,bridge_count,root_count;
static struct file_operations hello_fops = {
	.owner   		=  THIS_MODULE,    
	.open   		=  hello_open,     
	.release 		=  hello_close,
	.unlocked_ioctl =  hello_unlocked_ioctl,
};

void print_child_of_father(struct pci_bus *bus){
	struct list_head *child_pos;
	struct list_head *dev_pos;
	struct pci_bus *child_bus;
	int child_device_count=0;
	struct list_head *father_pos;
	int father_device_count=0;

	list_for_each(father_pos,&bus->devices){
		father_device_count++;
	}
	

	list_for_each(child_pos,&bus->children){
		child_device_count=0;
		child_bus=container_of(child_pos,struct pci_bus,node);
		list_for_each(dev_pos,&child_bus->devices){
			child_device_count++;
		}
		printk("the father of child(%d) %d is %d(%d)\n",child_device_count,child_bus->number,bus->number,father_device_count);
		print_child_of_father(child_bus);
	}
}

void print_pci_info(struct pci_bus *bus){
	struct list_head *pos_dev;
	struct pci_dev *dev;

	struct list_head *child_pos;
	struct pci_bus *child_bus;

	list_for_each(child_pos,&bus->children){
		child_bus=container_of(child_pos,struct pci_bus,node);
		print_pci_info(child_bus);
	}

	list_for_each(pos_dev,&bus->devices){
		dev= container_of(pos_dev,struct pci_dev,bus_list);
		//printk("dev number is [bus:dev:fun} {%x %x.%x} vendor id is %d, device id id %d . is multifunction %d\n",bus->number,dev->devfn>>3,dev->devfn&0x7,dev->vendor,dev->device,dev->multifunction);
		//printk("[bus:dev:fun} {%x %x.%x} vendor id is %d, device id id %d . is pci_dev->dev.obj.name : %s\n",bus->number,dev->devfn>>3,dev->devfn&0x7,dev->vendor,dev->device,dev->dev.kobj.name);
		printk("%2.0d  [bus:dev:fun} {%2x %2x.%-2x} vendor id is 0x%x, device id is 0x%x ",count,bus->number,dev->devfn>>3,dev->devfn&0x7,dev->vendor,dev->device);

		//printk("resource number is %ld ",sizeof(dev->resource)/sizeof(dev->resource[0]));
		if(dev->hdr_type==0){
			printk("device type is normal\n");
		}else if(dev->hdr_type==1){
			printk("device type is bridge\n");
			bridge_count++;
		}else{
			printk("I don't the device type!\n");
		}

		count++;
	}

}


static int hello_drv_init(void)
{
	int ret;
	struct list_head *pos_bus;
	struct pci_bus *bus;
	ret=alloc_chrdev_region(&my_device.devno,0,DEVICE_NUMBER,"hello");
	if (ret < 0) {
		printk("failed: register_chrdev_region\n");
		return ret;
	}

	cdev_init(&my_device.cdev, &hello_fops);
	ret = cdev_add(&my_device.cdev, my_device.devno, DEVICE_NUMBER);
	if (ret < 0) {
		printk("faield: cdev_add\n");
		return ret;
	}
	
	hello_class = class_create(THIS_MODULE, "hello_class");
	hello_class_dev = device_create(hello_class, NULL, my_device.devno, NULL, "hello_device"); 

	
	list_for_each(pos_bus,&pci_root_buses){
		bus = container_of(pos_bus,struct pci_bus,node);
		printk("root bus number is :0x%x\n",bus->number);
		print_pci_info(bus);
		print_child_of_father(bus);
		root_count++;
	}

	printk("root bus count:%d device count:%d bridge count:%d normal count:%d\n",root_count,count,bridge_count,count-bridge_count);

	return 0;
}

static void hello_drv_exit(void)
{
	device_destroy(hello_class,my_device.devno);
	class_destroy(hello_class);
		
	cdev_del(&(my_device.cdev));
	unregister_chrdev_region(my_device.devno,DEVICE_NUMBER);
}


module_init(hello_drv_init);
module_exit(hello_drv_exit);

MODULE_LICENSE("GPL");


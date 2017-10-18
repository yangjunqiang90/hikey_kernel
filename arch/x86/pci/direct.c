/*
 * direct.c - Low-level direct PCI config space access
 */

#include <linux/pci.h>
#include <linux/init.h>
#include <linux/dmi.h>
#include <asm/pci_x86.h>
 
/*
 * Functions for accessing PCI base (first 256 bytes) and extended
 * (4096 bytes per PCI function) configuration space with type 1
 * accesses.
 */

#define PCI_CONF1_ADDRESS(bus, devfn, reg) \
	(0x80000000 | ((reg & 0xF00) << 16) | (bus << 16) \
	| (devfn << 8) | (reg & 0xFC))

static int pci_conf1_read(unsigned int seg, unsigned int bus,
			  unsigned int devfn, int reg, int len, u32 *value)
{
	unsigned long flags;

	if (seg || (bus > 255) || (devfn > 255) || (reg > 4095)) {
		*value = -1;
		return -EINVAL;
	}

	raw_spin_lock_irqsave(&pci_config_lock, flags);

	outl(PCI_CONF1_ADDRESS(bus, devfn, reg), 0xCF8);

	switch (len) {
	case 1:
		*value = inb(0xCFC + (reg & 3));
		break;
	case 2:
		*value = inw(0xCFC + (reg & 2));
		break;
	case 4:
		*value = inl(0xCFC);
		break;
	}

	raw_spin_unlock_irqrestore(&pci_config_lock, flags);

	return 0;
}

static int pci_conf1_write(unsigned int seg, unsigned int bus,
			   unsigned int devfn, int reg, int len, u32 value)
{
	unsigned long flags;

	if (seg || (bus > 255) || (devfn > 255) || (reg > 4095))
		return -EINVAL;

	raw_spin_lock_irqsave(&pci_config_lock, flags);

	outl(PCI_CONF1_ADDRESS(bus, devfn, reg), 0xCF8);

	switch (len) {
	case 1:
		outb((u8)value, 0xCFC + (reg & 3));
		break;
	case 2:
		outw((u16)value, 0xCFC + (reg & 2));
		break;
	case 4:
		outl((u32)value, 0xCFC);
		break;
	}

	raw_spin_unlock_irqrestore(&pci_config_lock, flags);

	return 0;
}

#undef PCI_CONF1_ADDRESS

//这个结构其实就是pci设备配置空间操作的接口。
//封装了对PCI设备配置寄存器的读写操作。PCI设备的访问
//方式有BIOS、DIRECT的conf1和conf2、MMConfig多种。
//与conf1相依相生的就是direct.c中的pci_direct_conf1
const struct pci_raw_ops pci_direct_conf1 = {
	.read =		pci_conf1_read,//读取pci设备的配置空间函数
	.write =	pci_conf1_write,//往pci设备配置空间写数据
};


/*
 * Functions for accessing PCI configuration space with type 2 accesses
 */

#define PCI_CONF2_ADDRESS(dev, reg)	(u16)(0xC000 | (dev << 8) | reg)

static int pci_conf2_read(unsigned int seg, unsigned int bus,
			  unsigned int devfn, int reg, int len, u32 *value)
{
	unsigned long flags;
	int dev, fn;

	WARN_ON(seg);
	if ((bus > 255) || (devfn > 255) || (reg > 255)) {
		*value = -1;
		return -EINVAL;
	}

	dev = PCI_SLOT(devfn);
	fn = PCI_FUNC(devfn);

	if (dev & 0x10) 
		return PCIBIOS_DEVICE_NOT_FOUND;

	raw_spin_lock_irqsave(&pci_config_lock, flags);

	outb((u8)(0xF0 | (fn << 1)), 0xCF8);
	outb((u8)bus, 0xCFA);

	switch (len) {
	case 1:
		*value = inb(PCI_CONF2_ADDRESS(dev, reg));
		break;
	case 2:
		*value = inw(PCI_CONF2_ADDRESS(dev, reg));
		break;
	case 4:
		*value = inl(PCI_CONF2_ADDRESS(dev, reg));
		break;
	}

	outb(0, 0xCF8);

	raw_spin_unlock_irqrestore(&pci_config_lock, flags);

	return 0;
}

static int pci_conf2_write(unsigned int seg, unsigned int bus,
			   unsigned int devfn, int reg, int len, u32 value)
{
	unsigned long flags;
	int dev, fn;

	WARN_ON(seg);
	if ((bus > 255) || (devfn > 255) || (reg > 255)) 
		return -EINVAL;

	dev = PCI_SLOT(devfn);
	fn = PCI_FUNC(devfn);

	if (dev & 0x10) 
		return PCIBIOS_DEVICE_NOT_FOUND;

	raw_spin_lock_irqsave(&pci_config_lock, flags);

	outb((u8)(0xF0 | (fn << 1)), 0xCF8);
	outb((u8)bus, 0xCFA);

	switch (len) {
	case 1:
		outb((u8)value, PCI_CONF2_ADDRESS(dev, reg));
		break;
	case 2:
		outw((u16)value, PCI_CONF2_ADDRESS(dev, reg));
		break;
	case 4:
		outl((u32)value, PCI_CONF2_ADDRESS(dev, reg));
		break;
	}

	outb(0, 0xCF8);    

	raw_spin_unlock_irqrestore(&pci_config_lock, flags);

	return 0;
}

#undef PCI_CONF2_ADDRESS

static const struct pci_raw_ops pci_direct_conf2 = {
	.read =		pci_conf2_read,
	.write =	pci_conf2_write,
};


/*
 * Before we decide to use direct hardware access mechanisms, we try to do some
 * trivial checks to ensure it at least _seems_ to be working -- we just test
 * whether bus 00 contains a host bridge (this is similar to checking
 * techniques used in XFree86, but ours should be more reliable since we
 * attempt to make use of direct access hints provided by the PCI BIOS).
 *
 * This should be close to trivial, but it isn't, because there are buggy
 * chipsets (yes, you guessed it, by Intel and Compaq) that have no class ID.
 */
 //数确保这种访问模式可以正确执行。这里是测试
 //HOST-bridge是否存在，是否是主桥设备
static int __init pci_sanity_check(const struct pci_raw_ops *o)
{
	u32 x = 0;
	int year, devfn;

	if (pci_probe & PCI_NO_CHECKS)
		return 1;
	/* Assume Type 1 works for newer systems.
	   This handles machines that don't have anything on PCI Bus 0. */
	dmi_get_date(DMI_BIOS_DATE, &year, NULL, NULL);
	if (year >= 2001)
		return 1;

	//每个总线下最多偶255个设备号
	for (devfn = 0; devfn < 0x100; devfn++) {
		//读取设备的类型
		if (o->read(0, 0, devfn, PCI_CLASS_DEVICE, 2, &x))
			continue;
		if (x == PCI_CLASS_BRIDGE_HOST || x == PCI_CLASS_DISPLAY_VGA)
			return 1;

		//读取设备厂商
		if (o->read(0, 0, devfn, PCI_VENDOR_ID, 2, &x))
			continue;
		if (x == PCI_VENDOR_ID_INTEL || x == PCI_VENDOR_ID_COMPAQ)
			return 1;
	}

	DBG(KERN_WARNING "PCI: Sanity check failed\n");
	return 0;
}

static int __init pci_check_type1(void)
{
	unsigned long flags;
	unsigned int tmp;
	int works = 0;


	local_irq_save(flags);
	//前四个字节是地址端口，后四个字节是数据端口
//首先向0xCFB写入了0x01,然后保存0xCF8 四个字节的内容，接着
//向地址端口0xCF8写入0x800000000,然后读数据端口。

//pci总线为用户提供了动态查询pci设备信息的方法。在x86上,
//保留了0xCF8~0xCFF的8个寄存器.实际上就是对应地址为0xCF8的
//32位寄存器和地址为0xCFC的32位寄存器。
//在0xCF8寄存中写入要访问设备对应的总线号, 设备号、
//功能号和寄存器号组成的一个32位数写入0xCF8.然后从0xCFC
//上就可以取出对应pci设备的信息.

//对于32位的X86架构来说，有两个32位的I/O端口用于这个目
//的，就是地址端口0cf8和数据端口0cfc。访问某个设备的配
//置寄存器时，CPU向地址端口写入目标的地址，然后通过
//数据端口读写数据。
	//目的地址端口，要访问的地址
	outb(0x01, 0xCFB);//写入一个字节的数据
	tmp = inl(0xCF8);//读取四字节的数据，读地址中的数据
	outl(0x80000000, 0xCF8);//写入
	//判断0cf8是不是真的是当作主桥的地址端口
	//pci_sanity_check验证是否存在pci总线
	//如果0cf8确实是主桥的地址端口，则读时候，返回的肯
	//定是先前写到里边儿的值
	if (inl(0xCF8) == 0x80000000 && pci_sanity_check(&pci_direct_conf1)) {
		//0cf8极有可能就是主桥的地址端口，但也有可能只是
		//凑巧，所以要接着调用pci_sanity_check做更深入的验证
		works = 1;
	}
	outl(tmp, 0xCF8);//把先前的值再写进去。
	
	local_irq_restore(flags);

	return works;
}

static int __init pci_check_type2(void)
{
	unsigned long flags;
	int works = 0;

	local_irq_save(flags);

	outb(0x00, 0xCFB);
	outb(0x00, 0xCF8);
	outb(0x00, 0xCFA);
	if (inb(0xCF8) == 0x00 && inb(0xCFA) == 0x00 &&
	    pci_sanity_check(&pci_direct_conf2)) {
		works = 1;
	}

	local_irq_restore(flags);

	return works;
}
//设置访问pci设备配置空间操作的方法
void __init pci_direct_init(int type)
{
	//type==1
	if (type == 0)
		return;//不执行
	printk(KERN_INFO "PCI: Using configuration type %d for base access\n",
		 type);
	if (type == 1) {
		//pci设备配置空间操作的接口
		raw_pci_ops = &pci_direct_conf1;//进去
		if (raw_pci_ext_ops)
			return;
		if (!(pci_probe & PCI_HAS_IO_ECS))
			return;
		printk(KERN_INFO "PCI: Using configuration type 1 "
		       "for extended access\n");
		raw_pci_ext_ops = &pci_direct_conf1;
		return;
	}
	raw_pci_ops = &pci_direct_conf2;
}

//从IO地址空间申请8个字节用作访问PCI配置空间的IO端口
//0xCF8~0xCFF。前四个字节做地址端口，后四个字节做数据端
//口。
int __init pci_direct_probe(void)
{
	if ((pci_probe & PCI_PROBE_CONF1) == 0)
		goto type2;
	//在IO地址空间从0xCF8开始申请八个字节用于用于配置PCI
//0xCF8 io端口的基地址，实际的物理地址。
//io端口占用的范围。
//使用这段io地址的设备名。
//在/proc/ioport中按照名字或者地址可以找到
	if (!request_region(0xCF8, 8, "PCI conf1"))
		goto type2;

	if (pci_check_type1()) {//对1型配置方式做检查，进来吧
		//这个pci设备是主桥
		raw_pci_ops = &pci_direct_conf1;//赋值
		port_cf9_safe = true;
		return 1;
	}
	release_region(0xCF8, 8);

 type2:
	if ((pci_probe & PCI_PROBE_CONF2) == 0)
		return 0;
	if (!request_region(0xCF8, 4, "PCI conf2"))
		return 0;
	if (!request_region(0xC000, 0x1000, "PCI conf2"))
		goto fail2;

	if (pci_check_type2()) {
		raw_pci_ops = &pci_direct_conf2;
		port_cf9_safe = true;
		return 2;
	}

	release_region(0xC000, 0x1000);
 fail2:
	release_region(0xCF8, 4);
	return 0;
}

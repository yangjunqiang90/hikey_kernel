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

//����ṹ��ʵ����pci�豸���ÿռ�����Ľӿڡ�
//��װ�˶�PCI�豸���üĴ����Ķ�д������PCI�豸�ķ���
//��ʽ��BIOS��DIRECT��conf1��conf2��MMConfig���֡�
//��conf1���������ľ���direct.c�е�pci_direct_conf1
const struct pci_raw_ops pci_direct_conf1 = {
	.read =		pci_conf1_read,//��ȡpci�豸�����ÿռ亯��
	.write =	pci_conf1_write,//��pci�豸���ÿռ�д����
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
 //��ȷ�����ַ���ģʽ������ȷִ�С������ǲ���
 //HOST-bridge�Ƿ���ڣ��Ƿ��������豸
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

	//ÿ�����������ż255���豸��
	for (devfn = 0; devfn < 0x100; devfn++) {
		//��ȡ�豸������
		if (o->read(0, 0, devfn, PCI_CLASS_DEVICE, 2, &x))
			continue;
		if (x == PCI_CLASS_BRIDGE_HOST || x == PCI_CLASS_DISPLAY_VGA)
			return 1;

		//��ȡ�豸����
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
	//ǰ�ĸ��ֽ��ǵ�ַ�˿ڣ����ĸ��ֽ������ݶ˿�
//������0xCFBд����0x01,Ȼ�󱣴�0xCF8 �ĸ��ֽڵ����ݣ�����
//���ַ�˿�0xCF8д��0x800000000,Ȼ������ݶ˿ڡ�

//pci����Ϊ�û��ṩ�˶�̬��ѯpci�豸��Ϣ�ķ�������x86��,
//������0xCF8~0xCFF��8���Ĵ���.ʵ���Ͼ��Ƕ�Ӧ��ַΪ0xCF8��
//32λ�Ĵ����͵�ַΪ0xCFC��32λ�Ĵ�����
//��0xCF8�Ĵ���д��Ҫ�����豸��Ӧ�����ߺ�, �豸�š�
//���ܺźͼĴ�������ɵ�һ��32λ��д��0xCF8.Ȼ���0xCFC
//�ϾͿ���ȡ����Ӧpci�豸����Ϣ.

//����32λ��X86�ܹ���˵��������32λ��I/O�˿��������Ŀ
//�ģ����ǵ�ַ�˿�0cf8�����ݶ˿�0cfc������ĳ���豸����
//�üĴ���ʱ��CPU���ַ�˿�д��Ŀ��ĵ�ַ��Ȼ��ͨ��
//���ݶ˿ڶ�д���ݡ�
	//Ŀ�ĵ�ַ�˿ڣ�Ҫ���ʵĵ�ַ
	outb(0x01, 0xCFB);//д��һ���ֽڵ�����
	tmp = inl(0xCF8);//��ȡ���ֽڵ����ݣ�����ַ�е�����
	outl(0x80000000, 0xCF8);//д��
	//�ж�0cf8�ǲ�������ǵ������ŵĵ�ַ�˿�
	//pci_sanity_check��֤�Ƿ����pci����
	//���0cf8ȷʵ�����ŵĵ�ַ�˿ڣ����ʱ�򣬷��صĿ�
	//������ǰд����߶���ֵ
	if (inl(0xCF8) == 0x80000000 && pci_sanity_check(&pci_direct_conf1)) {
		//0cf8���п��ܾ������ŵĵ�ַ�˿ڣ���Ҳ�п���ֻ��
		//���ɣ�����Ҫ���ŵ���pci_sanity_check�����������֤
		works = 1;
	}
	outl(tmp, 0xCF8);//����ǰ��ֵ��д��ȥ��
	
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
//���÷���pci�豸���ÿռ�����ķ���
void __init pci_direct_init(int type)
{
	//type==1
	if (type == 0)
		return;//��ִ��
	printk(KERN_INFO "PCI: Using configuration type %d for base access\n",
		 type);
	if (type == 1) {
		//pci�豸���ÿռ�����Ľӿ�
		raw_pci_ops = &pci_direct_conf1;//��ȥ
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

//��IO��ַ�ռ�����8���ֽ���������PCI���ÿռ��IO�˿�
//0xCF8~0xCFF��ǰ�ĸ��ֽ�����ַ�˿ڣ����ĸ��ֽ������ݶ�
//�ڡ�
int __init pci_direct_probe(void)
{
	if ((pci_probe & PCI_PROBE_CONF1) == 0)
		goto type2;
	//��IO��ַ�ռ��0xCF8��ʼ����˸��ֽ�������������PCI
//0xCF8 io�˿ڵĻ���ַ��ʵ�ʵ������ַ��
//io�˿�ռ�õķ�Χ��
//ʹ�����io��ַ���豸����
//��/proc/ioport�а������ֻ��ߵ�ַ�����ҵ�
	if (!request_region(0xCF8, 8, "PCI conf1"))
		goto type2;

	if (pci_check_type1()) {//��1�����÷�ʽ����飬������
		//���pci�豸������
		raw_pci_ops = &pci_direct_conf1;//��ֵ
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

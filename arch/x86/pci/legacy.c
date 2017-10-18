/*
 * legacy.c - traditional, old school PCI bus probing
 */
#include <linux/init.h>
#include <linux/export.h>
#include <linux/pci.h>
#include <asm/pci_x86.h>

/*
 * Discover remaining PCI buses in case there are peer host bridges.
 * We use the number of last PCI bus provided by the PCI BIOS.
 */
static void pcibios_fixup_peer_bridges(void)
{
	int n;

	if (pcibios_last_bus <= 0 || pcibios_last_bus > 0xff)
		return;
	DBG("PCI: Peer bridge fixup\n");
pci_register_driver
	for (n=0; n <= pcibios_last_bus; n++)
		pcibios_scan_specific_bus(n);
}
//pci_arch_init先执行
int __init pci_legacy_init(void)
{
	//访问pci配置空间的方式是否已经配置了。
	if (!raw_pci_ops) {//pci_direct_init()中赋值
		printk("PCI: System does not support PCI\n");
		return 0;
	}

	printk("PCI: Probing PCI hardware\n");
	//那么0的含义自然就是从PCI根总线开始。
	pcibios_scan_root(0);//探测PCI设备，完成pci设备的枚举过程。进来吧
	return 0;
}

void pcibios_scan_specific_bus(int busn)
{
	int devfn;
	u32 l;

	if (pci_find_bus(0, busn))
		return;

	for (devfn = 0; devfn < 256; devfn += 8) {
		if (!raw_pci_read(0, busn, devfn, PCI_VENDOR_ID, 2, &l) &&
		    l != 0x0000 && l != 0xffff) {
			DBG("Found device at %02x:%02x [%04x]\n", busn, devfn, l);
			printk(KERN_INFO "PCI: Discovered peer bus %02x\n", busn);
			pcibios_scan_root(busn);
			return;
		}
	}
}
EXPORT_SYMBOL_GPL(pcibios_scan_specific_bus);

int __init pci_subsys_init(void)
{
	/*
	 * The init function returns an non zero value when
	 * pci_legacy_init should be invoked.
	 */
	 //这pci.init可能(Pci_x86.c文件中有宏开关)会被初始化成两个函
	 //数pci_legacy_init和pci_acpi_init 

	//如果配置了ACPI的话，就初始化成pci_acpi_init，否则就初始
	//化成pci_legacy_init，这里我们先只考虑没有配置ACPI的情况

	//猜想这里是要确保pci_legacy_init得到执行

	if (x86_init.pci.init())//
		pci_legacy_init();//进去

	pcibios_fixup_peer_bridges();
	x86_init.pci.init_irq();
	pcibios_init();

	return 0;
}
subsys_initcall(pci_subsys_init);

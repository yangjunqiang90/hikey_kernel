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
//pci_arch_init��ִ��
int __init pci_legacy_init(void)
{
	//����pci���ÿռ�ķ�ʽ�Ƿ��Ѿ������ˡ�
	if (!raw_pci_ops) {//pci_direct_init()�и�ֵ
		printk("PCI: System does not support PCI\n");
		return 0;
	}

	printk("PCI: Probing PCI hardware\n");
	//��ô0�ĺ�����Ȼ���Ǵ�PCI�����߿�ʼ��
	pcibios_scan_root(0);//̽��PCI�豸�����pci�豸��ö�ٹ��̡�������
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
	 //��pci.init����(Pci_x86.c�ļ����к꿪��)�ᱻ��ʼ����������
	 //��pci_legacy_init��pci_acpi_init 

	//���������ACPI�Ļ����ͳ�ʼ����pci_acpi_init������ͳ�ʼ
	//����pci_legacy_init������������ֻ����û������ACPI�����

	//����������Ҫȷ��pci_legacy_init�õ�ִ��

	if (x86_init.pci.init())//
		pci_legacy_init();//��ȥ

	pcibios_fixup_peer_bridges();
	x86_init.pci.init_irq();
	pcibios_init();

	return 0;
}
subsys_initcall(pci_subsys_init);

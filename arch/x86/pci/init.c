#include <linux/pci.h>
#include <linux/init.h>
#include <asm/pci_x86.h>
#include <asm/x86_init.h>

//���������arch_initcall(pci_arch_init)���ã���arch_initcall���õĺ���
//�������init����,������ĺ�����kernel������ʱ����Լ�
//ִ�еĺ�����
/* arch_initcall has too random ordering, so call the initializers
   in the right sequence from here. */
   //pci_subsys_init
static __init int pci_arch_init(void)
{
//ֱ�ӽ���PCI�豸��̽���ö��
// CONFIG_PCI_DIRECT��ʾֱ�ӽ���PCI�豸��̽���ö��,
#ifdef CONFIG_PCI_DIRECT
	int type = 0;
//��pci�淶�ж��������ֲ������ÿռ�ķ�����
//����1�͡��͡�2�͡���ͨ����ʹ�á�1�͡�����ˣ��ڴ�
//����pci_direct_probe()һ��᷵��1����ʹ�á�1�͡���

//2��ֻ��Ϊ������ܻ���ʹ���е�ĳЩ��ʽĸ����ݶ���Ƶġ�

//0xCF8�����ŵ����ö˿�
	type = pci_direct_probe();//����
#endif

	if (!(pci_probe & PCI_PROBE_NOEARLY))
		pci_mmcfg_early_init();

	if (x86_init.pci.arch_init && !x86_init.pci.arch_init())
		return 0;
//ͨ��BIOS����PCI�豸��̽���ö��
//��ʾͨ��BIOS����PCI�豸��̽���ö�٣����費ʹ��bios
#ifdef CONFIG_PCI_BIOS
	pci_pcbios_init();//��ִ��
#endif
	/*
	 * don't check for raw_pci_ops here because we want pcbios as last
	 * fallback, yet it's needed to run first to set pcibios_last_bus
	 * in case legacy PCI probing is used. otherwise detecting peer busses
	 * fails.
	 */
 //ֱ�ӽ���PCI�豸��̽���ö�٣�����ʹ���ֶ�ȥö��
#ifdef CONFIG_PCI_DIRECT
	//���÷���pci���ÿռ�ķ���
	pci_direct_init(type);//����
#endif
	if (!raw_pci_ops && !raw_pci_ext_ops)
		printk(KERN_ERR
		"PCI: Fatal: No config space access function found\n");

	dmi_check_pciprobe();

	dmi_check_skip_isa_align();

	return 0;
}
arch_initcall(pci_arch_init);

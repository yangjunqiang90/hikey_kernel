#include <linux/pci.h>
#include <linux/init.h>
#include <asm/pci_x86.h>
#include <asm/x86_init.h>

//这个函数由arch_initcall(pci_arch_init)调用，而arch_initcall引用的函数
//都会放在init区域,这里面的函数是kernel启动的时候会自己
//执行的函数。
/* arch_initcall has too random ordering, so call the initializers
   in the right sequence from here. */
   //pci_subsys_init
static __init int pci_arch_init(void)
{
//直接进行PCI设备的探测和枚举
// CONFIG_PCI_DIRECT表示直接进行PCI设备的探测和枚举,
#ifdef CONFIG_PCI_DIRECT
	int type = 0;
//在pci规范中定义了两种操作配置空间的方法，
//即“1型”和“2型”，通常会使用“1型”。因此，在代
//码中pci_direct_probe()一般会返回1，即使用“1型”。

//2型只是为了与可能还在使用中的某些老式母板兼容而设计的。

//0xCF8是主桥的配置端口
	type = pci_direct_probe();//进来
#endif

	if (!(pci_probe & PCI_PROBE_NOEARLY))
		pci_mmcfg_early_init();

	if (x86_init.pci.arch_init && !x86_init.pci.arch_init())
		return 0;
//通过BIOS进行PCI设备的探测和枚举
//表示通过BIOS进行PCI设备的探测和枚举，假设不使用bios
#ifdef CONFIG_PCI_BIOS
	pci_pcbios_init();//不执行
#endif
	/*
	 * don't check for raw_pci_ops here because we want pcbios as last
	 * fallback, yet it's needed to run first to set pcibios_last_bus
	 * in case legacy PCI probing is used. otherwise detecting peer busses
	 * fails.
	 */
 //直接进行PCI设备的探测和枚举，假设使用手动去枚举
#ifdef CONFIG_PCI_DIRECT
	//设置访问pci配置空间的方法
	pci_direct_init(type);//进来
#endif
	if (!raw_pci_ops && !raw_pci_ext_ops)
		printk(KERN_ERR
		"PCI: Fatal: No config space access function found\n");

	dmi_check_pciprobe();

	dmi_check_skip_isa_align();

	return 0;
}
arch_initcall(pci_arch_init);

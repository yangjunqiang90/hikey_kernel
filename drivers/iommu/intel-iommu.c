/*
 * Copyright 漏 2006-2014 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * Authors: David Woodhouse <dwmw2@infradead.org>,
 *          Ashok Raj <ashok.raj@intel.com>,
 *          Shaohua Li <shaohua.li@intel.com>,
 *          Anil S Keshavamurthy <anil.s.keshavamurthy@intel.com>,
 *          Fenghua Yu <fenghua.yu@intel.com>
 */

#include <linux/init.h>
#include <linux/bitmap.h>
#include <linux/debugfs.h>
#include <linux/export.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/pci.h>
#include <linux/dmar.h>
#include <linux/dma-mapping.h>
#include <linux/mempool.h>
#include <linux/memory.h>
#include <linux/timer.h>
#include <linux/iova.h>
#include <linux/iommu.h>
#include <linux/intel-iommu.h>
#include <linux/syscore_ops.h>
#include <linux/tboot.h>
#include <linux/dmi.h>
#include <linux/pci-ats.h>
#include <linux/memblock.h>
#include <linux/dma-contiguous.h>
#include <asm/irq_remapping.h>
#include <asm/cacheflush.h>
#include <asm/iommu.h>

#include "irq_remapping.h"

#define ROOT_SIZE		VTD_PAGE_SIZE
#define CONTEXT_SIZE		VTD_PAGE_SIZE

#define IS_GFX_DEVICE(pdev) ((pdev->class >> 16) == PCI_BASE_CLASS_DISPLAY)
#define IS_USB_DEVICE(pdev) ((pdev->class >> 8) == PCI_CLASS_SERIAL_USB)
#define IS_ISA_DEVICE(pdev) ((pdev->class >> 8) == PCI_CLASS_BRIDGE_ISA)
#define IS_AZALIA(pdev) ((pdev)->vendor == 0x8086 && (pdev)->device == 0x3a3e)

#define IOAPIC_RANGE_START	(0xfee00000)
#define IOAPIC_RANGE_END	(0xfeefffff)
#define IOVA_START_ADDR		(0x1000)

//默认domain地址宽度，
//也就是domain的gaw值
#define DEFAULT_DOMAIN_ADDRESS_WIDTH 48

//agaw的最大值
#define MAX_AGAW_WIDTH 64
//max agaw pfn width  = 64-12
#define MAX_AGAW_PFN_WIDTH	(MAX_AGAW_WIDTH - VTD_PAGE_SHIFT)

#define __DOMAIN_MAX_PFN(gaw)  ((((uint64_t)1) << (gaw-VTD_PAGE_SHIFT)) - 1)
#define __DOMAIN_MAX_ADDR(gaw) ((((uint64_t)1) << gaw) - 1)

/* We limit DOMAIN_MAX_PFN to fit in an unsigned long, and DOMAIN_MAX_ADDR
   to match. That way, we can use 'unsigned long' for PFNs with impunity. */
#define DOMAIN_MAX_PFN(gaw)	((unsigned long) min_t(uint64_t, \
				__DOMAIN_MAX_PFN(gaw), (unsigned long)-1))
#define DOMAIN_MAX_ADDR(gaw)	(((uint64_t)__DOMAIN_MAX_PFN(gaw)) << VTD_PAGE_SHIFT)

/* IO virtual address start page frame number */
//io虚拟地址开始的页帧号吗
#define IOVA_START_PFN		(1)

#define IOVA_PFN(addr)		((addr) >> PAGE_SHIFT)
#define DMA_32BIT_PFN		IOVA_PFN(DMA_BIT_MASK(32))
#define DMA_64BIT_PFN		IOVA_PFN(DMA_BIT_MASK(64))

/* page table handling */
//表示agaw加几，增加一级页表
#define LEVEL_STRIDE		(9)
//低LEVEL_STRIDE位全为1，其余位为0
#define LEVEL_MASK		(((u64)1 << LEVEL_STRIDE) - 1)

/*
 * This bitmap is used to advertise the page sizes our hardware support
 * to the IOMMU core, which will then use this information to split
 * physically contiguous memory regions it is mapping into page sizes
 * that we support.
 *
 * Traditionally the IOMMU core just handed us the mappings directly,
 * after making sure the size is an order of a 4KiB page and that the
 * mapping has natural alignment.
 *
 * To retain this behavior, we currently advertise that we support
 * all page sizes that are an order of 4KiB.
 *
 * If at some point we'd like to utilize the IOMMU core's new behavior,
 * we could change this to advertise the real page sizes we support.
 */
 //bitmap习惯用来表示iommu硬件核心支持的页大小
//低12位为0
#define INTEL_IOMMU_PGSIZES	(~0xFFFUL)

static inline int agaw_to_level(int agaw)
{
	return agaw + 2;
}

static inline int agaw_to_width(int agaw)
{
	return min_t(int, 30 + agaw * LEVEL_STRIDE, MAX_AGAW_WIDTH);
}

static inline int width_to_agaw(int width)
{
	return DIV_ROUND_UP(width - 30, LEVEL_STRIDE);
}
//级数转成多少位
//2级 就是9位，3级就是19位
static inline unsigned int level_to_offset_bits(int level)
{
	return (level - 1) * LEVEL_STRIDE;
}

//页地址编号右移级数对应的位数，然后保留地低9位
//得到在level级页表的便宜量。
//注意，顶级的级数最大
//从pfn取出level级的编号
static inline int pfn_level_offset(unsigned long pfn, int level)
{
	return (pfn >> level_to_offset_bits(level)) & LEVEL_MASK;
}

static inline unsigned long level_mask(int level)
{
	return -1UL << level_to_offset_bits(level);
}

static inline unsigned long level_size(int level)
{
	return 1UL << level_to_offset_bits(level);
}

static inline unsigned long align_to_level(unsigned long pfn, int level)
{
	return (pfn + level_size(level) - 1) & level_mask(level);
}
//lvl =2 return 10,0000,0000
//lvl =3 return 100,0000,0000,0000,0000
static inline unsigned long lvl_to_nr_pages(unsigned int lvl)
{
	return  1 << min_t(int, (lvl - 1) * LEVEL_STRIDE, MAX_AGAW_PFN_WIDTH);
}

/* VT-d pages must always be _smaller_ than MM pages. Otherwise things
   are never going to work. */
static inline unsigned long dma_to_mm_pfn(unsigned long dma_pfn)
{
	return dma_pfn >> (PAGE_SHIFT - VTD_PAGE_SHIFT);
}
//mm_pfn 页帧号
static inline unsigned long mm_to_dma_pfn(unsigned long mm_pfn)
{
	return mm_pfn << (PAGE_SHIFT - VTD_PAGE_SHIFT);
}
static inline unsigned long page_to_dma_pfn(struct page *pg)
{
	return mm_to_dma_pfn(page_to_pfn(pg));
}
//虚拟地址转换成dma 的页帧号
static inline unsigned long virt_to_dma_pfn(void *p)
{
	return page_to_dma_pfn(virt_to_page(p));
}

/* global iommu list, set NULL for ignored DMAR units */
static struct intel_iommu **g_iommus;//实际是iommu的指针数组

static void __init check_tylersburg_isoch(void);
static int rwbf_quirk;

/*
 * set to 1 to panic kernel if can't successfully enable VT-d
 * (used when kernel is launched w/ TXT)
 */
static int force_on = 0;//如果等于1 表示没有成功使能VT-d

/*
 * 0: Present
 * 1-11: Reserved
 * 12-63: Context Ptr (12 - (haw-1))
 * 64-127: Reserved
 */
 //这个结构用来描述pci总线
 //共256个root entry构成一张表
struct root_entry {
	u64	lo;
	u64	hi;
};
#define ROOT_ENTRY_NR (VTD_PAGE_SIZE/sizeof(struct root_entry))


/*
 * low 64 bits:
 * 0: present
 * 1: fault processing disable
 * 2-3: translation type
 * 12-63: address space root
 * high 64 bits:
 * 0-2: address width
 * 3-6: aval
 * 8-23: domain id
 */
struct context_entry {
	u64 lo;
	u64 hi;
};

//低地址的内存是否有效
//返回present位的值
static inline bool context_present(struct context_entry *context)
{
	return (context->lo & 1);
}
//设置context_entry的present为1
static inline void context_set_present(struct context_entry *context)
{
	context->lo |= 1;
}

////设置context_entry的fault域为0
static inline void context_set_fault_enable(struct context_entry *context)
{
	context->lo &= (((u64)-1) << 2) | 1;
}
//设置context_entry的translation
static inline void context_set_translation_type(struct context_entry *context,
						unsigned long value)
{
	context->lo &= (((u64)-1) << 4) | 3;
	context->lo |= (value & 3) << 2;
}

//设置context_entry的root adress ，其余为清零
static inline void context_set_address_root(struct context_entry *context,
					    unsigned long value)
{
	context->lo &= ~VTD_PAGE_MASK;
	context->lo |= value & VTD_PAGE_MASK;
}

static inline void context_set_address_width(struct context_entry *context,
					     unsigned long value)
{
	context->hi |= value & 7;
}
//设置context_entry中domain_id的值
static inline void context_set_domain_id(struct context_entry *context,
					 unsigned long value)
{
	context->hi |= (value & ((1 << 16) - 1)) << 8;
}

static inline void context_clear_entry(struct context_entry *context)
{
	context->lo = 0;
	context->hi = 0;
}

/*
 * 0: readable
 * 1: writable
 * 2-6: reserved
 * 7: super page  是否是大页，为1表示是大页
 * 8-10: available
 * 11: snoop behavior
 * 12-63: Host physcial address
 */
struct dma_pte {
	u64 val;
};

static inline void dma_clear_pte(struct dma_pte *pte)
{
	pte->val = 0;
}

//VTD页对齐
//把地址转成dmar pte的地址，
//得到真正的物理地址域，其余的域是标志域或保留域清零
static inline u64 dma_pte_addr(struct dma_pte *pte)
{
#ifdef CONFIG_64BIT
	return pte->val & VTD_PAGE_MASK;
#else
	/* Must have a full atomic 64-bit read */
	return  __cmpxchg64(&pte->val, 0ULL, 0ULL) & VTD_PAGE_MASK;
#endif
}

//是否可读或者可写
static inline bool dma_pte_present(struct dma_pte *pte)
{
	return (pte->val & 3) != 0;
}

//返回dma pte中的super page位的值
static inline bool dma_pte_superpage(struct dma_pte *pte)
{
	return (pte->val & DMA_PTE_LARGE_PAGE);
}

static inline int first_pte_in_page(struct dma_pte *pte)
{
	return !((unsigned long)pte & ~VTD_PAGE_MASK);
}

/*
 * This domain is a statically identity mapping domain.
 *	1. This domain creats a static 1:1 mapping to all usable memory.
 * 	2. It maps to each iommu if successful.
 *	3. Each iommu mapps to this domain if successful.
 */
 //这个domain是statically identity domain，该domain是等值映射domain
 //如果成功，它映射到每一个iommu，每一个iommu映射到这个domain
static struct dmar_domain *si_domain;

//iommu硬件是否支持pass throught技术的标志位
// 1 表示支持，0 表示不支持
static int hw_pass_through = 1;//硬件是否支持pass through技术，默认支持

/* domain represents a virtual machine, more than one devices
 * across iommus may be owned in one domain, e.g. kvm guest.
 */
 //当domain代表一个虚拟机器的时候，一个domain有多个device
 //多个设备通过iommu可能属于一个domain，列如kvm guest
#define DOMAIN_FLAG_VIRTUAL_MACHINE	(1 << 0)

/* si_domain contains mulitple devices */
#define DOMAIN_FLAG_STATIC_IDENTITY	(1 << 1)

struct dmar_domain {
	//domain在iommu中位图的下标
	int	id;			/* domain id */
	//内存节点号
	int	nid;			/* node id */
	DECLARE_BITMAP(iommu_bmp, DMAR_UNITS_SUPPORTED);
					/* bitmap of iommus this domain uses*/

	//device_domain_info链表
	struct list_head devices;	/* all devices' list */
	//iova domain
	//这个domain的iova domain
	struct iova_domain iovad;	/* iova's that belong to this domain */

	//这个domain的虚拟地址，从这个虚拟地址中可以获取
	//该页的描述结构 struct page *
	//dma 的page table
	//指向的是一级页表
	struct dma_pte	*pgd;		/* virtual address */

	//domain的地址宽度
	//iommu的宽度
	//可以理解为从虚拟机角度看到的物理地址宽度。
	//举个例子，如果一个虚拟机只能访问2G内存，
	//那么GAW就是31
	int		gaw;		/* max guest address width */

	/* adjusted guest address width, 0 is level 2 30-bit */
	//根据gaw可以计算这个值，最大为64
	//agaw决定了IO多级页表的级数
	//30以下为1级，30为2级，每加9多一级
	//为了保证9个bit长度的步长转化，转换函数guestwidth_to_adjustwidth
	//AGAW的最小长度是30个bit

	//agaw的值        		      		level值
	//000b: 			30-bit AGAW		(2-level page table)
	//001b: 			39-bit AGAW 		(3-level page table)
	//010b: 			48-bit AGAW 		(4-level page table)
	//011b: 			57-bit AGAW 		(5-level page table)
	//100b: 			64-bit AGAW 		(6-level page table)
	root_entry
	//所以可以看到kernel里agaw的一些转换代码会用到30和2这些数字：
	//请看agaw_to_level函数
	int		agaw;

	//domain 的类型
	int		flags;		/* flags to find out type of domain */

	//是否支持Queued Invalidation
	//只要该domain中有一个iommu不支持，该值就为0
	int		iommu_coherency;/* indicate coherency of iommu access */
	int		iommu_snooping; /* indicate snooping control feature*/
	//该domain被都少个iommu引用
	//该domain中有多少个iommu
	int		iommu_count;	/* reference count of iommu */
	int		iommu_superpage;/* Level of superpages supported:
					   0 == 4KiB (no superpages), 1 == 2MiB,
					   2 == 1GiB, 3 == 512GiB, 4 == 1TiB */
	spinlock_t	iommu_lock;	/* protect iommu set in domain */
	u64		max_addr;	/* maximum mapped address */

	struct iommu_domain domain;	/* generic domain data structure for
					   iommu core */
};

/* PCI domain-device relationship */
//pci domain 和设备的关系结构体
//表示device 和pci domain之间的关系
//每个dev 对应一个device_domain_info
struct device_domain_info {
	//这个结构体会被插入到两个链表中
	struct list_head link;	/* link to domain siblings *///domain的dvice链表
	struct list_head global; /* link to global list *///全局链表
	u8 bus;			/* PCI bus number */
	u8 devfn;		/* PCI devfn number */
	struct device *dev; /* it's NULL for PCIe-to-PCI bridge */
	struct intel_iommu *iommu; /* IOMMU used by this device */
	struct dmar_domain *domain; /* pointer to domain */
};

struct dmar_rmrr_unit {
	struct list_head list;		/* list of rmrr units	*/
	struct acpi_dmar_header *hdr;	/* ACPI header		*/
	u64	base_address;		/* reserved base address*/
	u64	end_address;		/* reserved end address */
	
	//一个头地址，一个对应数组的大小
	struct dmar_dev_scope *devices;	/* target devices */
	int	devices_cnt;		/* target device count */
};

struct dmar_atsr_unit {
	struct list_head list;		/* list of ATSR units */
	struct acpi_dmar_header *hdr;	/* ACPI header */
	struct dmar_dev_scope *devices;	/* target devices */
	int devices_cnt;		/* target device count */
	u8 include_all:1;		/* include all ports */
};

static LIST_HEAD(dmar_atsr_units);
static LIST_HEAD(dmar_rmrr_units);
//从dmar_rmrr_units中获取每一个dmar_rmrr_unit
#define for_each_rmrr_units(rmrr) \
	list_for_each_entry(rmrr, &dmar_rmrr_units, list)

static void flush_unmaps_timeout(unsigned long data);

static DEFINE_TIMER(unmap_timer,  flush_unmaps_timeout, 0, 0);

#define HIGH_WATER_MARK 250
struct deferred_flush_tables {
	int next;
	struct iova *iova[HIGH_WATER_MARK];
	struct dmar_domain *domain[HIGH_WATER_MARK];
	struct page *freelist[HIGH_WATER_MARK];
};

static struct deferred_flush_tables *deferred_flush;

/* bitmap for indexing intel_iommus */
static int g_num_of_iommus;

static DEFINE_SPINLOCK(async_umap_flush_lock);
static LIST_HEAD(unmaps_to_do);

static int timer_on;
static long list_size;

static void domain_exit(struct dmar_domain *domain);
static void domain_remove_dev_info(struct dmar_domain *domain);
static void domain_remove_one_dev_info(struct dmar_domain *domain,
				       struct device *dev);
static void iommu_detach_dependent_devices(struct intel_iommu *iommu,
					   struct device *dev);
static int domain_detach_iommu(struct dmar_domain *domain,
			       struct intel_iommu *iommu);

#ifdef CONFIG_INTEL_IOMMU_DEFAULT_ON
int dmar_disabled = 0;
#else
int dmar_disabled = 1;
#endif /*CONFIG_INTEL_IOMMU_DEFAULT_ON*/
//iommu的使能标志，1表示使能了，0表示没有使能
int intel_iommu_enabled = 0;
EXPORT_SYMBOL_GPL(intel_iommu_enabled);

static int dmar_map_gfx = 1;
static int dmar_forcedac;
static int intel_iommu_strict;
static int intel_iommu_superpage = 1;
static int intel_iommu_ecs = 1;

/* We only actually use ECS when PASID support (on the new bit 40)
 * is also advertised. Some early implementations 鈥� the ones with
 * PASID support on bit 28 鈥� have issues even when we *only* use
 * extended root/context tables. */
 //ECS: Extended Context Support，我们用扩展的root/context表
 //
 //软件ECS使能并且硬件ECS使能并且硬件支持PASID
#define ecs_enabled(iommu) (intel_iommu_ecs && ecap_ecs(iommu->ecap) && \
			    ecap_pasid(iommu->ecap))

int intel_iommu_gfx_mapped;
EXPORT_SYMBOL_GPL(intel_iommu_gfx_mapped);

#define DUMMY_DEVICE_DOMAIN_INFO ((struct device_domain_info *)(-1))
static DEFINE_SPINLOCK(device_domain_lock);
static LIST_HEAD(device_domain_list);

static const struct iommu_ops intel_iommu_ops;

/* Convert generic 'struct iommu_domain to private struct dmar_domain */
static struct dmar_domain *to_dmar_domain(struct iommu_domain *dom)
{
	return container_of(dom, struct dmar_domain, domain);
}

static int __init intel_iommu_setup(char *str)
{
	if (!str)
		return -EINVAL;
	while (*str) {
		if (!strncmp(str, "on", 2)) {
			dmar_disabled = 0;
			printk(KERN_INFO "Intel-IOMMU: enabled\n");
		} else if (!strncmp(str, "off", 3)) {
			dmar_disabled = 1;
			printk(KERN_INFO "Intel-IOMMU: disabled\n");
		} else if (!strncmp(str, "igfx_off", 8)) {
			dmar_map_gfx = 0;
			printk(KERN_INFO
				"Intel-IOMMU: disable GFX device mapping\n");
		} else if (!strncmp(str, "forcedac", 8)) {
			printk(KERN_INFO
				"Intel-IOMMU: Forcing DAC for PCI devices\n");
			dmar_forcedac = 1;
		} else if (!strncmp(str, "strict", 6)) {
			printk(KERN_INFO
				"Intel-IOMMU: disable batched IOTLB flush\n");
			intel_iommu_strict = 1;
		} else if (!strncmp(str, "sp_off", 6)) {
			printk(KERN_INFO
				"Intel-IOMMU: disable supported super page\n");
			intel_iommu_superpage = 0;
		} else if (!strncmp(str, "ecs_off", 7)) {
			printk(KERN_INFO
				"Intel-IOMMU: disable extended context table support\n");
			intel_iommu_ecs = 0;
		}

		str += strcspn(str, ",");
		while (*str == ',')
			str++;
	}
	return 0;
}
__setup("intel_iommu=", intel_iommu_setup);

static struct kmem_cache *iommu_domain_cache;
static struct kmem_cache *iommu_devinfo_cache;
irq_get_desc_buslock
//在NUMA 节点为node ，分配一个内存页，返回内存页的虚拟地址
static inline void *alloc_pgtable_page(int node)
{
	struct page *page;
	void *vaddr = NULL;
	//申请一个page地址空间，order为0表示分配一个内存页。
	page = alloc_pages_node(node, GFP_ATOMIC | __GFP_ZERO, 0);
	if (page)
		vaddr = page_address(page);//取得page对应的虚拟地址
	return vaddr;
}
kfifo_skip
static inline void free_pgtable_page(void *vaddr)
{
	free_page((unsigned long)vaddr);
}
//申请一个dmar_domain，从domain cache中返回这个对象。
static inline void *alloc_domain_mem(void)
{
	return kmem_cache_alloc(iommu_domain_cache, GFP_ATOMIC);
}

static void free_domain_mem(void *vaddr)
{
	kmem_cache_free(iommu_domain_cache, vaddr);
}

static inline void * alloc_devinfo_mem(void)
{
	return kmem_cache_alloc(iommu_devinfo_cache, GFP_ATOMIC);
}

//申请一个device_domain_info，从iommu_devinfo_cache缓冲区中得到。
static inline void free_devinfo_mem(void *vaddr)
{
	kmem_cache_free(iommu_devinfo_cache, vaddr);
}

//根据flag判断这个domain是否是虚拟机器
static inline int domain_type_is_vm(struct dmar_domain *domain)
{
	return domain->flags & DOMAIN_FLAG_VIRTUAL_MACHINE;
}

static inline int domain_type_is_vm_or_si(struct dmar_domain *domain)
{
	return domain->flags & (DOMAIN_FLAG_VIRTUAL_MACHINE |
				DOMAIN_FLAG_STATIC_IDENTITY);
}
//返回1 表示传进来的地址范围与domain支持
//的页地址范围合法。就是iommu的地址空间
static inline int domain_pfn_supported(struct dmar_domain *domain,
				       unsigned long pfn)
{
	//得到页地址可以有多少位
	//也就是页地址的取值范围
	int addr_width = agaw_to_width(domain->agaw) - VTD_PAGE_SHIFT;

	//domain中最后一页地址没有越界为0
	//页地址宽度小于系统位数为1
	
	return !(addr_width < BITS_PER_LONG && pfn >> addr_width);
}

static int __iommu_calculate_agaw(struct intel_iommu *iommu, int max_gaw)
{
	unsigned long sagaw;
	int agaw = -1;

	sagaw = cap_sagaw(iommu->cap);
	for (agaw = width_to_agaw(max_gaw);
	     agaw >= 0; agaw--) {
		if (test_bit(agaw, &sagaw))
			break;
	}

	return agaw;
}

/*
 * Calculate max SAGAW for each iommu.
 */
int iommu_calculate_max_sagaw(struct intel_iommu *iommu)
{
	return __iommu_calculate_agaw(iommu, MAX_AGAW_WIDTH);
}

/*
 * calculate agaw for each iommu.
 * "SAGAW" may be different across iommus, use a default agaw, and
 * get a supported less agaw for iommus that don't support the default agaw.
 */
int iommu_calculate_agaw(struct intel_iommu *iommu)
{
	return __iommu_calculate_agaw(iommu, DEFAULT_DOMAIN_ADDRESS_WIDTH);
}

/* This functionin only returns single iommu in a domain */
static struct intel_iommu *domain_get_iommu(struct dmar_domain *domain)
{
	int iommu_id;

	/* si_domain and vm domain should not get here. */
	BUG_ON(domain_type_is_vm_or_si(domain));
	iommu_id = find_first_bit(domain->iommu_bmp, g_num_of_iommus);
	if (iommu_id < 0 || iommu_id >= g_num_of_iommus)
		return NULL;

	return g_iommus[iommu_id];
}
//更新domain中iommu_coherency的值
static void domain_update_iommu_coherency(struct dmar_domain *domain)
{
	struct dmar_drhd_unit *drhd;
	struct intel_iommu *iommu;
	bool found = false;
	int i;

	domain->iommu_coherency = 1;

	//如果domain中的所有iommu中有一个不支持Queued Invalidation
	//则domain的iommu_coherency为0
	for_each_set_bit(i, domain->iommu_bmp, g_num_of_iommus) {
		//i 表示位图中为1位的下标，也就是iommu的编号
		found = true;
		if (!ecap_coherent(g_iommus[i]->ecap)) {
			//如果硬件不支持Queued Invalidation
			domain->iommu_coherency = 0;
			break;
		}
	}
	if (found)
		return;

	/* No hardware attached; use lowest common denominator */
	//这个domain竟然没有一个iommu
	//那么便利所有的iommu，如果有一个iommu不支持
	//该为就为0，如果都支持，不更新
	rcu_read_lock();
	for_each_active_iommu(iommu, drhd) {
		if (!ecap_coherent(iommu->ecap)) {
			domain->iommu_coherency = 0;
			break;
		}
	}
	rcu_read_unlock();
}
//除了skip之外，其他的iommu如果有一个的snoop control位为0，
//则返回0
static int domain_update_iommu_snooping(struct intel_iommu *skip)
{
	struct dmar_drhd_unit *drhd;
	struct intel_iommu *iommu;
	int ret = 1;

	rcu_read_lock();
	for_each_active_iommu(iommu, drhd) {
		if (iommu != skip) {
			//不是指定的iommu
			if (!ecap_sc_support(iommu->ecap)) {
				ret = 0;
				break;
			}
		}
	}
	rcu_read_unlock();

	return ret;
}

static int domain_update_iommu_superpage(struct intel_iommu *skip)
{
	struct dmar_drhd_unit *drhd;
	struct intel_iommu *iommu;
	int mask = 0xf;

	if (!intel_iommu_superpage) {
		return 0;
	}

	/* set iommu_superpage to the smallest common denominator */
	rcu_read_lock();
	for_each_active_iommu(iommu, drhd) {
		if (iommu != skip) {
			mask &= cap_super_page_val(iommu->cap);
			if (!mask)
				break;
		}
	}
	rcu_read_unlock();

	return fls(mask);
}

/* Some capabilities may be different across iommus */
//更新domain中iommu_coherency，iommu_snooping，iommu_superpage的值
static void domain_update_iommu_cap(struct dmar_domain *domain)
{
	domain_update_iommu_coherency(domain);
	domain->iommu_snooping = domain_update_iommu_snooping(NULL);
	domain->iommu_superpage = domain_update_iommu_superpage(NULL);
}

//从iommu的root_entry数组中取出第bus项，我们使用ECS，对内容
//进行了扩展，devfn以2为单位访问，可以访问2 * 256个context_entry
//一页(4k)最多有256个context_entry，一个root_entry可以访问两个页
//根据devfn的最高位判断访问哪个页，为1访问hi的地址空间的页
//root_entry中保存的是页的物理地址，最低位为1表示空间可用

//返回两个context_entry(一个16字节) 的地址，
//放回iommu中context的表项地址
//iommu二级页表项的某个表项的地址，二级页表不存在就申请一个表
//bus是一级页表的索引
//devfn是二级页表的索引
//alloc表示如果二级页表如果没有，是否允许申请
// 1 表示允许，0 表示不允许
static inline struct context_entry *iommu_context_addr(struct intel_iommu *iommu,
						       u8 bus, u8 devfn, int alloc)
{
	struct root_entry *root = &iommu->root_entry[bus];
	struct context_entry *context;//虚拟地址，页的起始地址，不是页内的地址
	u64 *entry;

	entry = &root->lo;
	if (ecs_enabled(iommu)) {
		//如果ecs成功使能了
		//因为devfn要左移一位，所以如果有进位就去高地址
		//否则取地址
		if (devfn >= 0x80) {
			devfn -= 0x80;
			entry = &root->hi;
		}
		devfn *= 2;//devfn左移一位
	}
	if (*entry & 1)
		//地址的最低位为1，
		//物理地址转成虚拟地址
		//直接return &context[devfn];
		context = phys_to_virt(*entry & VTD_PAGE_MASK);
	else {
		//是虚拟地址
		unsigned long phy_addr;
		if (!alloc)
			return NULL;

		context = alloc_pgtable_page(iommu->node);
		if (!context)
			return NULL;

		__iommu_flush_cache(iommu, (void *)context, CONTEXT_SIZE);
		phy_addr = virt_to_phys((void *)context);
		*entry = phy_addr | 1;
		__iommu_flush_cache(iommu, entry, sizeof(*entry));
	}
	return &context[devfn];
}

//返回是否为dev->archdata.iommu赋值了。
//0表示赋值了
//1表示没有赋值
static int iommu_dummy(struct device *dev)
{
	return dev->archdata.iommu == DUMMY_DEVICE_DOMAIN_INFO;
}

//返回与device相关联的iommu
//如果iommu的dmar_drhd_unit的dmar_dev_scope的数组中某一项的
//device是传入的device，就返回这个iommu
//hrhd中dmar_dev_scope特定的一项的bus，devfn
//或者传入的dev转成pci dev 的相关数据
//获得这个设备的iommu
static struct intel_iommu *device_to_iommu(struct device *dev, u8 *bus, u8 *devfn)
{
	struct dmar_drhd_unit *drhd = NULL;
	struct intel_iommu *iommu;
	struct device *tmp;
	struct pci_dev *ptmp, *pdev = NULL;
	u16 segment = 0;//pci bus中的domain_nr
	int i;

	if (iommu_dummy(dev))
		return NULL;//dev->archdata.iommu赋值了，device_domain_info类型

	//dev->archdata.iommu没有赋值了，device_domain_info类型

	if (dev_is_pci(dev)) {//返回这个设备是否是pci设备，通过bus变量确定
		pdev = to_pci_dev(dev);//转成pci_dev
		segment = pci_domain_nr(pdev->bus);//获得pci bus中的domain_nr
	} else if (has_acpi_companion(dev))
		dev = &ACPI_COMPANION(dev)->dev;//不是pci设备

	rcu_read_lock();
	for_each_active_iommu(iommu, drhd) {
		if (pdev && segment != drhd->segment)
			//如果pci设备存在并且
			//drhd中的pci domain 不等于pci bus中
			//的domain_nr
			continue;

		//遍历drhd中的dmar_dev_scope
		for_each_active_dev_scope(drhd->devices,
					  drhd->devices_cnt, i, tmp) {

			  //如果这个设备就是传入的设备
			if (tmp == dev) {
				//猜 :  这里的代码应该会执行到。
				*bus = drhd->devices[i].bus;
				*devfn = drhd->devices[i].devfn;
				goto out;
			}

			if (!pdev || !dev_is_pci(tmp))
				continue;

			//pdev存在并且tmp是pci设备下面才会执行，只是
			//tmp不是函数传入的pci设备

			//既然是pci设备，那就获取这个pci设备呗
			ptmp = to_pci_dev(tmp);
			if (ptmp->subordinate &&
			    ptmp->subordinate->number <= pdev->bus->number &&
			    ptmp->subordinate->busn_res.end >= pdev->bus->number)
				goto got_pdev;
		}

		//猜 :  这里的代码应该会执行到。
		if (pdev && drhd->include_all) {
		got_pdev:
			*bus = pdev->bus->number;
			*devfn = pdev->devfn;
			goto out;
		}
	}
	iommu = NULL;
 out:
	rcu_read_unlock();

	return iommu;
}

static void domain_flush_cache(struct dmar_domain *domain,
			       void *addr, int size)
{
	if (!domain->iommu_coherency)
		clflush_cache_range(addr, size);
}

static int device_context_mapped(struct intel_iommu *iommu, u8 bus, u8 devfn)
{
	struct context_entry *context;
	int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&iommu->lock, flags);
	context = iommu_context_addr(iommu, bus, devfn, 0);
	if (context)
		ret = context_present(context);
	spin_unlock_irqrestore(&iommu->lock, flags);
	return ret;
}

static void clear_context_table(struct intel_iommu *iommu, u8 bus, u8 devfn)
{
	struct context_entry *context;
	unsigned long flags;

	spin_lock_irqsave(&iommu->lock, flags);
	context = iommu_context_addr(iommu, bus, devfn, 0);
	if (context) {
		context_clear_entry(context);
		__iommu_flush_cache(iommu, context, sizeof(*context));
	}
	spin_unlock_irqrestore(&iommu->lock, flags);
}

static void free_context_table(struct intel_iommu *iommu)
{
	int i;
	unsigned long flags;
	struct context_entry *context;

	spin_lock_irqsave(&iommu->lock, flags);
	if (!iommu->root_entry) {
		goto out;
	}
	for (i = 0; i < ROOT_ENTRY_NR; i++) {
		context = iommu_context_addr(iommu, i, 0, 0);
		if (context)
			free_pgtable_page(context);

		if (!ecs_enabled(iommu))
			continue;

		context = iommu_context_addr(iommu, i, 0x80, 0);
		if (context)
			free_pgtable_page(context);

	}
	free_pgtable_page(iommu->root_entry);
	iommu->root_entry = NULL;
out:
	spin_unlock_irqrestore(&iommu->lock, flags);
}
//pfn ：虚拟地址页地址
//target_level：最大级数
//在domain的pgd指向的顶级页表中的target_level级找到
//pfn对应的dma_pte
static struct dma_pte *pfn_to_dma_pte(struct dmar_domain *domain,
				      unsigned long pfn, int *target_level)
{
	struct dma_pte *parent, *pte = NULL;
	int level = agaw_to_level(domain->agaw);
	int offset;

	BUG_ON(!domain->pgd);

	if (!domain_pfn_supported(domain, pfn))
		/* Address beyond IOMMU's addressing capabilities. */
		return NULL;

	parent = domain->pgd;

	while (1) {
		void *tmp_page;

		//得到pfn在当前级页表的偏移量
		//注意。假如有三级页表，顶级的level为3
		//取出在level级中，pfn的值
		offset = pfn_level_offset(pfn, level);

		//取出表项的地址
		pte = &parent[offset];
		if (!*target_level && (dma_pte_superpage(pte) || !dma_pte_present(pte)))
			//*target_level为0并且
			//super page 位为1或者既不能读也不能写
			//退出
			break;
		
		if (level == *target_level)
			break;//当前级数和传入说明指定的级数相等，退出

		if (!dma_pte_present(pte)) {
			//如果这个地址指向的内存也中的数据既不能读也不能写
			
			uint64_t pteval;
			//重新申请一页
			tmp_page = alloc_pgtable_page(domain->nid);

			if (!tmp_page)
				return NULL;

			domain_flush_cache(domain, tmp_page, VTD_PAGE_SIZE);
			//得到dma的页地址
			//低位时权限，可读可写
			pteval = ((uint64_t)virt_to_dma_pfn(tmp_page) << VTD_PAGE_SHIFT) | DMA_PTE_READ | DMA_PTE_WRITE;

			//如果pte->val为0，则pte->val=pteval
			if (cmpxchg64(&pte->val, 0ULL, pteval))
				//如果pte->val已经被设置了。
				/* Someone else set it while we were thinking; use theirs. */
				free_pgtable_page(tmp_page);
			else
				domain_flush_cache(domain, pte, sizeof(*pte));
		}
		if (level == 1)
			break;

		parent = phys_to_virt(dma_pte_addr(pte));
		level--;
	}

	if (!*target_level)
		*target_level = level;

	return pte;
}


/* return address's pte at specific level */
static struct dma_pte *dma_pfn_level_pte(struct dmar_domain *domain,
					 unsigned long pfn,
					 int level, int *large_page)
{
	struct dma_pte *parent, *pte = NULL;
	int total = agaw_to_level(domain->agaw);
	int offset;

	parent = domain->pgd;
	while (level <= total) {
		offset = pfn_level_offset(pfn, total);
		pte = &parent[offset];
		if (level == total)
			return pte;

		if (!dma_pte_present(pte)) {
			*large_page = total;
			break;
		}

		if (dma_pte_superpage(pte)) {
			*large_page = total;
			return pte;
		}

		parent = phys_to_virt(dma_pte_addr(pte));
		total--;
	}
	return NULL;
}

/* clear last level pte, a tlb flush should be followed */
static void dma_pte_clear_range(struct dmar_domain *domain,
				unsigned long start_pfn,
				unsigned long last_pfn)
{
	unsigned int large_page = 1;
	struct dma_pte *first_pte, *pte;

	BUG_ON(!domain_pfn_supported(domain, start_pfn));
	BUG_ON(!domain_pfn_supported(domain, last_pfn));
	BUG_ON(start_pfn > last_pfn);

	/* we don't need lock here; nobody else touches the iova range */
	do {
		large_page = 1;
		first_pte = pte = dma_pfn_level_pte(domain, start_pfn, 1, &large_page);
		if (!pte) {
			start_pfn = align_to_level(start_pfn + 1, large_page + 1);
			continue;
		}
		do {
			dma_clear_pte(pte);
			start_pfn += lvl_to_nr_pages(large_page);
			pte++;
		} while (start_pfn <= last_pfn && !first_pte_in_page(pte));

		domain_flush_cache(domain, first_pte,
				   (void *)pte - (void *)first_pte);

	} while (start_pfn && start_pfn <= last_pfn);
}

static void dma_pte_free_level(struct dmar_domain *domain, int level,
			       struct dma_pte *pte, unsigned long pfn,
			       unsigned long start_pfn, unsigned long last_pfn)
{
	pfn = max(start_pfn, pfn);
	pte = &pte[pfn_level_offset(pfn, level)];

	do {
		unsigned long level_pfn;
		struct dma_pte *level_pte;

		if (!dma_pte_present(pte) || dma_pte_superpage(pte))
			goto next;

		level_pfn = pfn & level_mask(level - 1);
		level_pte = phys_to_virt(dma_pte_addr(pte));

		if (level > 2)
			dma_pte_free_level(domain, level - 1, level_pte,
					   level_pfn, start_pfn, last_pfn);

		/* If range covers entire pagetable, free it */
		if (!(start_pfn > level_pfn ||
		      last_pfn < level_pfn + level_size(level) - 1)) {
			dma_clear_pte(pte);
			domain_flush_cache(domain, pte, sizeof(*pte));
			free_pgtable_page(level_pte);
		}
next:
		pfn += level_size(level);
	} while (!first_pte_in_page(++pte) && pfn <= last_pfn);
}

/* free page table pages. last level pte should already be cleared */
static void dma_pte_free_pagetable(struct dmar_domain *domain,
				   unsigned long start_pfn,
				   unsigned long last_pfn)
{
	BUG_ON(!domain_pfn_supported(domain, start_pfn));
	BUG_ON(!domain_pfn_supported(domain, last_pfn));
	BUG_ON(start_pfn > last_pfn);

	dma_pte_clear_range(domain, start_pfn, last_pfn);

	/* We don't need lock here; nobody else touches the iova range */
	dma_pte_free_level(domain, agaw_to_level(domain->agaw),
			   domain->pgd, 0, start_pfn, last_pfn);

	/* free pgd */
	if (start_pfn == 0 && last_pfn == DOMAIN_MAX_PFN(domain->gaw)) {
		free_pgtable_page(domain->pgd);
		domain->pgd = NULL;
	}
}

/* When a page at a given level is being unlinked from its parent, we don't
   need to *modify* it at all. All we need to do is make a list of all the
   pages which can be freed just as soon as we've flushed the IOTLB and we
   know the hardware page-walk will no longer touch them.
   The 'pte' argument is the *parent* PTE, pointing to the page that is to
   be freed. */
static struct page *dma_pte_list_pagetables(struct dmar_domain *domain,
					    int level, struct dma_pte *pte,
					    struct page *freelist)
{
	struct page *pg;

	pg = pfn_to_page(dma_pte_addr(pte) >> PAGE_SHIFT);
	pg->freelist = freelist;
	freelist = pg;

	if (level == 1)
		return freelist;

	pte = page_address(pg);
	do {
		if (dma_pte_present(pte) && !dma_pte_superpage(pte))
			freelist = dma_pte_list_pagetables(domain, level - 1,
							   pte, freelist);
		pte++;
	} while (!first_pte_in_page(pte));

	return freelist;
}

static struct page *dma_pte_clear_level(struct dmar_domain *domain, int level,
					struct dma_pte *pte, unsigned long pfn,
					unsigned long start_pfn,
					unsigned long last_pfn,
					struct page *freelist)
{
	struct dma_pte *first_pte = NULL, *last_pte = NULL;

	pfn = max(start_pfn, pfn);
	pte = &pte[pfn_level_offset(pfn, level)];

	do {
		unsigned long level_pfn;

		if (!dma_pte_present(pte))
			goto next;

		level_pfn = pfn & level_mask(level);

		/* If range covers entire pagetable, free it */
		if (start_pfn <= level_pfn &&
		    last_pfn >= level_pfn + level_size(level) - 1) {
			/* These suborbinate page tables are going away entirely. Don't
			   bother to clear them; we're just going to *free* them. */
			if (level > 1 && !dma_pte_superpage(pte))
				freelist = dma_pte_list_pagetables(domain, level - 1, pte, freelist);

			dma_clear_pte(pte);
			if (!first_pte)
				first_pte = pte;
			last_pte = pte;
		} else if (level > 1) {
			/* Recurse down into a level that isn't *entirely* obsolete */
			freelist = dma_pte_clear_level(domain, level - 1,
						       phys_to_virt(dma_pte_addr(pte)),
						       level_pfn, start_pfn, last_pfn,
						       freelist);
		}
next:
		pfn += level_size(level);
	} while (!first_pte_in_page(++pte) && pfn <= last_pfn);

	if (first_pte)
		domain_flush_cache(domain, first_pte,
				   (void *)++last_pte - (void *)first_pte);

	return freelist;
}

/* We can't just free the pages because the IOMMU may still be walking
   the page tables, and may have cached the intermediate levels. The
   pages can only be freed after the IOTLB flush has been done. */
//不能随便释放这个页，因为iommu可能还在引用这个页，
//这个也只有在iotlb刷新之后才能释放
//返回domain中虚拟地址对应的页地址
struct page *domain_unmap(struct dmar_domain *domain,
			  unsigned long start_pfn,
			  unsigned long last_pfn)
{
	struct page *freelist = NULL;

	BUG_ON(!domain_pfn_supported(domain, start_pfn));
	BUG_ON(!domain_pfn_supported(domain, last_pfn));
	BUG_ON(start_pfn > last_pfn);

	/* we don't need lock here; nobody else touches the iova range */
	//不用上锁，没有人访问iova range
	//这个函数会刷新
	freelist = dma_pte_clear_level(domain, agaw_to_level(domain->agaw),
				       domain->pgd, 0, start_pfn, last_pfn, NULL);

	/* free pgd */
	if (start_pfn == 0 && last_pfn == DOMAIN_MAX_PFN(domain->gaw)) {
		struct page *pgd_page = virt_to_page(domain->pgd);
		pgd_page->freelist = freelist;//是dma_pte_clear_level的返回值
		freelist = pgd_page;//domain->pgd的物理页地址

		//虚拟地址置空，这个地址被转成页地址，返回了
		domain->pgd = NULL;
	}

	return freelist;
}

void dma_free_pagelist(struct page *freelist)
{
	struct page *pg;

	while ((pg = freelist)) {
		freelist = pg->freelist;
		free_pgtable_page(page_address(pg));
	}
}

/* iommu handling */
//申请内存，根据是否一致性处理这块内存页
//设置iommu的root_entry成员变量
static int iommu_alloc_root_entry(struct intel_iommu *iommu)
{
	struct root_entry *root;
	unsigned long flags;
	//为iommu申请一个内存页，并返回该内存页的虚拟地址。
	root = (struct root_entry *)alloc_pgtable_page(iommu->node);
	if (!root) {
		pr_err("IOMMU: allocating root entry for %s failed\n",
			iommu->name);
		return -ENOMEM;
	}

	//根据寄存器的值，进行是否调用clflush_cache_range函数，
	//处理内存空间
	//Extended Capability Register的[0]位
	__iommu_flush_cache(iommu, root, ROOT_SIZE);

	spin_lock_irqsave(&iommu->lock, flags);
	iommu->root_entry = root;
	spin_unlock_irqrestore(&iommu->lock, flags);

	return 0;
}

//设置root_entry的物理地址
//更新root-entry table的地址
//DMAR_RTADDR_REG只在iommu_set_root_entry这个函数中使用
static void iommu_set_root_entry(struct intel_iommu *iommu)
{
	u64 addr;
	u32 sts;
	unsigned long flag;

	addr = virt_to_phys(iommu->root_entry);
	if (ecs_enabled(iommu))
		addr |= DMA_RTADDR_RTT;

	raw_spin_lock_irqsave(&iommu->register_lock, flag);
	dmar_writeq(iommu->reg + DMAR_RTADDR_REG, addr);//主要是这个操作

	writel(iommu->gcmd | DMA_GCMD_SRTP, iommu->reg + DMAR_GCMD_REG);

	/* Make sure hardware complete it */
	//保证上面的操作完成了
	IOMMU_WAIT_OP(iommu, DMAR_GSTS_REG,
		      readl, (sts & DMA_GSTS_RTPS), sts);

	raw_spin_unlock_irqrestore(&iommu->register_lock, flag);
}

//向iommu的gcmd寄存器的Write Buffer Flush位写1
static void iommu_flush_write_buffer(struct intel_iommu *iommu)
{
	u32 val;
	unsigned long flag;

	if (!rwbf_quirk && !cap_rwbf(iommu->cap))
		return;

	raw_spin_lock_irqsave(&iommu->register_lock, flag);
	writel(iommu->gcmd | DMA_GCMD_WBF, iommu->reg + DMAR_GCMD_REG);

	/* Make sure hardware complete it */
	IOMMU_WAIT_OP(iommu, DMAR_GSTS_REG,
		      readl, (!(val & DMA_GSTS_WBFS)), val);

	raw_spin_unlock_irqrestore(&iommu->register_lock, flag);
}

/* return value determine if we need a write buffer flush */
static void __iommu_flush_context(struct intel_iommu *iommu,
				  u16 did, u16 source_id, u8 function_mask,
				  u64 type)
{
	u64 val = 0;
	unsigned long flag;

	switch (type) {
	case DMA_CCMD_GLOBAL_INVL:
		val = DMA_CCMD_GLOBAL_INVL;
		break;
	case DMA_CCMD_DOMAIN_INVL:
		val = DMA_CCMD_DOMAIN_INVL|DMA_CCMD_DID(did);
		break;
	case DMA_CCMD_DEVICE_INVL:
		val = DMA_CCMD_DEVICE_INVL|DMA_CCMD_DID(did)
			| DMA_CCMD_SID(source_id) | DMA_CCMD_FM(function_mask);
		break;
	default:
		BUG();
	}
	val |= DMA_CCMD_ICC;

	raw_spin_lock_irqsave(&iommu->register_lock, flag);
	dmar_writeq(iommu->reg + DMAR_CCMD_REG, val);

	/* Make sure hardware complete it */
	IOMMU_WAIT_OP(iommu, DMAR_CCMD_REG,
		dmar_readq, (!(val & DMA_CCMD_ICC)), val);

	raw_spin_unlock_irqrestore(&iommu->register_lock, flag);
}

/* return value determine if we need a write buffer flush */
static void __iommu_flush_iotlb(struct intel_iommu *iommu, u16 did,
				u64 addr, unsigned int size_order, u64 type)
{
	int tlb_offset = ecap_iotlb_offset(iommu->ecap);
	u64 val = 0, val_iva = 0;
	unsigned long flag;

	switch (type) {
	case DMA_TLB_GLOBAL_FLUSH:
		/* global flush doesn't need set IVA_REG */
		val = DMA_TLB_GLOBAL_FLUSH|DMA_TLB_IVT;
		break;
	case DMA_TLB_DSI_FLUSH:
		val = DMA_TLB_DSI_FLUSH|DMA_TLB_IVT|DMA_TLB_DID(did);
		break;
	case DMA_TLB_PSI_FLUSH:
		val = DMA_TLB_PSI_FLUSH|DMA_TLB_IVT|DMA_TLB_DID(did);
		/* IH bit is passed in as part of address */
		val_iva = size_order | addr;
		break;
	default:
		BUG();
	}
	/* Note: set drain read/write */
#if 0
	/*
	 * This is probably to be super secure.. Looks like we can
	 * ignore it without any impact.
	 */
	if (cap_read_drain(iommu->cap))
		val |= DMA_TLB_READ_DRAIN;
#endif
	if (cap_write_drain(iommu->cap))
		val |= DMA_TLB_WRITE_DRAIN;

	raw_spin_lock_irqsave(&iommu->register_lock, flag);
	/* Note: Only uses first TLB reg currently */
	if (val_iva)
		dmar_writeq(iommu->reg + tlb_offset, val_iva);
	dmar_writeq(iommu->reg + tlb_offset + 8, val);

	/* Make sure hardware complete it */
	IOMMU_WAIT_OP(iommu, tlb_offset + 8,
		dmar_readq, (!(val & DMA_TLB_IVT)), val);

	raw_spin_unlock_irqrestore(&iommu->register_lock, flag);

	/* check IOTLB invalidation granularity */
	if (DMA_TLB_IAIG(val) == 0)
		printk(KERN_ERR"IOMMU: flush IOTLB failed\n");
	if (DMA_TLB_IAIG(val) != DMA_TLB_IIRG(type))
		pr_debug("IOMMU: tlb flush request %Lx, actual %Lx\n",
			(unsigned long long)DMA_TLB_IIRG(type),
			(unsigned long long)DMA_TLB_IAIG(val));
}
pci_iommu_init
//从domain的deivice成员的链表中找到参数传递的device_domain_info

static struct device_domain_info *
iommu_support_dev_iotlb (struct dmar_domain *domain, struct intel_iommu *iommu,
			 u8 bus, u8 devfn)
{
	bool found = false;
	unsigned long flags;
	struct device_domain_info *info;
	struct pci_dev *pdev;

	if (!ecap_dev_iotlb_support(iommu->ecap))
		return NULL;//如果不支持device tlb

	if (!iommu->qi)
		//如果iommu没有Queued invalidation
		return NULL;

	spin_lock_irqsave(&device_domain_lock, flags);
	
	//遍历domain中的device_domain_info链表，找到与传入参数
	//相对应的device_domain_info
	list_for_each_entry(info, &domain->devices, link)
		if (info->iommu == iommu && info->bus == bus &&
		    info->devfn == devfn) {
			found = true;
			break;
		}
	spin_unlock_irqrestore(&device_domain_lock, flags);

	if (!found || !info->dev || !dev_is_pci(info->dev))
		//没有找到与参数匹配的device_domain_info或者
		//找到了但是device_domain_info中dev不存在或者
		//device_domain_info中的dev存在，但不是pci device
		return NULL;

	pdev = to_pci_dev(info->dev);

	
	if (!pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_ATS))
		return NULL;//没有找到存在的capability

	if (!dmar_find_matched_atsr_unit(pdev))
		return NULL;//如果没找到与pdev匹配的atsr

	return info;
}
//使能pci的ATS capability
static void iommu_enable_dev_iotlb(struct device_domain_info *info)
{
	if (!info || !dev_is_pci(info->dev))
		//如果info为空，或者info中的device不是pci device
		//直接退出
		return;

	// enable the ATS capability
	pci_enable_ats(to_pci_dev(info->dev), VTD_PAGE_SHIFT);
}

//disable pci ats
static void iommu_disable_dev_iotlb(struct device_domain_info *info)
{
	if (!info->dev || !dev_is_pci(info->dev) ||
	    !pci_ats_enabled(to_pci_dev(info->dev)))
		return;

	pci_disable_ats(to_pci_dev(info->dev));
}

static void iommu_flush_dev_iotlb(struct dmar_domain *domain,
				  u64 addr, unsigned mask)
{
	u16 sid, qdep;
	unsigned long flags;
	struct device_domain_info *info;

	spin_lock_irqsave(&device_domain_lock, flags);
	list_for_each_entry(info, &domain->devices, link) {
		struct pci_dev *pdev;
		if (!info->dev || !dev_is_pci(info->dev))
			continue;//如果dev为空，或者dev是不是pci dev

		pdev = to_pci_dev(info->dev);//转换成pci dev
		if (!pci_ats_enabled(pdev))
			continue;//pci的Address Translation Service没有使能

		//高8位是pci的bus number，低8位是pci的devfn number
		sid = info->bus << 8 | info->devfn;
		qdep = pci_ats_queue_depth(pdev);//返回dev->ats->qdep;
		qi_flush_dev_iotlb(info->iommu, sid, qdep, addr, mask);
	}
	spin_unlock_irqrestore(&device_domain_lock, flags);
}

static void iommu_flush_iotlb_psi(struct intel_iommu *iommu, u16 did,
				  unsigned long pfn, unsigned int pages, int ih, int map)
{
	unsigned int mask = ilog2(__roundup_pow_of_two(pages));
	uint64_t addr = (uint64_t)pfn << VTD_PAGE_SHIFT;

	BUG_ON(pages == 0);

	if (ih)
		ih = 1 << 6;
	/*
	 * Fallback to domain selective flush if no PSI support or the size is
	 * too big.
	 * PSI requires page size to be 2 ^ x, and the base address is naturally
	 * aligned to the size
	 */
	if (!cap_pgsel_inv(iommu->cap) || mask > cap_max_amask_val(iommu->cap))
		iommu->flush.flush_iotlb(iommu, did, 0, 0,
						DMA_TLB_DSI_FLUSH);
	else
		iommu->flush.flush_iotlb(iommu, did, addr | ih, mask,
						DMA_TLB_PSI_FLUSH);

	/*
	 * In caching mode, changes of pages from non-present to present require
	 * flush. However, device IOTLB doesn't need to be flushed in this case.
	 */
	if (!cap_caching_mode(iommu->cap) || !map)
		iommu_flush_dev_iotlb(iommu->domains[did], addr, mask);
}

static void iommu_disable_protect_mem_regions(struct intel_iommu *iommu)
{
	u32 pmen;
	unsigned long flags;

	raw_spin_lock_irqsave(&iommu->register_lock, flags);
	pmen = readl(iommu->reg + DMAR_PMEN_REG);
	pmen &= ~DMA_PMEN_EPM;
	writel(pmen, iommu->reg + DMAR_PMEN_REG);

	/* wait for the protected region status bit to clear */
	IOMMU_WAIT_OP(iommu, DMAR_PMEN_REG,
		readl, !(pmen & DMA_PMEN_PRS), pmen);

	raw_spin_unlock_irqrestore(&iommu->register_lock, flags);
}

static void iommu_enable_translation(struct intel_iommu *iommu)
{
	u32 sts;
	unsigned long flags;

	raw_spin_lock_irqsave(&iommu->register_lock, flags);
	iommu->gcmd |= DMA_GCMD_TE;
	writel(iommu->gcmd, iommu->reg + DMAR_GCMD_REG);

	/* Make sure hardware complete it */
	IOMMU_WAIT_OP(iommu, DMAR_GSTS_REG,
		      readl, (sts & DMA_GSTS_TES), sts);

	raw_spin_unlock_irqrestore(&iommu->register_lock, flags);
}

//禁止转换，设置iommu的Global Command 寄存器的[31]=0
static void iommu_disable_translation(struct intel_iommu *iommu)
{
	u32 sts;
	unsigned long flag;
	//离开临界区之后保存中断的状态，进入时是什么状态，
	//离开时还是什么状态
	raw_spin_lock_irqsave(&iommu->register_lock, flag);
	iommu->gcmd &= ~DMA_GCMD_TE;//Global Command Register的[31]=0表示禁止转换
	//设置iommu的Global Command Register
	writel(iommu->gcmd, iommu->reg + DMAR_GCMD_REG);//寄存器的值写入寄存器

	/* Make sure hardware complete it */
	//等待Translation Enable Status的值为0，说明禁止转换设置成功
	IOMMU_WAIT_OP(iommu, DMAR_GSTS_REG,
		      readl, (!(sts & DMA_GSTS_TES)), sts);

	raw_spin_unlock_irqrestore(&iommu->register_lock, flag);//中断的状态不变
}

//iommu有多少个domain就申请多少个domain数组
static int iommu_init_domains(struct intel_iommu *iommu)
{
	unsigned long ndomains;
	unsigned long nlongs;

	ndomains = cap_ndoms(iommu->cap);
	pr_debug("IOMMU%d: Number of Domains supported <%ld>\n",
		 iommu->seq_id, ndomains);
	//一个long是32位，也就是一个long可以表示32个domain
	//将domain的个数转成有几个long长度
	//有几个32
	nlongs = BITS_TO_LONGS(ndomains);

	spin_lock_init(&iommu->lock);

	/* TBD: there might be 64K domains,
	 * consider other allocation for future chip
	 */
	 //申请一个domain id数组
	iommu->domain_ids = kcalloc(nlongs, sizeof(unsigned long), GFP_KERNEL);
	if (!iommu->domain_ids) {
		pr_err("IOMMU%d: allocating domain id array failed\n",
		       iommu->seq_id);
		return -ENOMEM;
	}
	
	//申请对应数量的domain数组指针
	iommu->domains = kcalloc(ndomains, sizeof(struct dmar_domain *),
			GFP_KERNEL);
	if (!iommu->domains) {
		pr_err("IOMMU%d: allocating domain array failed\n",
		       iommu->seq_id);
		kfree(iommu->domain_ids);
		iommu->domain_ids = NULL;
		return -ENOMEM;
	}

	/*
	 * if Caching mode is set, then invalid translations are tagged
	 * with domainid 0. Hence we need to pre-allocate it.
	 */
	 
	 //Capability Register[7]
	 //如果Caching mode被置位了，domain id0应该标记无效转换，
	 //因次我们需要预先申请它
	if (cap_caching_mode(iommu->cap))//只取[7]的值，如果该值为1，
		//将iommu->domain_ids的第0位，置1
		set_bit(0, iommu->domain_ids);
	return 0;
}

static void disable_dmar_iommu(struct intel_iommu *iommu)
{
	struct dmar_domain *domain;
	int i;

	if ((iommu->domains) && (iommu->domain_ids)) {
		for_each_set_bit(i, iommu->domain_ids, cap_ndoms(iommu->cap)) {
			/*
			 * Domain id 0 is reserved for invalid translation
			 * if hardware supports caching mode.
			 */
			if (cap_caching_mode(iommu->cap) && i == 0)
				continue;

			domain = iommu->domains[i];
			clear_bit(i, iommu->domain_ids);
			if (domain_detach_iommu(domain, iommu) == 0 &&
			    !domain_type_is_vm(domain))
				domain_exit(domain);
		}
	}

	if (iommu->gcmd & DMA_GCMD_TE)
		iommu_disable_translation(iommu);
}

static void free_dmar_iommu(struct intel_iommu *iommu)
{
	if ((iommu->domains) && (iommu->domain_ids)) {
		kfree(iommu->domains);
		kfree(iommu->domain_ids);
		iommu->domains = NULL;
		iommu->domain_ids = NULL;
	}

	g_iommus[iommu->seq_id] = NULL;

	/* free context mapping */
	free_context_table(iommu);
}
//申请一个dmar_domain并对成员变量初始化
//flags 这个domain的类型
static struct dmar_domain *alloc_domain(int flags)
{
	/* domain id for virtual machine, it won't be set in context */
	static atomic_t vm_domid = ATOMIC_INIT(0);
	struct dmar_domain *domain;

	domain = alloc_domain_mem();
	if (!domain)
		return NULL;

	memset(domain, 0, sizeof(*domain));
	domain->nid = -1;
	domain->flags = flags;
	spin_lock_init(&domain->iommu_lock);
	INIT_LIST_HEAD(&domain->devices);
	if (flags & DOMAIN_FLAG_VIRTUAL_MACHINE)
		domain->id = atomic_inc_return(&vm_domid);

	return domain;
}

//domain给iommu
//iommu->domains[num] = domain;
//返回domain在iommu中的位置
static int __iommu_attach_domain(struct dmar_domain *domain,
				 struct intel_iommu *iommu)
{
	int num;
	unsigned long ndomains;

	ndomains = cap_ndoms(iommu->cap);//获得domain的数量

	//在domain的位图指针指向的区域中找到第一个0位
	num = find_first_zero_bit(iommu->domain_ids, ndomains);
	if (num < ndomains) {
		set_bit(num, iommu->domain_ids);
		iommu->domains[num] = domain;//主要的代码
	} else {
		num = -ENOSPC;
	}

	return num;
}
//iommu中的位图domain_ids，第一个0位的下标值，
//就是iommu中domains的数组下标值。他指向的就是
//第一个参数domain，返回下标值
static int iommu_attach_domain(struct dmar_domain *domain,
			       struct intel_iommu *iommu)
{
	int num;
	unsigned long flags;

	spin_lock_irqsave(&iommu->lock, flags);
	num = __iommu_attach_domain(domain, iommu);
	spin_unlock_irqrestore(&iommu->lock, flags);
	if (num < 0)
		pr_err("IOMMU: no free domain ids\n");

	return num;
}

//把domain给iommu，返回在iommu中的位置
static int iommu_attach_vm_domain(struct dmar_domain *domain,
				  struct intel_iommu *iommu)
{
	int num;
	unsigned long ndomains;

	//这个domain是否已经给iommu了
	ndomains = cap_ndoms(iommu->cap);
	for_each_set_bit(num, iommu->domain_ids, ndomains)
		if (iommu->domains[num] == domain)
			return num;

	return __iommu_attach_domain(domain, iommu);
}
//把这个domain从这个iommu中移除，
//这个iommu中的bitmap和domains数组中可能有多个位置对应
//这个domain，也有可能只有一个位置，
//关键就是看domain的类型了
//这个domain如果是虚拟的，可能在这个iommu中，也可能不在
//如果不是虚拟的，这个domain必须在这个iommu中
//调用这个函数前必须对参数进行判断
static void iommu_detach_domain(struct dmar_domain *domain,
				struct intel_iommu *iommu)
{
	unsigned long flags;
	int num, ndomains;

	spin_lock_irqsave(&iommu->lock, flags);
	if (domain_type_is_vm_or_si(domain)) {
		//如果domain的类型是虚拟机器或者是static identity
		//这两种类型的domain会对应多个设备
		ndomains = cap_ndoms(iommu->cap);
		for_each_set_bit(num, iommu->domain_ids, ndomains) {
			//num是每次遍历的下标。ndomains是结束条件
			if (iommu->domains[num] == domain) {
				clear_bit(num, iommu->domain_ids);//清空iommu中位图对应的位
				iommu->domains[num] = NULL;//清空数组中对该domain的引用
				break;
			}
		}
	} else {
		clear_bit(domain->id, iommu->domain_ids);//清空iommu中位图中的domain编号
		iommu->domains[domain->id] = NULL;//清空iommu中对这个domain的引用
	}
	spin_unlock_irqrestore(&iommu->lock, flags);
}

//更新domain被引用的次数。
//设置domain中的bitmap中iommu的标号的位置置1，表示domain被哪个
//iommu引用
//IOMMU给domain
static void domain_attach_iommu(struct dmar_domain *domain,
			       struct intel_iommu *iommu)
{
	unsigned long flags;

	spin_lock_irqsave(&domain->iommu_lock, flags);

	//domain中的bitmap哪位置1了，就说明被哪个iommu引用了
	if (!test_and_set_bit(iommu->seq_id, domain->iommu_bmp)) {
		//domain->iommu_bmp中的iommu->seq_id位之前需要是0，才执行
		//此时置1
		domain->iommu_count++;//更新该domain被引用的次数
		if (domain->iommu_count == 1)//如果这个domain只被引用了一次
			domain->nid = iommu->node;//标记iommu在哪个内存节点上面
		domain_update_iommu_cap(domain);
	}
	spin_unlock_irqrestore(&domain->iommu_lock, flags);
}

static int domain_detach_iommu(struct dmar_domain *domain,
			       struct intel_iommu *iommu)
{
	unsigned long flags;
	int count = INT_MAX;

	spin_lock_irqsave(&domain->iommu_lock, flags);
	if (test_and_clear_bit(iommu->seq_id, domain->iommu_bmp)) {
		count = --domain->iommu_count;
		domain_update_iommu_cap(domain);
	}
	spin_unlock_irqrestore(&domain->iommu_lock, flags);

	return count;
}

//默认的iova domain
static struct iova_domain reserved_iova_list;
static struct lock_class_key reserved_rbtree_key;

static int dmar_init_reserved_ranges(void)
{
	struct pci_dev *pdev = NULL;
	struct iova *iova;
	int i;

	init_iova_domain(&reserved_iova_list, VTD_PAGE_SIZE, IOVA_START_PFN,
			DMA_32BIT_PFN);

	lockdep_set_class(&reserved_iova_list.iova_rbtree_lock,
		&reserved_rbtree_key);

	/* IOAPIC ranges shouldn't be accessed by DMA */
	iova = reserve_iova(&reserved_iova_list, IOVA_PFN(IOAPIC_RANGE_START),
		IOVA_PFN(IOAPIC_RANGE_END));
	if (!iova) {
		printk(KERN_ERR "Reserve IOAPIC range failed\n");
		return -ENODEV;
	}

	/* Reserve all PCI MMIO to avoid peer-to-peer access */
	for_each_pci_dev(pdev) {
		struct resource *r;

		for (i = 0; i < PCI_NUM_RESOURCES; i++) {
			r = &pdev->resource[i];
			if (!r->flags || !(r->flags & IORESOURCE_MEM))
				continue;
			iova = reserve_iova(&reserved_iova_list,
					    IOVA_PFN(r->start),
					    IOVA_PFN(r->end));
			if (!iova) {
				printk(KERN_ERR "Reserve iova failed\n");
				return -ENODEV;
			}
		}
	}
	return 0;
}

//为domain的iova domain的红黑树设置默认的值
static void domain_reserve_special_ranges(struct dmar_domain *domain)
{
	copy_reserved_iova(&reserved_iova_list, &domain->iovad);
}

//根据domain的地址宽度，计算agaw的值
//最大为64
static inline int guestwidth_to_adjustwidth(int gaw)
{
	int agaw;
	int r = (gaw - 12) % 9;

	if (r == 0)
		agaw = gaw;
	else
		agaw = gaw + 9 - r;
	if (agaw > 64)
		agaw = 64;
	return agaw;
}

static int domain_init(struct dmar_domain *domain, int guest_width)
{
	struct intel_iommu *iommu;
	int adjust_width, agaw;
	unsigned long sagaw;

	init_iova_domain(&domain->iovad, VTD_PAGE_SIZE, IOVA_START_PFN,
			DMA_32BIT_PFN);
	domain_reserve_special_ranges(domain);

	/* calculate AGAW */
	iommu = domain_get_iommu(domain);
	if (guest_width > cap_mgaw(iommu->cap))
		guest_width = cap_mgaw(iommu->cap);
	domain->gaw = guest_width;
	adjust_width = guestwidth_to_adjustwidth(guest_width);
	agaw = width_to_agaw(adjust_width);
	sagaw = cap_sagaw(iommu->cap);
	if (!test_bit(agaw, &sagaw)) {
		/* hardware doesn't support it, choose a bigger one */
		pr_debug("IOMMU: hardware doesn't support agaw %d\n", agaw);
		agaw = find_next_bit(&sagaw, 5, agaw);
		if (agaw >= 5)
			return -ENODEV;
	}
	domain->agaw = agaw;

	if (ecap_coherent(iommu->ecap))
		domain->iommu_coherency = 1;
	else
		domain->iommu_coherency = 0;

	if (ecap_sc_support(iommu->ecap))
		domain->iommu_snooping = 1;
	else
		domain->iommu_snooping = 0;

	if (intel_iommu_superpage)
		domain->iommu_superpage = fls(cap_super_page_val(iommu->cap));
	else
		domain->iommu_superpage = 0;

	domain->nid = iommu->node;

	/* always allocate the top pgd */
	domain->pgd = (struct dma_pte *)alloc_pgtable_page(domain->nid);
	if (!domain->pgd)
		return -ENOMEM;
	__iommu_flush_cache(iommu, domain->pgd, PAGE_SIZE);
	return 0;
}

//将制定的domain从系统中退出，返还给domain cache
static void domain_exit(struct dmar_domain *domain)
{
	struct dmar_drhd_unit *drhd;
	struct intel_iommu *iommu;
	struct page *freelist = NULL;

	/* Domain 0 is reserved, so dont process it */
	if (!domain)
		return;

	/* Flush any lazy unmaps that may reference this domain */
	//刷新没有unmap的区域,这些区域可能引用了这个domain
	if (!intel_iommu_strict)
		flush_unmaps_timeout(0);

	/* remove associated devices */
	//释放与domain相关联的device_domain_info结构体
	domain_remove_dev_info(domain);

	/* destroy iovas */
	//释放domain中的iova domain
	put_iova_domain(&domain->iovad);
	
	//把domain的虚拟地址置null，然后返回该虚拟地址对应的页地址
	freelist = domain_unmap(domain, 0, DOMAIN_MAX_PFN(domain->gaw));

	/* clear attached or cached domains */
	rcu_read_lock();
	for_each_active_iommu(iommu, drhd)
		//iommu_detach_domain种与两个分支，
		//domain_type_is_vm走的是第一个分支
		//test_bit走的是第二个分支
		if (domain_type_is_vm(domain) ||
		    test_bit(iommu->seq_id, domain->iommu_bmp))
		    //如果这个domain是vm或者这个domain引用了这个iommu
			iommu_detach_domain(domain, iommu);//移除iommu中的domain
	rcu_read_unlock();

	//释放domain的中的虚拟地址
	dma_free_pagelist(freelist);

	//释放这个domain
	free_domain_mem(domain);
}
//bus 是iommu的一级页表下标
//devfn 是iommu的二级页表下标
//设置iommu中二级页表某一项的内容
static int domain_context_mapping_one(struct dmar_domain *domain,
				      struct intel_iommu *iommu,
				      u8 bus, u8 devfn, int translation)
{
	struct context_entry *context;
	unsigned long flags;
	struct dma_pte *pgd;
	int id;
	int agaw;
	struct device_domain_info *info = NULL;

	pr_debug("Set context mapping for %02x:%02x.%d\n",
		bus, PCI_SLOT(devfn), PCI_FUNC(devfn));

	BUG_ON(!domain->pgd);
	BUG_ON(translation != CONTEXT_TT_PASS_THROUGH &&
	       translation != CONTEXT_TT_MULTI_LEVEL);

	spin_lock_irqsave(&iommu->lock, flags);
	//获得iommu中指定的context地址，如果二级页表没有
	//允许申请
	context = iommu_context_addr(iommu, bus, devfn, 1);
	spin_unlock_irqrestore(&iommu->lock, flags);
	if (!context)
		return -ENOMEM;
	
	spin_lock_irqsave(&iommu->lock, flags);
	if (context_present(context)) {
		//如果低地址的内存有效
		spin_unlock_irqrestore(&iommu->lock, flags);
		return 0;
	}

	id = domain->id;
	pgd = domain->pgd;

	if (domain_type_is_vm_or_si(domain)) {
		if (domain_type_is_vm(domain)) {
			id = iommu_attach_vm_domain(domain, iommu);
			if (id < 0) {
				spin_unlock_irqrestore(&iommu->lock, flags);
				pr_err("IOMMU: no free domain ids\n");
				return -EFAULT;
			}
		}

		/* Skip top levels of page tables for
		 * iommu which has less agaw than default.
		 * Unnecessary for PT mode.
		 */
		if (translation != CONTEXT_TT_PASS_THROUGH) {
			for (agaw = domain->agaw; agaw != iommu->agaw; agaw--) {
				pgd = phys_to_virt(dma_pte_addr(pgd));
				if (!dma_pte_present(pgd)) {
					//既不可读也不可写的地址
					spin_unlock_irqrestore(&iommu->lock, flags);
					return -ENOMEM;
				}
			}
		}
	}

	//设置contxt中domain id的值
	context_set_domain_id(context, id);

	if (translation != CONTEXT_TT_PASS_THROUGH) {
		info = iommu_support_dev_iotlb(domain, iommu, bus, devfn);
		translation = info ? CONTEXT_TT_DEV_IOTLB :
				     CONTEXT_TT_MULTI_LEVEL;
	}
	/*
	 * In pass through mode, AW must be programmed to indicate the largest
	 * AGAW value supported by hardware. And ASR is ignored by hardware.
	 */
	if (unlikely(translation == CONTEXT_TT_PASS_THROUGH))
		context_set_address_width(context, iommu->msagaw);
	else {
		//root address
		context_set_address_root(context, virt_to_phys(pgd));
		context_set_address_width(context, iommu->agaw);
	}

	context_set_translation_type(context, translation);
	context_set_fault_enable(context);
	context_set_present(context);
	domain_flush_cache(domain, context, sizeof(*context));

	/*
	 * It's a non-present to present mapping. If hardware doesn't cache
	 * non-present entry we only need to flush the write-buffer. If the
	 * _does_ cache non-present entries, then it does so in the special
	 * domain #0, which we have to flush:
	 */
	if (cap_caching_mode(iommu->cap)) {
		iommu->flush.flush_context(iommu, 0,
					   (((u16)bus) << 8) | devfn,
					   DMA_CCMD_MASK_NOBIT,
					   DMA_CCMD_DEVICE_INVL);
		iommu->flush.flush_iotlb(iommu, id, 0, 0, DMA_TLB_DSI_FLUSH);
	} else {
		iommu_flush_write_buffer(iommu);
	}
	iommu_enable_dev_iotlb(info);
	spin_unlock_irqrestore(&iommu->lock, flags);

	domain_attach_iommu(domain, iommu);

	return 0;
}

struct domain_context_mapping_data {
	struct dmar_domain *domain;
	struct intel_iommu *iommu;
	int translation;//context entry中translation域的值
};

static int domain_context_mapping_cb(struct pci_dev *pdev,
				     u16 alias, void *opaque)
{
	struct domain_context_mapping_data *data = opaque;

	return domain_context_mapping_one(data->domain, data->iommu,
					  PCI_BUS_NUM(alias), alias & 0xff,
					  data->translation);
}

///* context entry init */
//将domain的pgd的内容写入到dev对应的iommu中的二级页表的某一项
//translation是页表项中translation的值
//pci 设备可以填写多个页表项，
//非pci设备只能填充一项
static int
domain_context_mapping(struct dmar_domain *domain, struct device *dev,
		       int translation)
{
	struct intel_iommu *iommu;
	u8 bus, devfn;
	struct domain_context_mapping_data data;

	//返回一个与dev相关联的iommu和bus，devfn号码
	iommu = device_to_iommu(dev, &bus, &devfn);
	if (!iommu)
		return -ENODEV;

	if (!dev_is_pci(dev))
		//如果设备不是pci设备
		
		return domain_context_mapping_one(domain, iommu, bus, devfn,
						  translation);
	//设备是pci设备
	data.domain = domain;
	data.iommu = iommu;
	data.translation = translation;

	//遍历dmar alizas ，从第一个变量开始，每个调用
	//domain_context_mapping_cb函数传入的参数是data
	//每次映射一次
	//将pci_device根据bus 号加入
	return pci_for_each_dma_alias(to_pci_dev(dev),
				      &domain_context_mapping_cb, &data);
}

static int domain_context_mapped_cb(struct pci_dev *pdev,
				    u16 alias, void *opaque)
{
	struct intel_iommu *iommu = opaque;

	return !device_context_mapped(iommu, PCI_BUS_NUM(alias), alias & 0xff);
}

static int domain_context_mapped(struct device *dev)
{
	struct intel_iommu *iommu;
	u8 bus, devfn;

	iommu = device_to_iommu(dev, &bus, &devfn);
	if (!iommu)
		return -ENODEV;

	if (!dev_is_pci(dev))
		return device_context_mapped(iommu, bus, devfn);

	return !pci_for_each_dma_alias(to_pci_dev(dev),
				       domain_context_mapped_cb, iommu);
}

/* Returns a number of VTD pages, but aligned to MM page size */
//返回iommu支持的页大小的数量，起始地址与mm page size对齐
//也就是一共多少页
static inline unsigned long aligned_nrpages(unsigned long host_addr,
					    size_t size)
{
	host_addr &= ~PAGE_MASK;//把低PAGE_SHIFT保留，其余清零
	return PAGE_ALIGN(host_addr + size) >> VTD_PAGE_SHIFT;
}

/* Return largest possible superpage level for a given mapping */
//根据给定的映射返回最大支持的级数
//返回最大硬件支持的级数，该级数可能和domain中iommu_superpage
//的值不一致。
static inline int hardware_largepage_caps(struct dmar_domain *domain,
					  unsigned long iov_pfn,
					  unsigned long phy_pfn,
					  unsigned long pages)
{
	int support, level = 1;
	unsigned long pfnmerge;

	//获得domain中Level of superpages
	support = domain->iommu_superpage;

	/* To use a large page, the virtual *and* physical addresses
	   must be aligned to 2MiB/1GiB/etc. Lower bits set in either
	   of them will mean we have to use smaller pages. So just
	   merge them and check both at once. */
	   //为了使用大页，虚拟地址和物理地址
	   //必须和2MiB/1GiB/etc对齐，他们任何一个低位的值
	   //被设置了将意味着必须使用比当前小一级的页，因此同时需要
	   //合并和检查。
	   //因为页大小是4k，12位，加9位就是2M，加18位就是1G
	   //相或的目的是判读虚拟地址和物理地址的低位是否
	   //被设置了，被设置了就说明必须使用较小的对齐方式
	pfnmerge = iov_pfn | phy_pfn;
	
	while (support && !(pfnmerge & ~VTD_STRIDE_MASK)) {
	//如果support不为0，
	//并且虚拟地址和物理地址的低9位没有设置值
		pages >>= VTD_STRIDE_SHIFT;
		if (!pages)
			break;//放大级数之后，变成0页了，返回上次的级数
		pfnmerge >>= VTD_STRIDE_SHIFT;
		level++;
		support--;
	}
	return level;
}
//映射的目的是能够根据iov_pfn得到hpa的地址
//过程类似为虚拟地址和物理地址映射的过程
//nr_pages映射多少页
//prot页表中物理地址的权限
//如果映射4M的地址空间，则只填充domain中pgd中的两个表项
//一个表项是2M，将页的两个起始物理地址写入这两个表项中
//表示第一表项内的物理地址到第二个表项内表示的结束物理
//地址为映射的空间
static int __domain_mapping(struct dmar_domain *domain, unsigned long iov_pfn,
			    struct scatterlist *sg, unsigned long phys_pfn,
			    unsigned long nr_pages, int prot)
{
	struct dma_pte *first_pte = NULL, *pte = NULL;
	//物理地址和权限的组合
	phys_addr_t uninitialized_var(pteval);
	unsigned long sg_res = 0;//多少页
	unsigned int largepage_lvl = 0;
	unsigned long lvl_pages = 0;

	//如果虚拟页地址的范围不在domain可访问内，打印栈信息
	BUG_ON(!domain_pfn_supported(domain, iov_pfn + nr_pages - 1));

	if ((prot & (DMA_PTE_READ|DMA_PTE_WRITE)) == 0)
		return -EINVAL;//如果既不能读也不能写就直接返回错误码

	prot &= DMA_PTE_READ | DMA_PTE_WRITE | DMA_PTE_SNP;

	if (!sg) {
		//我们传进来的是NULL
		sg_res = nr_pages;
		//物理页号的地址和权限都在这个地址中体现了
		pteval = ((phys_addr_t)phys_pfn << VTD_PAGE_SHIFT) | prot;
	}

	while (nr_pages > 0) {//还需要映射的页的数量
		uint64_t tmp;

		if (!sg_res) {//不执行
			sg_res = aligned_nrpages(sg->offset, sg->length);
			sg->dma_address = ((dma_addr_t)iov_pfn << VTD_PAGE_SHIFT) + sg->offset;
			sg->dma_length = sg->length;
			pteval = page_to_phys(sg_page(sg)) | prot;
			phys_pfn = pteval >> VTD_PAGE_SHIFT;
		}

		if (!pte) {
			//如果pte为0执行，
			//第一次的时候会执行，指向页表的第一项也会执行
			
			//返回最大支持的级数
			//返回哪一级页表
			largepage_lvl = hardware_largepage_caps(domain, iov_pfn, phys_pfn, sg_res);

			//在domain的pgd中找到iov_pfn指向的dma_pte
			//把iov_pfn转成largepage_lvl级的值
			first_pte = pte = pfn_to_dma_pte(domain, iov_pfn, &largepage_lvl);
			if (!pte)
				return -ENOMEM;
			/* It is large page*/
			if (largepage_lvl > 1) {
				//如果是大页的内容
				//如果这是大页
				//nr_superpages祖宗页表的表项编号
				unsigned long nr_superpages, end_pfn;

				//设置地址中的super page域为1
				pteval |= DMA_PTE_LARGE_PAGE;
				//largepage_lvl = 2    ==》lvl_pages  =  10,0000,0000
				//largepage_lvl = 3    ==》lvl_pages  =  100,0000,0000,0000,0000
				lvl_pages = lvl_to_nr_pages(largepage_lvl);

				//下面两行代码可以只保留当前页表的索引值
				//假如 lvl_pages=10,0000,0000
				//sg_res右移9位
				nr_superpages = sg_res / lvl_pages;
				//虚拟页表的最后一页编号
				end_pfn = iov_pfn + nr_superpages * lvl_pages - 1;

				/*
				 * Ensure that old small page tables are
				 * removed to make room for superpage(s).
				 */
				 //确保以前小的页表被移除了
				dma_pte_free_pagetable(domain, iov_pfn, end_pfn);
			} else {
				//super page域清0
				pteval &= ~(uint64_t)DMA_PTE_LARGE_PAGE;
			}

		}
		/* We don't need lock here, nobody else
		 * touches the iova range
		 */
		 //之前如果内容为0，把物理地址和其他域的值
		 //写入到这个表项里面
		 //写入成功返回0uLL
		tmp = cmpxchg64_local(&pte->val, 0ULL, pteval);
		if (tmp) {
			//写入失败，之前这个表项已经有值了
			static int dumps = 5;
			//这个页表项的内容已经被设置了，把以前的内容和现在的内容都打印出来
			printk(KERN_CRIT "ERROR: DMA PTE for vPFN 0x%lx already set (to %llx not %llx)\n",
			       iov_pfn, tmp, (unsigned long long)pteval);
			if (dumps) {
				dumps--;
				debug_dma_dump_mappings(NULL);
			}
			WARN_ON(1);
		}
		
		//因为largepage_lvl 可能大于1，也有可能不大于1
		lvl_pages = lvl_to_nr_pages(largepage_lvl);

		BUG_ON(nr_pages < lvl_pages);//小于一个页表项的内容，也就是不到一个页表项
		BUG_ON(sg_res < lvl_pages);

		//减去已经映射的大小，更新还要映射的页数量
		nr_pages -= lvl_pages;
		//更新下次要映射的开始页地址
		iov_pfn += lvl_pages;
		phys_pfn += lvl_pages;
		//更新要写入的值
		pteval += lvl_pages * VTD_PAGE_SIZE;
		sg_res -= lvl_pages;

		/* If the next PTE would be the first in a new page, then we
		   need to flush the cache on the entries we've just written.
		   And then we'll need to recalculate 'pte', so clear it and
		   let it get set again in the if (!pte) block above.

		   If we're done (!nr_pages) we need to flush the cache too.

		   Also if we've been setting superpages, we may need to
		   recalculate 'pte' and switch back to smaller pages for the
		   end of the mapping, if the trailing size is not enough to
		   use another superpage (i.e. sg_res < lvl_pages). */
		//指向页表中的下个表项
		pte++;
		if (!nr_pages || first_pte_in_page(pte) ||
		    (largepage_lvl > 1 && sg_res < lvl_pages)) {
		    //下次要映射的页为0，或者
		    //下一个pte为第一页或者
		    //largepage_lvl > 1并且sg_res < lvl_pages
			//刷新
			domain_flush_cache(domain, first_pte,
					   (void *)pte - (void *)first_pte);
			pte = NULL;
		}

		if (!sg_res && nr_pages)
			sg = sg_next(sg);//nr_pages不为0并且sg_res为0
			
	}
	return 0;
}

static inline int domain_sg_mapping(struct dmar_domain *domain, unsigned long iov_pfn,
				    struct scatterlist *sg, unsigned long nr_pages,
				    int prot)
{
	return __domain_mapping(domain, iov_pfn, sg, 0, nr_pages, prot);
}
//iov_pfn：io虚地址的中的页地址
//phys_pfn：物理地址的页地址
//nr_pages：多少页
//prot：权限
static inline int domain_pfn_mapping(struct dmar_domain *domain, unsigned long iov_pfn,
				     unsigned long phys_pfn, unsigned long nr_pages,
				     int prot)
{
	return __domain_mapping(domain, iov_pfn, NULL, phys_pfn, nr_pages, prot);
}

static void iommu_detach_dev(struct intel_iommu *iommu, u8 bus, u8 devfn)
{
	if (!iommu)
		return;

	clear_context_table(iommu, bus, devfn);
	iommu->flush.flush_context(iommu, 0, 0, 0,
					   DMA_CCMD_GLOBAL_INVL);
	iommu->flush.flush_iotlb(iommu, 0, 0, 0, DMA_TLB_GLOBAL_FLUSH);
}

//从对应的链表中删除这个device_domain_info
//并将info->dev->archdata.iommu = NULL;
static inline void unlink_domain_info(struct device_domain_info *info)
{
	assert_spin_locked(&device_domain_lock);
	list_del(&info->link);
	list_del(&info->global);
	if (info->dev)
		info->dev->archdata.iommu = NULL;
}
//释放与domain相关联的device_domain_info结构体
static void domain_remove_dev_info(struct dmar_domain *domain)
{
	struct device_domain_info *info, *tmp;
	unsigned long flags;

	spin_lock_irqsave(&device_domain_lock, flags);
	list_for_each_entry_safe(info, tmp, &domain->devices, link) {
		unlink_domain_info(info);//释放链表里的引用
		spin_unlock_irqrestore(&device_domain_lock, flags);

		iommu_disable_dev_iotlb(info);
		iommu_detach_dev(info->iommu, info->bus, info->devfn);

		if (domain_type_is_vm(domain)) {
			//如果domain是虚拟机器
			iommu_detach_dependent_devices(info->iommu, info->dev);
			domain_detach_iommu(domain, info->iommu);
		}


		//释放device_domain_info这个结构体
		free_devinfo_mem(info);
		spin_lock_irqsave(&device_domain_lock, flags);
	}
	spin_unlock_irqrestore(&device_domain_lock, flags);
}

/*
 * find_domain
 * Note: we use struct device->archdata.iommu stores the info
 */
 //return dev->archdata.iommu->domain
static struct dmar_domain *find_domain(struct device *dev)
{
	struct device_domain_info *info;

	/* No lock here, assumes no domain exit in normal case */
	info = dev->archdata.iommu;
	if (info)
		return info->domain;
	return NULL;
}

//从domain链表中查找domain，
//查找条件根据bus number ，segment ，devfn
static inline struct device_domain_info *
dmar_search_domain_by_dev_info(int segment, int bus, int devfn)
{
	struct device_domain_info *info;

	list_for_each_entry(info, &device_domain_list, global)
		if (info->iommu->segment == segment && info->bus == bus &&
		    info->devfn == devfn)
			return info;

	return NULL;
}
// bus;		/* PCI bus number */
// devfn;		/* PCI devfn number */
//参数全部填充device_domain_info这个结构体，
//如果dev中没有device_domain_info，就将这个device_domain_info放到两个链表中
//返回device_domain_info中的domain，可能是以前的也可能是传入的参数的
//返回插入的domain
static struct dmar_domain *dmar_insert_dev_info(struct intel_iommu *iommu,
						int bus, int devfn,
						struct device *dev,
						struct dmar_domain *domain)
{
	struct dmar_domain *found = NULL;
	struct device_domain_info *info;
	unsigned long flags;

	info = alloc_devinfo_mem();
	if (!info)
		return NULL;

	info->bus = bus;
	info->devfn = devfn;
	info->dev = dev;
	info->domain = domain;
	info->iommu = iommu;

	spin_lock_irqsave(&device_domain_lock, flags);
	if (dev)
		//传入的device不是空的
		found = find_domain(dev);//dev->archdata.iommu->domain
	else {
		//传入的device是空的
		struct device_domain_info *info2;
		info2 = dmar_search_domain_by_dev_info(iommu->segment, bus, devfn);
		if (info2)
			found = info2->domain;
	}
	if (found) {
		//如果这个device有dmar_domain，说明已经插入了，
		//直接返回这个dmar_domain
		spin_unlock_irqrestore(&device_domain_lock, flags);
		free_devinfo_mem(info);
		/* Caller must free the original domain */
		return found;
	}

	//device 没有dmar_domain，device中如果有domain就不会执行到这里

	//把info插入domain->devices链表
	list_add(&info->link, &domain->devices);
	
	//把info插入device_domain_list链表
	list_add(&info->global, &device_domain_list);
	if (dev)
		dev->archdata.iommu = info;//将这个domain赋值给dev
	spin_unlock_irqrestore(&device_domain_lock, flags);

	return domain;
}

static int get_last_alias(struct pci_dev *pdev, u16 alias, void *opaque)
{
	*(u16 *)opaque = alias;
	return 0;
}

/* domain is initialized */
//获得dev对应的一个domain，这个domain是新建立的或者是系统存在的。
static struct dmar_domain *get_domain_for_dev(struct device *dev, int gaw)
{
	struct dmar_domain *domain, *tmp;
	struct intel_iommu *iommu;
	struct device_domain_info *info;
	u16 dma_alias;
	unsigned long flags;
	u8 bus, devfn;

	domain = find_domain(dev);//从dev中获取domain
	if (domain)
		return domain;
	
	//刚才没有获取到domain，说明设备中没有
	//device 和pci domain之间的关系结构体device_domain_info

	//返回值如下，当然还有一种情况
	//*bus = pdev->bus->number;
	//*devfn = pdev->devfn;
	iommu = device_to_iommu(dev, &bus, &devfn);
	if (!iommu)
		return NULL;

	if (dev_is_pci(dev)) {
		struct pci_dev *pdev = to_pci_dev(dev);

		pci_for_each_dma_alias(pdev, get_last_alias, &dma_alias);

		spin_lock_irqsave(&device_domain_lock, flags);
		//通过dev info 找domain
		info = dmar_search_domain_by_dev_info(pci_domain_nr(pdev->bus),
						      PCI_BUS_NUM(dma_alias),
						      dma_alias & 0xff);
		if (info) {
			iommu = info->iommu;
			domain = info->domain;
		}
		spin_unlock_irqrestore(&device_domain_lock, flags);

		/* DMA alias already has a domain, uses it */
		//找到了domain
		if (info)
			goto found_domain;
	}

	//没有找到domain

	/* Allocate and initialize new domain for the device */
	//先申请一个domain
	domain = alloc_domain(0);
	if (!domain)
		return NULL;

	//一个domain只能给一个iommu，一个iommu可以有多个domain
	//下面是这种关系的建立过程
	
	//将申请的domain给iommu，domain->id就是iommu对应的domain，
	//这个domain在iommu中domain指针数组的下标
	domain->id = iommu_attach_domain(domain, iommu);
	if (domain->id < 0) {
		free_domain_mem(domain);
		return NULL;
	}
	domain_attach_iommu(domain, iommu);
	if (domain_init(domain, gaw)) {
		domain_exit(domain);
		return NULL;
	}

//建立结束

	/* register PCI DMA alias device */
	if (dev_is_pci(dev)) {
		//插入一个device_domain_info结构体，
		//返回插入的domain
		tmp = dmar_insert_dev_info(iommu, PCI_BUS_NUM(dma_alias),
					   dma_alias & 0xff, NULL, domain);

		if (!tmp || tmp != domain) {
			//如果插入domain失败，或者没有插入domain，以前有domain了
			//说明这个domain虽然已经建立了，但是没有通过
			//device_domain_info插入系统中，就将这个domain退出系统。
			domain_exit(domain);//这个domain退出系统
			//系统中已经存在的domain或者为null
			domain = tmp;
		}

		if (!domain)
			return NULL;
	}
	
	//domain是系统中已经存在的domain，
	//或者是新建立的domain

	//没有找到domain，这个domain是新申请的。
	//和找到domain了，下面都会执行
found_domain:
	tmp = dmar_insert_dev_info(iommu, bus, devfn, dev, domain);

	if (!tmp || tmp != domain) {
		domain_exit(domain);
		domain = tmp;
	}

	return domain;
}

//iommu 等值映射标志
static int iommu_identity_mapping;
#define IDENTMAP_ALL		1
#define IDENTMAP_GFX		2
#define IDENTMAP_AZALIA		4

//为物理内存做等值映射
static int iommu_domain_identity_map(struct dmar_domain *domain,
				     unsigned long long start,
				     unsigned long long end)
{
	//以页为单位的地址
	unsigned long first_vpfn = start >> VTD_PAGE_SHIFT;
	unsigned long last_vpfn = end >> VTD_PAGE_SHIFT;


	// 为物理内存建立一个iova虚拟内存区域，iova用于管理
	//gpa区域
	if (!reserve_iova(&domain->iovad, dma_to_mm_pfn(first_vpfn),
			  dma_to_mm_pfn(last_vpfn))) {
		printk(KERN_ERR "IOMMU: reserve iova failed\n");
		return -ENOMEM;
	}

	pr_debug("Mapping reserved region %llx-%llx for domain %d\n",
		 start, end, domain->id);
	/*
	 * RMRR range might have overlap with physical memory range,
	 * clear it first
	 */
	dma_pte_clear_range(domain, first_vpfn, last_vpfn);

	return domain_pfn_mapping(domain, first_vpfn, first_vpfn,
				  last_vpfn - first_vpfn + 1,
				  DMA_PTE_READ|DMA_PTE_WRITE);
}

//iommu准备identity map，然后里面会调用
//iommu_domain_identity_map
static int iommu_prepare_identity_map(struct device *dev,
				      unsigned long long start,
				      unsigned long long end)
{
	struct dmar_domain *domain;
	int ret;

	//获取dev对应的domain。
	domain = get_domain_for_dev(dev, DEFAULT_DOMAIN_ADDRESS_WIDTH);
	if (!domain)
		return -ENOMEM;

	//获取到dev对应的domain了。
	
	/* For _hardware_ pass through, don't bother. But for software
	   passthrough, we do it anyway -- it may indicate a memory
	   range which is reserved in E820, so which didn't get set
	   up to start with in si_domain */
	   //不管硬件是否支持pass through技术，软件一定支持
	   //这可能一个保留在E820的内存区域。
	   //因此设置在si domain中启动
	if (domain == si_domain && hw_pass_through) {
		printk("Ignoring identity map for HW passthrough device %s [0x%Lx - 0x%Lx]\n",
		       dev_name(dev), start, end);
		return 0;
	}

	printk(KERN_INFO
	       "IOMMU: Setting identity map for device %s [0x%Lx - 0x%Lx]\n",
	       dev_name(dev), start, end);
	
	if (end < start) {
		WARN(1, "Your BIOS is broken; RMRR ends before it starts!\n"
			"BIOS vendor: %s; Ver: %s; Product Version: %s\n",
			dmi_get_system_info(DMI_BIOS_VENDOR),
			dmi_get_system_info(DMI_BIOS_VERSION),
		     dmi_get_system_info(DMI_PRODUCT_VERSION));
		ret = -EIO;
		goto error;
	}

	if (end >> agaw_to_width(domain->agaw)) {
		WARN(1, "Your BIOS is broken; RMRR exceeds permitted address width (%d bits)\n"
		     "BIOS vendor: %s; Ver: %s; Product Version: %s\n",
		     agaw_to_width(domain->agaw),
		     dmi_get_system_info(DMI_BIOS_VENDOR),
		     dmi_get_system_info(DMI_BIOS_VERSION),
		     dmi_get_system_info(DMI_PRODUCT_VERSION));
		ret = -EIO;
		goto error;
	}

	ret = iommu_domain_identity_map(domain, start, end);
	if (ret)
		goto error;

	/* context entry init */
	ret = domain_context_mapping(domain, dev, CONTEXT_TT_MULTI_LEVEL);
	if (ret)
		goto error;

	return 0;

 error:
	domain_exit(domain);
	return ret;
}

//为pci准备rmrr
static inline int iommu_prepare_rmrr_dev(struct dmar_rmrr_unit *rmrr,
					 struct device *dev)
{
	//  如果等于(struct device_domain_info *)(-1)
	if (dev->archdata.iommu == DUMMY_DEVICE_DOMAIN_INFO)
		return 0;
	return iommu_prepare_identity_map(dev, rmrr->base_address,
					  rmrr->end_address);
}

#ifdef CONFIG_INTEL_IOMMU_FLOPPY_WA
static inline void iommu_prepare_isa(void)
{
	struct pci_dev *pdev;
	int ret;

	pdev = pci_get_class(PCI_CLASS_BRIDGE_ISA << 8, NULL);
	if (!pdev)
		return;

	printk(KERN_INFO "IOMMU: Prepare 0-16MiB unity mapping for LPC\n");
	ret = iommu_prepare_identity_map(&pdev->dev, 0, 16*1024*1024 - 1);

	if (ret)
		printk(KERN_ERR "IOMMU: Failed to create 0-16MiB identity map; "
		       "floppy might not work\n");

	pci_dev_put(pdev);
}
#else
static inline void iommu_prepare_isa(void)
{
	return;
}
#endif /* !CONFIG_INTEL_IOMMU_FLPY_WA */

static int md_domain_init(struct dmar_domain *domain, int guest_width);

//为iommu添加一个domain,该domain为等值映射domain
//等值映射domain的初始化
static int __init si_domain_init(int hw)
{
	struct dmar_drhd_unit *drhd;
	struct intel_iommu *iommu;
	int nid, ret = 0;
	bool first = true;

	si_domain = alloc_domain(DOMAIN_FLAG_STATIC_IDENTITY);
	if (!si_domain)
		return -EFAULT;

	for_each_active_iommu(iommu, drhd) {
		ret = iommu_attach_domain(si_domain, iommu);
		if (ret < 0) {
			domain_exit(si_domain);
			return -EFAULT;
		} else if (first) {
			//static indentity domain的id是第一个iommu中
			//在位图中位置的下标
			si_domain->id = ret;
			first = false;
		} else if (si_domain->id != ret) {
			domain_exit(si_domain);
			return -EFAULT;
		}
		domain_attach_iommu(si_domain, iommu);
	}

	if (md_domain_init(si_domain, DEFAULT_DOMAIN_ADDRESS_WIDTH)) {
		domain_exit(si_domain);
		return -EFAULT;
	}

	pr_debug("IOMMU: identity mapping domain is domain %d\n",
		 si_domain->id);

	if (hw)
		return 0;

	for_each_online_node(nid) {
		unsigned long start_pfn, end_pfn;
		int i;

		for_each_mem_pfn_range(i, nid, &start_pfn, &end_pfn, NULL) {
			ret = iommu_domain_identity_map(si_domain,
					PFN_PHYS(start_pfn), PFN_PHYS(end_pfn));
			if (ret)
				return ret;
		}
	}

	return 0;
}

static int identity_mapping(struct device *dev)
{
	struct device_domain_info *info;

	if (likely(!iommu_identity_mapping))
		return 0;

	info = dev->archdata.iommu;
	if (info && info != DUMMY_DEVICE_DOMAIN_INFO)
		return (info->domain == si_domain);

	return 0;
}
//将设备和domain关联
static int domain_add_dev_info(struct dmar_domain *domain,
			       struct device *dev, int translation)
{ 
	struct dmar_domain *ndomain;
	struct intel_iommu *iommu;
	u8 bus, devfn;
	int ret;

	//获得这个dev对应的iommu
	iommu = device_to_iommu(dev, &bus, &devfn);
	if (!iommu)
		return -ENODEV;

	ndomain = dmar_insert_dev_info(iommu, bus, devfn, dev, domain);
	if (ndomain != domain)
		return -EBUSY;

	//插入的必须是传入的domain，也就是必须是新申请的
	//device_domain_info

	//domain的pgd信息写入dev对应的iommu的二级页表中
	ret = domain_context_mapping(domain, dev, translation);
	if (ret) {
		//映射失败
		domain_remove_one_dev_info(domain, dev);
		return ret;
	}

	return 0;
}

//遍历rmrr链表，查找每个rmrr的dmar_dev_scope数组的每一项
//的device是否是传入的device
//如果有返回true
static bool device_has_rmrr(struct device *dev)
{
	struct dmar_rmrr_unit *rmrr;
	struct device *tmp;
	int i;

	rcu_read_lock();
	for_each_rmrr_units(rmrr) {
		/*
		 * Return TRUE if this RMRR contains the device that
		 * is passed in.
		 */
		 //遍历每一个dmar_dev_scope
		for_each_active_dev_scope(rmrr->devices,
					  rmrr->devices_cnt, i, tmp)
			if (tmp == dev) {
				rcu_read_unlock();
				return true;
			}
	}
	rcu_read_unlock();
	return false;
}

/*
 * There are a couple cases where we need to restrict the functionality of
 * devices associated with RMRRs.  The first is when evaluating a device for
 * identity mapping because problems exist when devices are moved in and out
 * of domains and their respective RMRR information is lost.  This means that
 * a device with associated RMRRs will never be in a "passthrough" domain.
 * The second is use of the device through the IOMMU API.  This interface
 * expects to have full control of the IOVA space for the device.  We cannot
 * satisfy both the requirement that RMRR access is maintained and have an
 * unencumbered IOVA space.  We also have no ability to quiesce the device's
 * use of the RMRR space or even inform the IOMMU API user of the restriction.
 * We therefore prevent devices associated with an RMRR from participating in
 * the IOMMU API, which eliminates them from device assignment.
 *
 * In both cases we assume that PCI USB devices with RMRRs have them largely
 * for historical reasons and that the RMRR space is not actively used post
 * boot.  This exclusion may change if vendors begin to abuse it.
 *
 * The same exception is made for graphics devices, with the requirement that
 * any use of the RMRR regions will be torn down before assigning the device
 * to a guest.
 */
static bool device_is_rmrr_locked(struct device *dev)
{
	if (!device_has_rmrr(dev))
		return false;//如果device没有rmrr就返回false

	//device有rmrr
	
	if (dev_is_pci(dev)) {
		//如果这个device是pci device
		struct pci_dev *pdev = to_pci_dev(dev);

		if (IS_USB_DEVICE(pdev) || IS_GFX_DEVICE(pdev))
			//device有rmrr，但是这个pci device是usb或者，gfx设备
			return false;
	}

	return true;
}

//判断iommu是否应该等值映射，identity map
static int iommu_should_identity_map(struct device *dev, int startup)
{

	if (dev_is_pci(dev)) {
		struct pci_dev *pdev = to_pci_dev(dev);

		if (device_is_rmrr_locked(dev))			
			return 0;
		
		//device有rmrr，并且这个pci device既不是usb也不是gfx设备
		//或者这个device没有rmrr

		//对有rmrr的做特殊处理

		if ((iommu_identity_mapping & IDENTMAP_AZALIA) && IS_AZALIA(pdev))
			//pci dev是azaliz并且iommu_identity_mapping是IDENTMAP_AZALIA
			return 1;

		if ((iommu_identity_mapping & IDENTMAP_GFX) && IS_GFX_DEVICE(pdev))
			//pci dev是gfx并且iommu_identity_mapping是IDENTMAP_GFX
			return 1;

		if (!(iommu_identity_mapping & IDENTMAP_ALL))
			//iommu_identity_mapping不是IDENTMAP_ALL
			return 0;

		/*
		 * We want to start off with all devices in the 1:1 domain, and
		 * take them out later if we find they can't access all of memory.
		 *
		 * However, we can't do this for PCI devices behind bridges,
		 * because all PCI devices behind the same bridge will end up
		 * with the same source-id on their transactions.
		 *
		 * Practically speaking, we can't change things around for these
		 * devices at run-time, because we can't be sure there'll be no
		 * DMA transactions in flight for any of their siblings.
		 *
		 * So PCI devices (unless they're on the root bus) as well as
		 * their parent PCI-PCI or PCIe-PCI bridges must be left _out_ of
		 * the 1:1 domain, just in _case_ one of their siblings turns out
		 * not to be able to map all of memory.
		 */
		if (!pci_is_pcie(pdev)) {
			//这个pci设备不是pcie设备
			if (!pci_is_root_bus(pdev->bus))
				return 0;
			if (pdev->class >> 8 == PCI_CLASS_BRIDGE_PCI)
				return 0;
		} else if (pci_pcie_type(pdev) == PCI_EXP_TYPE_PCI_BRIDGE)
			return 0;
	} else {
		if (device_has_rmrr(dev))
			return 0;
	}

	/*
	 * At boot time, we don't yet know if devices will be 64-bit capable.
	 * Assume that they will 鈥� if they turn out not to be, then we can
	 * take them out of the 1:1 domain later.
	 */
	if (!startup) {
		/*
		 * If the device's dma_mask is less than the system's memory
		 * size then this is not a candidate for identity mapping.
		 */
		u64 dma_mask = *dev->dma_mask;

		if (dev->coherent_dma_mask &&
		    dev->coherent_dma_mask < dma_mask)
			dma_mask = dev->coherent_dma_mask;

		return dma_mask >= dma_get_required_mask(dev);
	}

	return 1;
}

//将pci device加入到等值映射domain中去
//将static domain的pgd信息写入dev对应的iommu的二级页表中
static int __init dev_prepare_static_identity_mapping(struct device *dev, int hw)
{
	int ret;

	if (!iommu_should_identity_map(dev, 1))
		//如果iommu不应该identity map，直接返回
		return 0;

	//iommu应该identity map

	ret = domain_add_dev_info(si_domain, dev,
				  hw ? CONTEXT_TT_PASS_THROUGH :
				       CONTEXT_TT_MULTI_LEVEL);
	if (!ret)
		pr_info("IOMMU: %s identity mapping for device %s\n",
			hw ? "hardware" : "software", dev_name(dev));
	else if (ret == -ENODEV)
		/* device not associated with an iommu */
		ret = 0;

	return ret;
}

//为设备建立iommu等值映射(物理地址=虚拟地址)
static int __init iommu_prepare_static_identity_mapping(int hw)
{
	struct pci_dev *pdev = NULL;
	struct dmar_drhd_unit *drhd;
	struct intel_iommu *iommu;
	struct device *dev;
	int i;
	int ret = 0;

	//为iommu添加一个domain,该domain为等值映射domain
	ret = si_domain_init(hw);
	if (ret)
		return -EFAULT;

	//将pci device加入到等值映射domain中去
	for_each_pci_dev(pdev) {
		//static domain的pgd信息写入到每一个pci设备对应的iommu的二级页表中
		ret = dev_prepare_static_identity_mapping(&pdev->dev, hw);
		if (ret)
			return ret;
	}

	//对acpi中的设备加入到等值映射domain中去
	for_each_active_iommu(iommu, drhd)
		for_each_active_dev_scope(drhd->devices, drhd->devices_cnt, i, dev) {
			struct acpi_device_physical_node *pn;
			struct acpi_device *adev;
	
			if (dev->bus != &acpi_bus_type)
				continue;
				
			adev= to_acpi_device(dev);
			mutex_lock(&adev->physical_node_lock);
			list_for_each_entry(pn, &adev->physical_node_list, node) {
				ret = dev_prepare_static_identity_mapping(pn->dev, hw);
				if (ret)
					break;
			}
			mutex_unlock(&adev->physical_node_lock);
			if (ret)
				return ret;
		}

	return 0;
}

//queued invalidation用于iotlb, 可以一次提交多个invalidation请求
//qi的初始化操作，准备qi数据，分配qi的内存，使能qi，
//设置iommu的qi信息
static void intel_iommu_init_qi(struct intel_iommu *iommu)
{
	/*
	 * Start from the sane iommu hardware state.
	 * If the queued invalidation is already initialized by us
	 * (for example, while enabling interrupt-remapping) then
	 * we got the things already rolling from a sane state.
	 */
	 
	if (!iommu->qi) {
		//会执行
		/*
		 * Clear any previous faults.
		 */

		//清除错误状态
		dmar_fault(-1, iommu);
		/*
		 * Disable queued invalidation if supported and already enabled
		 * before OS handover.
		 */
		 
		 //禁止queued invalidation 
		dmar_disable_qi(iommu);
	}

	//设置iommu的qi，并使能qi，初始化操作都在这个函数里面
	if (dmar_enable_qi(iommu)) {
		/*
		 * Queued Invalidate not enabled, use Register Based Invalidate
		 */
		iommu->flush.flush_context = __iommu_flush_context;
		iommu->flush.flush_iotlb = __iommu_flush_iotlb;
		pr_info("IOMMU: %s using Register based invalidation\n",
			iommu->name);
	} else {
		//使能了iommu的qi
		iommu->flush.flush_context = qi_flush_context;
		iommu->flush.flush_iotlb = qi_flush_iotlb;
		pr_info("IOMMU: %s using Queued invalidation\n", iommu->name);
	}
}

static int __init init_dmars(void)
{
	struct dmar_drhd_unit *drhd;
	struct dmar_rmrr_unit *rmrr;
	struct device *dev;
	struct intel_iommu *iommu;
	int i, ret;

	/*
	 * for each drhd
	 *    allocate root
	 *    initialize and program root entry to not present
	 * endfor
	 */
	for_each_drhd_unit(drhd) {
		/*
		 * lock not needed as this is only incremented in the single
		 * threaded kernel __init code path all other access are read
		 * only
		 */
		 //统计多少个iommu
		if (g_num_of_iommus < DMAR_UNITS_SUPPORTED) {
			g_num_of_iommus++;
			continue;
		}
		printk_once(KERN_ERR "intel-iommu: exceeded %d IOMMUs\n",
			  DMAR_UNITS_SUPPORTED);
	}
	//申请足够多的 global iommu array
	/* Preallocate enough resources for IOMMU hot-addition */
	if (g_num_of_iommus < DMAR_UNITS_SUPPORTED)
		g_num_of_iommus = DMAR_UNITS_SUPPORTED;

	g_iommus = kcalloc(g_num_of_iommus, sizeof(struct intel_iommu *),
			GFP_KERNEL);
	if (!g_iommus) {
		printk(KERN_ERR "Allocating global iommu array failed\n");
		ret = -ENOMEM;
		goto error;
	}
	//申请和intel_iommu同样多的deferred_flush_tables
	deferred_flush = kzalloc(g_num_of_iommus *
		sizeof(struct deferred_flush_tables), GFP_KERNEL);
	if (!deferred_flush) {
		ret = -ENOMEM;
		goto free_g_iommus;
	}

	for_each_active_iommu(iommu, drhd) {
		g_iommus[iommu->seq_id] = iommu;//iommu表示本次获取到的intel_iommu。

		//为intel_iommu申请domain
		ret = iommu_init_domains(iommu);
		if (ret)
			goto free_iommu;

		/*
		 * TBD:
		 * we could share the same root & context tables
		 * among all IOMMU's. Need to Split it later.
		 */
		 //设置iommu的root_entry成员。一个内存页
		ret = iommu_alloc_root_entry(iommu);
		if (ret)
			goto free_iommu;

		//Extended Capability Register 。[6]:Pass Through
		if (!ecap_pass_through(iommu->ecap))
			//如果[6]为0。不支持pass throught技术
			hw_pass_through = 0;
	}

	for_each_active_iommu(iommu, drhd)
		intel_iommu_init_qi(iommu);//iommu中qi的初始化

	if (iommu_pass_through)
		iommu_identity_mapping |= IDENTMAP_ALL;

#ifdef CONFIG_INTEL_IOMMU_BROKEN_GFX_WA
	iommu_identity_mapping |= IDENTMAP_GFX;
#endif

	check_tylersburg_isoch();//检查芯片组的isoch，tlb是否为空

	/*
	 * If pass through is not set or not enabled, setup context entries for
	 * identity mappings for rmrr, gfx, and isa and may fall back to static
	 * identity mapping if iommu_identity_mapping is set.
	 */
	 //如果pass through 没有设置或者没有使能。
	if (iommu_identity_mapping) {
		//设置iommu的pass through 
		//identity mapping
		ret = iommu_prepare_static_identity_mapping(hw_pass_through);
		if (ret) {
			printk(KERN_CRIT "Failed to setup IOMMU pass-through\n");
			goto free_iommu;
		}
	}
	/*
	 * For each rmrr
	 *   for each dev attached to rmrr
	 *   do
	 *     locate drhd for dev, alloc domain for dev 为设备查找drhd，为设备申请domain
	 *     allocate free domain
	 *     allocate page table entries for rmrr
	 *     if context not allocated for bus
	 *           allocate and init context
	 *           set present in root table for this bus
	 *     init context with domain, translation etc
	 *    endfor
	 * endfor
	 */
	printk(KERN_INFO "IOMMU: Setting RMRR:\n");
	for_each_rmrr_units(rmrr) {
		/* some BIOS lists non-exist devices in DMAR table. */
		//在dmar table中，一些bios 列表没有device
		for_each_active_dev_scope(rmrr->devices, rmrr->devices_cnt,
					  i, dev) {
			ret = iommu_prepare_rmrr_dev(rmrr, dev);
			if (ret)
				printk(KERN_ERR
				       "IOMMU: mapping reserved region failed\n");
		}
	}

	iommu_prepare_isa();

	/*
	 * for each drhd
	 *   enable fault log
	 *   global invalidate context cache
	 *   global invalidate iotlb
	 *   enable translation
	 */
	for_each_iommu(iommu, drhd) {
		if (drhd->ignored) {
			/*
			 * we always have to disable PMRs or DMA may fail on
			 * this device
			 */
			if (force_on)
				iommu_disable_protect_mem_regions(iommu);
			continue;
		}

		iommu_flush_write_buffer(iommu);

		//为iommu设置申请中断
		ret = dmar_set_interrupt(iommu);
		if (ret)
			goto free_iommu;

		iommu_set_root_entry(iommu);

		iommu->flush.flush_context(iommu, 0, 0, 0, DMA_CCMD_GLOBAL_INVL);
		iommu->flush.flush_iotlb(iommu, 0, 0, 0, DMA_TLB_GLOBAL_FLUSH);
		iommu_enable_translation(iommu);
		iommu_disable_protect_mem_regions(iommu);
	}

	return 0;

free_iommu:
	for_each_active_iommu(iommu, drhd) {
		disable_dmar_iommu(iommu);
		free_dmar_iommu(iommu);
	}
	kfree(deferred_flush);
free_g_iommus:
	kfree(g_iommus);
error:
	return ret;
}

/* This takes a number of _MM_ pages, not VTD pages */
static struct iova *intel_alloc_iova(struct device *dev,
				     struct dmar_domain *domain,
				     unsigned long nrpages, uint64_t dma_mask)
{
	struct iova *iova = NULL;

	/* Restrict dma_mask to the width that the iommu can handle */
	dma_mask = min_t(uint64_t, DOMAIN_MAX_ADDR(domain->gaw), dma_mask);

	if (!dmar_forcedac && dma_mask > DMA_BIT_MASK(32)) {
		/*
		 * First try to allocate an io virtual address in
		 * DMA_BIT_MASK(32) and if that fails then try allocating
		 * from higher range
		 */
		iova = alloc_iova(&domain->iovad, nrpages,
				  IOVA_PFN(DMA_BIT_MASK(32)), 1);
		if (iova)
			return iova;
	}
	iova = alloc_iova(&domain->iovad, nrpages, IOVA_PFN(dma_mask), 1);
	if (unlikely(!iova)) {
		printk(KERN_ERR "Allocating %ld-page iova for %s failed",
		       nrpages, dev_name(dev));
		return NULL;
	}

	return iova;
}

static struct dmar_domain *__get_valid_domain_for_dev(struct device *dev)
{
	struct dmar_domain *domain;
	int ret;

	domain = get_domain_for_dev(dev, DEFAULT_DOMAIN_ADDRESS_WIDTH);
	if (!domain) {
		printk(KERN_ERR "Allocating domain for %s failed",
		       dev_name(dev));
		return NULL;
	}

	/* make sure context mapping is ok */
	if (unlikely(!domain_context_mapped(dev))) {
		ret = domain_context_mapping(domain, dev, CONTEXT_TT_MULTI_LEVEL);
		if (ret) {
			printk(KERN_ERR "Domain context map for %s failed",
			       dev_name(dev));
			return NULL;
		}
	}

	return domain;
}

static inline struct dmar_domain *get_valid_domain_for_dev(struct device *dev)
{
	struct device_domain_info *info;

	/* No lock here, assumes no domain exit in normal case */
	info = dev->archdata.iommu;
	if (likely(info))
		return info->domain;

	return __get_valid_domain_for_dev(dev);
}

/* Check if the dev needs to go through non-identity map and unmap process.*/
static int iommu_no_mapping(struct device *dev)
{
	int found;

	if (iommu_dummy(dev))
		return 1;

	if (!iommu_identity_mapping)
		return 0;

	found = identity_mapping(dev);
	if (found) {
		if (iommu_should_identity_map(dev, 0))
			return 1;
		else {
			/*
			 * 32 bit DMA is removed from si_domain and fall back
			 * to non-identity mapping.
			 */
			domain_remove_one_dev_info(si_domain, dev);
			printk(KERN_INFO "32bit %s uses non-identity mapping\n",
			       dev_name(dev));
			return 0;
		}
	} else {
		/*
		 * In case of a detached 64 bit DMA device from vm, the device
		 * is put into si_domain for identity mapping.
		 */
		if (iommu_should_identity_map(dev, 0)) {
			int ret;
			ret = domain_add_dev_info(si_domain, dev,
						  hw_pass_through ?
						  CONTEXT_TT_PASS_THROUGH :
						  CONTEXT_TT_MULTI_LEVEL);
			if (!ret) {
				printk(KERN_INFO "64bit %s uses identity mapping\n",
				       dev_name(dev));
				return 1;
			}
		}
	}

	return 0;
}

static dma_addr_t __intel_map_single(struct device *dev, phys_addr_t paddr,
				     size_t size, int dir, u64 dma_mask)
{
	struct dmar_domain *domain;
	phys_addr_t start_paddr;
	struct iova *iova;
	int prot = 0;
	int ret;
	struct intel_iommu *iommu;
	unsigned long paddr_pfn = paddr >> PAGE_SHIFT;

	BUG_ON(dir == DMA_NONE);

	if (iommu_no_mapping(dev))
		return paddr;

	domain = get_valid_domain_for_dev(dev);
	if (!domain)
		return 0;

	iommu = domain_get_iommu(domain);
	size = aligned_nrpages(paddr, size);

	iova = intel_alloc_iova(dev, domain, dma_to_mm_pfn(size), dma_mask);
	if (!iova)
		goto error;

	/*
	 * Check if DMAR supports zero-length reads on write only
	 * mappings..
	 */
	if (dir == DMA_TO_DEVICE || dir == DMA_BIDIRECTIONAL || \
			!cap_zlr(iommu->cap))
		prot |= DMA_PTE_READ;
	if (dir == DMA_FROM_DEVICE || dir == DMA_BIDIRECTIONAL)
		prot |= DMA_PTE_WRITE;
	/*
	 * paddr - (paddr + size) might be partial page, we should map the whole
	 * page.  Note: if two part of one page are separately mapped, we
	 * might have two guest_addr mapping to the same host paddr, but this
	 * is not a big problem
	 */
	ret = domain_pfn_mapping(domain, mm_to_dma_pfn(iova->pfn_lo),
				 mm_to_dma_pfn(paddr_pfn), size, prot);
	if (ret)
		goto error;

	/* it's a non-present to present mapping. Only flush if caching mode */
	if (cap_caching_mode(iommu->cap))
		iommu_flush_iotlb_psi(iommu, domain->id, mm_to_dma_pfn(iova->pfn_lo), size, 0, 1);
	else
		iommu_flush_write_buffer(iommu);

	start_paddr = (phys_addr_t)iova->pfn_lo << PAGE_SHIFT;
	start_paddr += paddr & ~PAGE_MASK;
	return start_paddr;

error:
	if (iova)
		__free_iova(&domain->iovad, iova);
	printk(KERN_ERR"Device %s request: %zx@%llx dir %d --- failed\n",
		dev_name(dev), size, (unsigned long long)paddr, dir);
	return 0;
}

static dma_addr_t intel_map_page(struct device *dev, struct page *page,
				 unsigned long offset, size_t size,
				 enum dma_data_direction dir,
				 struct dma_attrs *attrs)
{
	return __intel_map_single(dev, page_to_phys(page) + offset, size,
				  dir, *dev->dma_mask);
}

static void flush_unmaps(void)
{
	int i, j;

	timer_on = 0;

	/* just flush them all */
	for (i = 0; i < g_num_of_iommus; i++) {
		struct intel_iommu *iommu = g_iommus[i];//取出系统中每一个iommu
		if (!iommu)
			continue;

		if (!deferred_flush[i].next)//下一个没有了
			continue;

		/* In caching mode, global flushes turn emulation expensive */
		if (!cap_caching_mode(iommu->cap))
			//如果caching mode被置为了0
			iommu->flush.flush_iotlb(iommu, 0, 0, 0,
					 DMA_TLB_GLOBAL_FLUSH);
		for (j = 0; j < deferred_flush[i].next; j++) {
			unsigned long mask;
			struct iova *iova = deferred_flush[i].iova[j];
			struct dmar_domain *domain = deferred_flush[i].domain[j];

			/* On real hardware multiple invalidations are expensive */
			if (cap_caching_mode(iommu->cap))
				//如果caching mode被置为了1
				iommu_flush_iotlb_psi(
					iommu, 
					domain->id,
					iova->pfn_lo,//尾地址 
					iova_size(iova),//iova的大小，
					!deferred_flush[i].freelist[j],
					0);
			else {
				//如果caching mode被置为了0
				mask = ilog2(mm_to_dma_pfn(iova_size(iova)));
				iommu_flush_dev_iotlb(deferred_flush[i].domain[j],
						(uint64_t)iova->pfn_lo << PAGE_SHIFT, mask);
			}
			__free_iova(&deferred_flush[i].domain[j]->iovad, iova);
			if (deferred_flush[i].freelist[j])
				dma_free_pagelist(deferred_flush[i].freelist[j]);
		}
		deferred_flush[i].next = 0;
	}

	list_size = 0;
}

static void flush_unmaps_timeout(unsigned long data)
{
	unsigned long flags;

	spin_lock_irqsave(&async_umap_flush_lock, flags);
	flush_unmaps();
	spin_unlock_irqrestore(&async_umap_flush_lock, flags);
}

static void add_unmap(struct dmar_domain *dom, struct iova *iova, struct page *freelist)
{
	unsigned long flags;
	int next, iommu_id;
	struct intel_iommu *iommu;

	spin_lock_irqsave(&async_umap_flush_lock, flags);
	if (list_size == HIGH_WATER_MARK)
		flush_unmaps();

	iommu = domain_get_iommu(dom);
	iommu_id = iommu->seq_id;

	next = deferred_flush[iommu_id].next;
	deferred_flush[iommu_id].domain[next] = dom;
	deferred_flush[iommu_id].iova[next] = iova;
	deferred_flush[iommu_id].freelist[next] = freelist;
	deferred_flush[iommu_id].next++;

	if (!timer_on) {
		mod_timer(&unmap_timer, jiffies + msecs_to_jiffies(10));
		timer_on = 1;
	}
	list_size++;
	spin_unlock_irqrestore(&async_umap_flush_lock, flags);
}

static void intel_unmap(struct device *dev, dma_addr_t dev_addr)
{
	struct dmar_domain *domain;
	unsigned long start_pfn, last_pfn;
	struct iova *iova;
	struct intel_iommu *iommu;
	struct page *freelist;

	if (iommu_no_mapping(dev))
		return;

	domain = find_domain(dev);
	BUG_ON(!domain);

	iommu = domain_get_iommu(domain);

	iova = find_iova(&domain->iovad, IOVA_PFN(dev_addr));
	if (WARN_ONCE(!iova, "Driver unmaps unmatched page at PFN %llx\n",
		      (unsigned long long)dev_addr))
		return;

	start_pfn = mm_to_dma_pfn(iova->pfn_lo);
	last_pfn = mm_to_dma_pfn(iova->pfn_hi + 1) - 1;

	pr_debug("Device %s unmapping: pfn %lx-%lx\n",
		 dev_name(dev), start_pfn, last_pfn);

	freelist = domain_unmap(domain, start_pfn, last_pfn);

	if (intel_iommu_strict) {
		iommu_flush_iotlb_psi(iommu, domain->id, start_pfn,
				      last_pfn - start_pfn + 1, !freelist, 0);
		/* free iova */
		__free_iova(&domain->iovad, iova);
		dma_free_pagelist(freelist);
	} else {
		add_unmap(domain, iova, freelist);
		/*
		 * queue up the release of the unmap to save the 1/6th of the
		 * cpu used up by the iotlb flush operation...
		 */
	}
}

static void intel_unmap_page(struct device *dev, dma_addr_t dev_addr,
			     size_t size, enum dma_data_direction dir,
			     struct dma_attrs *attrs)
{
	intel_unmap(dev, dev_addr);
}

static void *intel_alloc_coherent(struct device *dev, size_t size,
				  dma_addr_t *dma_handle, gfp_t flags,
				  struct dma_attrs *attrs)
{
	struct page *page = NULL;
	int order;

	size = PAGE_ALIGN(size);
	order = get_order(size);

	if (!iommu_no_mapping(dev))
		flags &= ~(GFP_DMA | GFP_DMA32);
	else if (dev->coherent_dma_mask < dma_get_required_mask(dev)) {
		if (dev->coherent_dma_mask < DMA_BIT_MASK(32))
			flags |= GFP_DMA;
		else
			flags |= GFP_DMA32;
	}

	if (flags & __GFP_WAIT) {
		unsigned int count = size >> PAGE_SHIFT;

		page = dma_alloc_from_contiguous(dev, count, order);
		if (page && iommu_no_mapping(dev) &&
		    page_to_phys(page) + size > dev->coherent_dma_mask) {
			dma_release_from_contiguous(dev, page, count);
			page = NULL;
		}
	}

	if (!page)
		page = alloc_pages(flags, order);
	if (!page)
		return NULL;
	memset(page_address(page), 0, size);

	*dma_handle = __intel_map_single(dev, page_to_phys(page), size,
					 DMA_BIDIRECTIONAL,
					 dev->coherent_dma_mask);
	if (*dma_handle)
		return page_address(page);
	if (!dma_release_from_contiguous(dev, page, size >> PAGE_SHIFT))
		__free_pages(page, order);

	return NULL;
}

static void intel_free_coherent(struct device *dev, size_t size, void *vaddr,
				dma_addr_t dma_handle, struct dma_attrs *attrs)
{
	int order;
	struct page *page = virt_to_page(vaddr);

	size = PAGE_ALIGN(size);
	order = get_order(size);

	intel_unmap(dev, dma_handle);
	if (!dma_release_from_contiguous(dev, page, size >> PAGE_SHIFT))
		__free_pages(page, order);
}

static void intel_unmap_sg(struct device *dev, struct scatterlist *sglist,
			   int nelems, enum dma_data_direction dir,
			   struct dma_attrs *attrs)
{
	intel_unmap(dev, sglist[0].dma_address);
}

static int intel_nontranslate_map_sg(struct device *hddev,
	struct scatterlist *sglist, int nelems, int dir)
{
	int i;
	struct scatterlist *sg;

	for_each_sg(sglist, sg, nelems, i) {
		BUG_ON(!sg_page(sg));
		sg->dma_address = page_to_phys(sg_page(sg)) + sg->offset;
		sg->dma_length = sg->length;
	}
	return nelems;
}

static int intel_map_sg(struct device *dev, struct scatterlist *sglist, int nelems,
			enum dma_data_direction dir, struct dma_attrs *attrs)
{
	int i;
	struct dmar_domain *domain;
	size_t size = 0;
	int prot = 0;
	struct iova *iova = NULL;
	int ret;
	struct scatterlist *sg;
	unsigned long start_vpfn;
	struct intel_iommu *iommu;

	BUG_ON(dir == DMA_NONE);
	if (iommu_no_mapping(dev))
		return intel_nontranslate_map_sg(dev, sglist, nelems, dir);

	domain = get_valid_domain_for_dev(dev);
	if (!domain)
		return 0;

	iommu = domain_get_iommu(domain);

	for_each_sg(sglist, sg, nelems, i)
		size += aligned_nrpages(sg->offset, sg->length);

	iova = intel_alloc_iova(dev, domain, dma_to_mm_pfn(size),
				*dev->dma_mask);
	if (!iova) {
		sglist->dma_length = 0;
		return 0;
	}

	/*
	 * Check if DMAR supports zero-length reads on write only
	 * mappings..
	 */
	if (dir == DMA_TO_DEVICE || dir == DMA_BIDIRECTIONAL || \
			!cap_zlr(iommu->cap))
		prot |= DMA_PTE_READ;
	if (dir == DMA_FROM_DEVICE || dir == DMA_BIDIRECTIONAL)
		prot |= DMA_PTE_WRITE;

	start_vpfn = mm_to_dma_pfn(iova->pfn_lo);

	ret = domain_sg_mapping(domain, start_vpfn, sglist, size, prot);
	if (unlikely(ret)) {
		dma_pte_free_pagetable(domain, start_vpfn,
				       start_vpfn + size - 1);
		__free_iova(&domain->iovad, iova);
		return 0;
	}

	/* it's a non-present to present mapping. Only flush if caching mode */
	if (cap_caching_mode(iommu->cap))
		iommu_flush_iotlb_psi(iommu, domain->id, start_vpfn, size, 0, 1);
	else
		iommu_flush_write_buffer(iommu);

	return nelems;
}

static int intel_mapping_error(struct device *dev, dma_addr_t dma_addr)
{
	return !dma_addr;
}

struct dma_map_ops intel_dma_ops = {
	.alloc = intel_alloc_coherent,
	.free = intel_free_coherent,
	.map_sg = intel_map_sg,
	.unmap_sg = intel_unmap_sg,
	.map_page = intel_map_page,
	.unmap_page = intel_unmap_page,
	.mapping_error = intel_mapping_error,
};

static inline int iommu_domain_cache_init(void)
{
	int ret = 0;

	iommu_domain_cache = kmem_cache_create("iommu_domain",
					 sizeof(struct dmar_domain),
					 0,
					 SLAB_HWCACHE_ALIGN,

					 NULL);
	if (!iommu_domain_cache) {
		printk(KERN_ERR "Couldn't create iommu_domain cache\n");
		ret = -ENOMEM;
	}

	return ret;
}

static inline int iommu_devinfo_cache_init(void)
{
	int ret = 0;

	iommu_devinfo_cache = kmem_cache_create("iommu_devinfo",
					 sizeof(struct device_domain_info),
					 0,
					 SLAB_HWCACHE_ALIGN,
					 NULL);
	if (!iommu_devinfo_cache) {
		printk(KERN_ERR "Couldn't create devinfo cache\n");
		ret = -ENOMEM;
	}

	return ret;
}
//分配ova, domain,dev_info三种cache
static int __init iommu_init_mempool(void)
{
	int ret;
	ret = iommu_iova_cache_init();//iommu_iova_cache  ，都是使用kmem_cache_create()
	if (ret)
		return ret;

	ret = iommu_domain_cache_init();//dmar_domain cache 也就是iommu_domain_cache
	if (ret)
		goto domain_error;

	ret = iommu_devinfo_cache_init();//iommu_devinfo_cache
	if (!ret)
		return ret; //分配成功在这里返回

	kmem_cache_destroy(iommu_domain_cache);
domain_error:
	iommu_iova_cache_destroy();

	return -ENOMEM;
}

static void __init iommu_exit_mempool(void)
{
	kmem_cache_destroy(iommu_devinfo_cache);
	kmem_cache_destroy(iommu_domain_cache);
	iommu_iova_cache_destroy();
}

static void quirk_ioat_snb_local_iommu(struct pci_dev *pdev)
{
	struct dmar_drhd_unit *drhd;
	u32 vtbar;
	int rc;

	/* We know that this device on this chipset has its own IOMMU.
	 * If we find it under a different IOMMU, then the BIOS is lying
	 * to us. Hope that the IOMMU for this device is actually
	 * disabled, and it needs no translation...
	 */
	rc = pci_bus_read_config_dword(pdev->bus, PCI_DEVFN(0, 0), 0xb0, &vtbar);
	if (rc) {
		/* "can't" happen */
		dev_info(&pdev->dev, "failed to run vt-d quirk\n");
		return;
	}
	vtbar &= 0xffff0000;

	/* we know that the this iommu should be at offset 0xa000 from vtbar */
	drhd = dmar_find_matched_drhd_unit(pdev);
	if (WARN_TAINT_ONCE(!drhd || drhd->reg_base_addr - vtbar != 0xa000,
			    TAINT_FIRMWARE_WORKAROUND,
			    "BIOS assigned incorrect VT-d unit for Intel(R) QuickData Technology device\n"))
		pdev->dev.archdata.iommu = DUMMY_DEVICE_DOMAIN_INFO;
}
DECLARE_PCI_FIXUP_ENABLE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IOAT_SNB, quirk_ioat_snb_local_iommu);

static void __init init_no_remapping_devices(void)
{
	struct dmar_drhd_unit *drhd;
	struct device *dev;
	int i;

	for_each_drhd_unit(drhd) {
		if (!drhd->include_all) {
			for_each_active_dev_scope(drhd->devices,
						  drhd->devices_cnt, i, dev)
				break;
			/* ignore DMAR unit if no devices exist */
			if (i == drhd->devices_cnt)
				drhd->ignored = 1;
		}
	}

	for_each_active_drhd_unit(drhd) {
		if (drhd->include_all)
			continue;

		for_each_active_dev_scope(drhd->devices,
					  drhd->devices_cnt, i, dev)
			if (!dev_is_pci(dev) || !IS_GFX_DEVICE(to_pci_dev(dev)))
				break;
		if (i < drhd->devices_cnt)
			continue;

		/* This IOMMU has *only* gfx devices. Either bypass it or
		   set the gfx_mapped flag, as appropriate */
		if (dmar_map_gfx) {
			intel_iommu_gfx_mapped = 1;
		} else {
			drhd->ignored = 1;
			for_each_active_dev_scope(drhd->devices,
						  drhd->devices_cnt, i, dev)
						  //pci_dev不用iommu
				dev->archdata.iommu = DUMMY_DEVICE_DOMAIN_INFO;
		}
	}
}

#ifdef CONFIG_SUSPEND
static int init_iommu_hw(void)
{
	struct dmar_drhd_unit *drhd;
	struct intel_iommu *iommu = NULL;

	for_each_active_iommu(iommu, drhd)
		if (iommu->qi)
			dmar_reenable_qi(iommu);

	for_each_iommu(iommu, drhd) {
		if (drhd->ignored) {
			/*
			 * we always have to disable PMRs or DMA may fail on
			 * this device
			 */
			if (force_on)
				iommu_disable_protect_mem_regions(iommu);
			continue;
		}
	
		iommu_flush_write_buffer(iommu);

		iommu_set_root_entry(iommu);

		iommu->flush.flush_context(iommu, 0, 0, 0,
					   DMA_CCMD_GLOBAL_INVL);
		iommu->flush.flush_iotlb(iommu, 0, 0, 0, DMA_TLB_GLOBAL_FLUSH);
		iommu_enable_translation(iommu);
		iommu_disable_protect_mem_regions(iommu);
	}

	return 0;
}

static void iommu_flush_all(void)
{
	struct dmar_drhd_unit *drhd;
	struct intel_iommu *iommu;

	for_each_active_iommu(iommu, drhd) {
		iommu->flush.flush_context(iommu, 0, 0, 0,
					   DMA_CCMD_GLOBAL_INVL);
		iommu->flush.flush_iotlb(iommu, 0, 0, 0,
					 DMA_TLB_GLOBAL_FLUSH);
	}
}

static int iommu_suspend(void)
{
	struct dmar_drhd_unit *drhd;
	struct intel_iommu *iommu = NULL;
	unsigned long flag;

	for_each_active_iommu(iommu, drhd) {
		iommu->iommu_state = kzalloc(sizeof(u32) * MAX_SR_DMAR_REGS,
						 GFP_ATOMIC);
		if (!iommu->iommu_state)
			goto nomem;
	}

	iommu_flush_all();

	for_each_active_iommu(iommu, drhd) {
		iommu_disable_translation(iommu);

		raw_spin_lock_irqsave(&iommu->register_lock, flag);

		iommu->iommu_state[SR_DMAR_FECTL_REG] =
			readl(iommu->reg + DMAR_FECTL_REG);
		iommu->iommu_state[SR_DMAR_FEDATA_REG] =
			readl(iommu->reg + DMAR_FEDATA_REG);
		iommu->iommu_state[SR_DMAR_FEADDR_REG] =
			readl(iommu->reg + DMAR_FEADDR_REG);
		iommu->iommu_state[SR_DMAR_FEUADDR_REG] =
			readl(iommu->reg + DMAR_FEUADDR_REG);

		raw_spin_unlock_irqrestore(&iommu->register_lock, flag);
	}
	return 0;

nomem:
	for_each_active_iommu(iommu, drhd)
		kfree(iommu->iommu_state);

	return -ENOMEM;
}

static void iommu_resume(void)
{
	struct dmar_drhd_unit *drhd;
	struct intel_iommu *iommu = NULL;
	unsigned long flag;

	if (init_iommu_hw()) {
		if (force_on)
			panic("tboot: IOMMU setup failed, DMAR can not resume!\n");
		else
			WARN(1, "IOMMU setup failed, DMAR can not resume!\n");
		return;
	}

	for_each_active_iommu(iommu, drhd) {

		raw_spin_lock_irqsave(&iommu->register_lock, flag);

		writel(iommu->iommu_state[SR_DMAR_FECTL_REG],
			iommu->reg + DMAR_FECTL_REG);
		writel(iommu->iommu_state[SR_DMAR_FEDATA_REG],
			iommu->reg + DMAR_FEDATA_REG);
		writel(iommu->iommu_state[SR_DMAR_FEADDR_REG],
			iommu->reg + DMAR_FEADDR_REG);
		writel(iommu->iommu_state[SR_DMAR_FEUADDR_REG],
			iommu->reg + DMAR_FEUADDR_REG);

		raw_spin_unlock_irqrestore(&iommu->register_lock, flag);
	}

	for_each_active_iommu(iommu, drhd)
		kfree(iommu->iommu_state);
}

static struct syscore_ops iommu_syscore_ops = {
	//在系统的suspend/resume 中被调用，
	.resume		= iommu_resume,
	.suspend	= iommu_suspend,
};

static void __init init_iommu_pm_ops(void)
{
	register_syscore_ops(&iommu_syscore_ops);
}

#else
static inline void init_iommu_pm_ops(void) {}
#endif	/* CONFIG_PM */


int __init dmar_parse_one_rmrr(struct acpi_dmar_header *header, void *arg)
{
	struct acpi_dmar_reserved_memory *rmrr;
	struct dmar_rmrr_unit *rmrru;

	rmrru = kzalloc(sizeof(*rmrru), GFP_KERNEL);
	if (!rmrru)
		return -ENOMEM;

	rmrru->hdr = header;
	rmrr = (struct acpi_dmar_reserved_memory *)header;
	rmrru->base_address = rmrr->base_address;
	rmrru->end_address = rmrr->end_address;
	rmrru->devices = dmar_alloc_dev_scope((void *)(rmrr + 1),
				((void *)rmrr) + rmrr->header.length,
				&rmrru->devices_cnt);
	if (rmrru->devices_cnt && rmrru->devices == NULL) {
		kfree(rmrru);
		return -ENOMEM;
	}

	list_add(&rmrru->list, &dmar_rmrr_units);

	return 0;
}

static struct dmar_atsr_unit *dmar_find_atsr(struct acpi_dmar_atsr *atsr)
{
	struct dmar_atsr_unit *atsru;
	struct acpi_dmar_atsr *tmp;

	list_for_each_entry_rcu(atsru, &dmar_atsr_units, list) {
		tmp = (struct acpi_dmar_atsr *)atsru->hdr;
		if (atsr->segment != tmp->segment)
			continue;
		if (atsr->header.length != tmp->header.length)
			continue;
		if (memcmp(atsr, tmp, atsr->header.length) == 0)
			return atsru;
	}

	return NULL;
}

int dmar_parse_one_atsr(struct acpi_dmar_header *hdr, void *arg)
{
	struct acpi_dmar_atsr *atsr;
	struct dmar_atsr_unit *atsru;

	if (system_state != SYSTEM_BOOTING && !intel_iommu_enabled)
		return 0;

	atsr = container_of(hdr, struct acpi_dmar_atsr, header);
	atsru = dmar_find_atsr(atsr);
	if (atsru)
		return 0;

	atsru = kzalloc(sizeof(*atsru) + hdr->length, GFP_KERNEL);
	if (!atsru)
		return -ENOMEM;

	/*
	 * If memory is allocated from slab by ACPI _DSM method, we need to
	 * copy the memory content because the memory buffer will be freed
	 * on return.
	 */
	atsru->hdr = (void *)(atsru + 1);
	memcpy(atsru->hdr, hdr, hdr->length);
	atsru->include_all = atsr->flags & 0x1;
	if (!atsru->include_all) {
		atsru->devices = dmar_alloc_dev_scope((void *)(atsr + 1),
				(void *)atsr + atsr->header.length,
				&atsru->devices_cnt);
		if (atsru->devices_cnt && atsru->devices == NULL) {
			kfree(atsru);
			return -ENOMEM;
		}
	}

	list_add_rcu(&atsru->list, &dmar_atsr_units);

	return 0;
}

static void intel_iommu_free_atsr(struct dmar_atsr_unit *atsru)
{
	dmar_free_dev_scope(&atsru->devices, &atsru->devices_cnt);
	kfree(atsru);
}

int dmar_release_one_atsr(struct acpi_dmar_header *hdr, void *arg)
{
	struct acpi_dmar_atsr *atsr;
	struct dmar_atsr_unit *atsru;

	atsr = container_of(hdr, struct acpi_dmar_atsr, header);
	atsru = dmar_find_atsr(atsr);
	if (atsru) {
		list_del_rcu(&atsru->list);
		synchronize_rcu();
		intel_iommu_free_atsr(atsru);
	}

	return 0;
}

int dmar_check_one_atsr(struct acpi_dmar_header *hdr, void *arg)
{
	int i;
	struct device *dev;
	struct acpi_dmar_atsr *atsr;
	struct dmar_atsr_unit *atsru;

	atsr = container_of(hdr, struct acpi_dmar_atsr, header);
	atsru = dmar_find_atsr(atsr);
	if (!atsru)
		return 0;

	if (!atsru->include_all && atsru->devices && atsru->devices_cnt)
		for_each_active_dev_scope(atsru->devices, atsru->devices_cnt,
					  i, dev)
			return -EBUSY;

	return 0;
}

static int intel_iommu_add(struct dmar_drhd_unit *dmaru)
{
	int sp, ret = 0;
	struct intel_iommu *iommu = dmaru->iommu;

	if (g_iommus[iommu->seq_id])
		return 0;

	if (hw_pass_through && !ecap_pass_through(iommu->ecap)) {
		pr_warn("IOMMU: %s doesn't support hardware pass through.\n",
			iommu->name);
		return -ENXIO;
	}
	if (!ecap_sc_support(iommu->ecap) &&
	    domain_update_iommu_snooping(iommu)) {
		pr_warn("IOMMU: %s doesn't support snooping.\n",
			iommu->name);
		return -ENXIO;
	}
	sp = domain_update_iommu_superpage(iommu) - 1;
	if (sp >= 0 && !(cap_super_page_val(iommu->cap) & (1 << sp))) {
		pr_warn("IOMMU: %s doesn't support large page.\n",
			iommu->name);
		return -ENXIO;
	}

	/*
	 * Disable translation if already enabled prior to OS handover.
	 */
	if (iommu->gcmd & DMA_GCMD_TE)
		iommu_disable_translation(iommu);

	g_iommus[iommu->seq_id] = iommu;
	ret = iommu_init_domains(iommu);
	if (ret == 0)
		ret = iommu_alloc_root_entry(iommu);
	if (ret)
		goto out;

	if (dmaru->ignored) {
		/*
		 * we always have to disable PMRs or DMA may fail on this device
		 */
		if (force_on)
			iommu_disable_protect_mem_regions(iommu);
		return 0;
	}

	intel_iommu_init_qi(iommu);
	iommu_flush_write_buffer(iommu);
	ret = dmar_set_interrupt(iommu);
	if (ret)
		goto disable_iommu;

	iommu_set_root_entry(iommu);
	iommu->flush.flush_context(iommu, 0, 0, 0, DMA_CCMD_GLOBAL_INVL);
	iommu->flush.flush_iotlb(iommu, 0, 0, 0, DMA_TLB_GLOBAL_FLUSH);
	iommu_enable_translation(iommu);

	if (si_domain) {
		ret = iommu_attach_domain(si_domain, iommu);
		if (ret < 0 || si_domain->id != ret)
			goto disable_iommu;
		domain_attach_iommu(si_domain, iommu);
	}

	iommu_disable_protect_mem_regions(iommu);
	return 0;

disable_iommu:
	disable_dmar_iommu(iommu);
out:
	free_dmar_iommu(iommu);
	return ret;
}

int dmar_iommu_hotplug(struct dmar_drhd_unit *dmaru, bool insert)
{
	int ret = 0;
	struct intel_iommu *iommu = dmaru->iommu;

	if (!intel_iommu_enabled)
		return 0;
	if (iommu == NULL)
		return -EINVAL;

	if (insert) {
		ret = intel_iommu_add(dmaru);
	} else {
		disable_dmar_iommu(iommu);
		free_dmar_iommu(iommu);
	}

	return ret;
}

static void intel_iommu_free_dmars(void)
{
	struct dmar_rmrr_unit *rmrru, *rmrr_n;
	struct dmar_atsr_unit *atsru, *atsr_n;

	list_for_each_entry_safe(rmrru, rmrr_n, &dmar_rmrr_units, list) {
		list_del(&rmrru->list);
		dmar_free_dev_scope(&rmrru->devices, &rmrru->devices_cnt);
		kfree(rmrru);
	}

	list_for_each_entry_safe(atsru, atsr_n, &dmar_atsr_units, list) {
		list_del(&atsru->list);
		intel_iommu_free_atsr(atsru);
	}
}
//通过dev的bus成员变量的parent找到根的pci_bus
//根pci bus的self(pci device类型)的dev成员的地址是
//dmar_atsr_unit链表中某一个成员的acpi_dmar_atsr
//的dmar_dev_scope数组中某一项的dev的地址
// 1 能找到，0 没有找到
int dmar_find_matched_atsr_unit(struct pci_dev *dev)
{
	int i, ret = 1;
	struct pci_bus *bus;
	struct pci_dev *bridge = NULL;
	struct device *tmp;
	struct acpi_dmar_atsr *atsr;
	struct dmar_atsr_unit *atsru;

	dev = pci_physfn(dev);

	//一直遍历父亲的pci bus
	//通过bus 中的self成员判断是否是root port
	for (bus = dev->bus; bus; bus = bus->parent) {
		bridge = bus->self;
		if (!bridge || !pci_is_pcie(bridge) ||pci_pcie_type(bridge) == PCI_EXP_TYPE_PCI_BRIDGE)
			return 0;
		if (pci_pcie_type(bridge) == PCI_EXP_TYPE_ROOT_PORT)
			break;
	}
	if (!bridge)
		return 0;

	//bus 是根，bridge是根的self
	
	rcu_read_lock();
	list_for_each_entry_rcu(atsru, &dmar_atsr_units, list) {
		atsr = container_of(atsru->hdr, struct acpi_dmar_atsr, header);
		if (atsr->segment != pci_domain_nr(dev->bus))
			continue;

		for_each_dev_scope(atsru->devices, atsru->devices_cnt, i, tmp)
			if (tmp == &bridge->dev)
				goto out;

		if (atsru->include_all)
			goto out;
	}
	ret = 0;
out:
	rcu_read_unlock();

	return ret;
}

//将dmar_pci_notify_info中pci device的一些信息填充到

//dmar_rmrr_unit ，dmar_atsr_unit中每一个成员对应的
//acpi_dmar_reserved_memory，acpi_dmar_atsr的
//dmar_dev_scope数组中的某一项
int dmar_iommu_notify_scope_dev(struct dmar_pci_notify_info *info)
{
	int ret = 0;
	struct dmar_rmrr_unit *rmrru;
	struct dmar_atsr_unit *atsru;
	struct acpi_dmar_atsr *atsr;
	struct acpi_dmar_reserved_memory *rmrr;

	if (!intel_iommu_enabled && system_state != SYSTEM_BOOTING)
		return 0;//iommu没有使能并且系统没有启动

	//从dmar_rmrr_unit链表中取出每一个dmar_rmrr_unit
	list_for_each_entry(rmrru, &dmar_rmrr_units, list) {
		rmrr = container_of(rmrru->hdr,
				    struct acpi_dmar_reserved_memory, header);
		if (info->event == BUS_NOTIFY_ADD_DEVICE) {
			//把dmar_pci_notify_info中pci device的一些信息填充
			//dmar_rmrr_unit种dmar_dev_scope数组中的某一项
			ret = dmar_insert_dev_scope(info,
				(void *)(rmrr + 1),
				((void *)rmrr) + rmrr->header.length,
				rmrr->segment,
				rmrru->devices,
				rmrru->devices_cnt);
			if(ret < 0)
				return ret;
		} else if (info->event == BUS_NOTIFY_DEL_DEVICE) {
			dmar_remove_dev_scope(info, rmrr->segment,
				rmrru->devices, rmrru->devices_cnt);
		}
	}

	list_for_each_entry(atsru, &dmar_atsr_units, list) {
		if (atsru->include_all)
			continue;

		atsr = container_of(atsru->hdr, struct acpi_dmar_atsr, header);
		if (info->event == BUS_NOTIFY_ADD_DEVICE) {
			ret = dmar_insert_dev_scope(info, (void *)(atsr + 1),
					(void *)atsr + atsr->header.length,
					atsr->segment, atsru->devices,
					atsru->devices_cnt);
			if (ret > 0)
				break;
			else if(ret < 0)
				return ret;
		} else if (info->event == BUS_NOTIFY_DEL_DEVICE) {
			if (dmar_remove_dev_scope(info, atsr->segment,
					atsru->devices, atsru->devices_cnt))
				break;
		}
	}

	return 0;
}

/*
 * Here we only respond to action of unbound device from driver.
 *
 * Added device is not attached to its DMAR domain here yet. That will happen
 * when mapping the device to iova.
 */
 //通知链的处理函数
static int device_notifier(struct notifier_block *nb,
				  unsigned long action, void *data)
{
	struct device *dev = data;
	struct dmar_domain *domain;

	if (iommu_dummy(dev))
		return 0;

	if (action != BUS_NOTIFY_REMOVED_DEVICE)
		return 0;

	domain = find_domain(dev);
	if (!domain)
		return 0;

	down_read(&dmar_global_lock);
	domain_remove_one_dev_info(domain, dev);
	if (!domain_type_is_vm_or_si(domain) && list_empty(&domain->devices))
		domain_exit(domain);
	up_read(&dmar_global_lock);

	return 0;
}

//通知链实例
static struct notifier_block device_nb = {
	.notifier_call = device_notifier,//处理函数
};

//内存通知链的处理函数
static int intel_iommu_memory_notifier(struct notifier_block *nb,
				       unsigned long val, void *v)
{
	struct memory_notify *mhp = v;
	unsigned long long start, end;
	unsigned long start_vpfn, last_vpfn;

	switch (val) {
	case MEM_GOING_ONLINE:
		start = mhp->start_pfn << PAGE_SHIFT;
		end = ((mhp->start_pfn + mhp->nr_pages) << PAGE_SHIFT) - 1;
		if (iommu_domain_identity_map(si_domain, start, end)) {
			pr_warn("dmar: failed to build identity map for [%llx-%llx]\n",
				start, end);
			return NOTIFY_BAD;
		}
		break;

	case MEM_OFFLINE:
	case MEM_CANCEL_ONLINE:
		start_vpfn = mm_to_dma_pfn(mhp->start_pfn);
		last_vpfn = mm_to_dma_pfn(mhp->start_pfn + mhp->nr_pages - 1);
		while (start_vpfn <= last_vpfn) {
			struct iova *iova;
			struct dmar_drhd_unit *drhd;
			struct intel_iommu *iommu;
			struct page *freelist;

			iova = find_iova(&si_domain->iovad, start_vpfn);
			if (iova == NULL) {
				pr_debug("dmar: failed get IOVA for PFN %lx\n",
					 start_vpfn);
				break;
			}

			iova = split_and_remove_iova(&si_domain->iovad, iova,
						     start_vpfn, last_vpfn);
			if (iova == NULL) {
				pr_warn("dmar: failed to split IOVA PFN [%lx-%lx]\n",
					start_vpfn, last_vpfn);
				return NOTIFY_BAD;
			}

			freelist = domain_unmap(si_domain, iova->pfn_lo,
					       iova->pfn_hi);

			rcu_read_lock();
			for_each_active_iommu(iommu, drhd)
				iommu_flush_iotlb_psi(iommu, si_domain->id,
					iova->pfn_lo, iova_size(iova),
					!freelist, 0);
			rcu_read_unlock();
			dma_free_pagelist(freelist);

			start_vpfn = iova->pfn_hi + 1;
			free_iova_mem(iova);
		}
		break;
	}

	return NOTIFY_OK;
}

static struct notifier_block intel_iommu_memory_nb = {
	.notifier_call = intel_iommu_memory_notifier,
	.priority = 0
};


static ssize_t intel_iommu_show_version(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct intel_iommu *iommu = dev_get_drvdata(dev);
	u32 ver = readl(iommu->reg + DMAR_VER_REG);
	return sprintf(buf, "%d:%d\n",
		       DMAR_VER_MAJOR(ver), DMAR_VER_MINOR(ver));
}
static DEVICE_ATTR(version, S_IRUGO, intel_iommu_show_version, NULL);

static ssize_t intel_iommu_show_address(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct intel_iommu *iommu = dev_get_drvdata(dev);
	return sprintf(buf, "%llx\n", iommu->reg_phys);
}
static DEVICE_ATTR(address, S_IRUGO, intel_iommu_show_address, NULL);

static ssize_t intel_iommu_show_cap(struct device *dev,
				    struct device_attribute *attr,
				    char *buf)
{
	struct intel_iommu *iommu = dev_get_drvdata(dev);
	return sprintf(buf, "%llx\n", iommu->cap);
}
static DEVICE_ATTR(cap, S_IRUGO, intel_iommu_show_cap, NULL);

static ssize_t intel_iommu_show_ecap(struct device *dev,
				    struct device_attribute *attr,
				    char *buf)
{
	struct intel_iommu *iommu = dev_get_drvdata(dev);
	return sprintf(buf, "%llx\n", iommu->ecap);
}
static DEVICE_ATTR(ecap, S_IRUGO, intel_iommu_show_ecap, NULL);

static struct attribute *intel_iommu_attrs[] = {
	&dev_attr_version.attr,
	&dev_attr_address.attr,
	&dev_attr_cap.attr,
	&dev_attr_ecap.attr,
	NULL,
};

static struct attribute_group intel_iommu_group = {
	.name = "intel-iommu",
	.attrs = intel_iommu_attrs,
};

//iommu的device的const struct attribute_group **groups;	/* optional groups */
const struct attribute_group *intel_iommu_groups[] = {
	&intel_iommu_group,
	NULL,
};

int __init intel_iommu_init(void)
{
	int ret = -ENODEV;
	struct dmar_drhd_unit *drhd;
	struct intel_iommu *iommu;

	/* VT-d is required for a TXT/tboot launch, so enforce that */
	force_on = tboot_force_iommu();

	//该模块要用到的内存池初始化iova, domain,dev_info三种cache
	if (iommu_init_mempool()) {
		if (force_on)
			panic("tboot: Failed to initialize iommu memory\n");
		return -ENOMEM;
	}

	down_write(&dmar_global_lock);
	if (dmar_table_init()) {	//parses the DMA reporting table
		if (force_on)//如果等于1 表示没有成功使能VT-d
			panic("tboot: Failed to initialize DMAR table\n");
		goto out_free_dmar;
	}

	/*
	 * Disable translation if already enabled prior to OS handover.
	 */
	 //如果在移交给操作系统之前已经使能转换了，现在禁止掉
	for_each_active_iommu(iommu, drhd)
		if (iommu->gcmd & DMA_GCMD_TE)//根据寄存器的值判断是否关闭了
			iommu_disable_translation(iommu);
//遍历系统中的pci_dev,
//建立哪些dmar unit 管理哪些pci_dev的映射
	if (dmar_dev_scope_init() < 0) {//里面涉及dmar_drhd_units
		if (force_on)
			panic("tboot: Failed to initialize DMAR device scope\n");
		goto out_free_dmar;
	}

	if (no_iommu || dmar_disabled)
		goto out_free_dmar;

	if (list_empty(&dmar_rmrr_units))
		printk(KERN_INFO "DMAR: No RMRR found\n");

	if (list_empty(&dmar_atsr_units))
		printk(KERN_INFO "DMAR: No ATSR found\n");

	//reserve iommu ranges
	if (dmar_init_reserved_ranges()) {
		if (force_on)
			panic("tboot: Failed to reserve iommu ranges\n");
		goto out_free_reserved_range;
	}
	
	//一开始设置所有pci_dev都不用iommu
	init_no_remapping_devices();
	
	//初始化dmar硬件并为每个dmar建立一个dmar_domain数组;
	//为pci_dev准备rmrr. rmrr区域的内存是vm host要保留出来使用的, 
	//因此在为device建立iommu时要特别考虑(不建立iommu)
	ret = init_dmars();
	if (ret) {
		if (force_on)
			panic("tboot: Failed to initialize DMARs\n");
		printk(KERN_ERR "IOMMU: dmar init failed\n");
		goto out_free_reserved_range;
	}
	up_write(&dmar_global_lock);
	printk(KERN_INFO
	"PCI-DMA: Intel(R) Virtualization Technology for Directed I/O\n");

	init_timer(&unmap_timer);
#ifdef CONFIG_SWIOTLB
	swiotlb = 0;//不运行
#endif
	dma_ops = &intel_dma_ops;

	//register_syscore_ops
	//暂停与继续相关的函数
	init_iommu_pm_ops();

	for_each_active_iommu(iommu, drhd)
		iommu->iommu_dev = iommu_device_create(NULL, iommu,
						       intel_iommu_groups,
						       iommu->name);

	//设置pci 的bus type 的成员变量
	bus_set_iommu(&pci_bus_type, &intel_iommu_ops);

	//bus->p->bus_notifie
	bus_register_notifier(&pci_bus_type, &device_nb);
	if (si_domain && !hw_pass_through)
		//如果有static identity domain并且不支持pass through
		register_memory_notifier(&intel_iommu_memory_nb);

	intel_iommu_enabled = 1;

	return 0;

out_free_reserved_range:
	put_iova_domain(&reserved_iova_list);
out_free_dmar:
	intel_iommu_free_dmars();
	up_write(&dmar_global_lock);
	iommu_exit_mempool();
	return ret;
}

static int iommu_detach_dev_cb(struct pci_dev *pdev, u16 alias, void *opaque)
{
	struct intel_iommu *iommu = opaque;

	iommu_detach_dev(iommu, PCI_BUS_NUM(alias), alias & 0xff);
	return 0;
}

/*
 * NB - intel-iommu lacks any sort of reference counting for the users of
 * dependent devices.  If multiple endpoints have intersecting dependent
 * devices, unbinding the driver from any one of them will possibly leave
 * the others unable to operate.
 */
static void iommu_detach_dependent_devices(struct intel_iommu *iommu,
					   struct device *dev)
{
	if (!iommu || !dev || !dev_is_pci(dev))
		return;

	pci_for_each_dma_alias(to_pci_dev(dev), &iommu_detach_dev_cb, iommu);
}

static void domain_remove_one_dev_info(struct dmar_domain *domain,
				       struct device *dev)
{
	struct device_domain_info *info, *tmp;
	struct intel_iommu *iommu;
	unsigned long flags;
	bool found = false;
	u8 bus, devfn;

	iommu = device_to_iommu(dev, &bus, &devfn);
	if (!iommu)
		return;

	spin_lock_irqsave(&device_domain_lock, flags);
	list_for_each_entry_safe(info, tmp, &domain->devices, link) {
		if (info->iommu == iommu && info->bus == bus &&
		    info->devfn == devfn) {
			unlink_domain_info(info);
			spin_unlock_irqrestore(&device_domain_lock, flags);

			iommu_disable_dev_iotlb(info);
			iommu_detach_dev(iommu, info->bus, info->devfn);
			iommu_detach_dependent_devices(iommu, dev);
			free_devinfo_mem(info);

			spin_lock_irqsave(&device_domain_lock, flags);

			if (found)
				break;
			else
				continue;
		}

		/* if there is no other devices under the same iommu
		 * owned by this domain, clear this iommu in iommu_bmp
		 * update iommu count and coherency
		 */
		if (info->iommu == iommu)
			found = true;
	}

	spin_unlock_irqrestore(&device_domain_lock, flags);

	if (found == 0) {
		domain_detach_iommu(domain, iommu);
		if (!domain_type_is_vm_or_si(domain))
			iommu_detach_domain(domain, iommu);
	}
}
//主要设置iovad，gaw，agaw，pgd
//次要设置iommu_coherency，iommu_snooping，iommu_superpage，max_addr，
static int md_domain_init(struct dmar_domain *domain, int guest_width)
{
	int adjust_width;
	
	//iovad->granule = VTD_PAGE_SIZE;
	//iovad->start_pfn = IOVA_START_PFN;
	//iovad->dma_32bit_pfn = DMA_32BIT_PFN;
	//为domain->iovad的成员赋值
	init_iova_domain(&domain->iovad, VTD_PAGE_SIZE, IOVA_START_PFN,
			DMA_32BIT_PFN);
	//设置默认的红黑树
	domain_reserve_special_ranges(domain);

	/* calculate AGAW */
	domain->gaw = guest_width;
	adjust_width = guestwidth_to_adjustwidth(guest_width);
	domain->agaw = width_to_agaw(adjust_width);

	domain->iommu_coherency = 0;
	domain->iommu_snooping = 0;
	domain->iommu_superpage = 0;
	domain->max_addr = 0;

	/* always allocate the top pgd */
	domain->pgd = (struct dma_pte *)alloc_pgtable_page(domain->nid);
	if (!domain->pgd)
		return -ENOMEM;
	domain_flush_cache(domain, domain->pgd, PAGE_SIZE);
	return 0;
}

//申请的是dmar domain，返回的是dmar domain中的iommu domain
static struct iommu_domain *intel_iommu_domain_alloc(unsigned type)
{
	struct dmar_domain *dmar_domain;
	struct iommu_domain *domain;

	if (type != IOMMU_DOMAIN_UNMANAGED)
		return NULL;

	dmar_domain = alloc_domain(DOMAIN_FLAG_VIRTUAL_MACHINE);
	if (!dmar_domain) {
		printk(KERN_ERR
			"intel_iommu_domain_init: dmar_domain == NULL\n");
		return NULL;
	}
	if (md_domain_init(dmar_domain, DEFAULT_DOMAIN_ADDRESS_WIDTH)) {
		printk(KERN_ERR
			"intel_iommu_domain_init() failed\n");
		domain_exit(dmar_domain);
		return NULL;
	}
	domain_update_iommu_cap(dmar_domain);

	domain = &dmar_domain->domain;
	domain->geometry.aperture_start = 0;
	domain->geometry.aperture_end   = __DOMAIN_MAX_ADDR(dmar_domain->gaw);
	domain->geometry.force_aperture = true;

	return domain;
}

static void intel_iommu_domain_free(struct iommu_domain *domain)
{
	domain_exit(to_dmar_domain(domain));
}

static int intel_iommu_attach_device(struct iommu_domain *domain,
				     struct device *dev)
{
	struct dmar_domain *dmar_domain = to_dmar_domain(domain);
	struct intel_iommu *iommu;
	int addr_width;
	u8 bus, devfn;

	if (device_is_rmrr_locked(dev)) {
		dev_warn(dev, "Device is ineligible for IOMMU domain attach due to platform RMRR
						requirement.  Contact your platform vendor.\n");
		return -EPERM;
	}

	/* normally dev is not mapped */
	//通常设备没有映射
	if (unlikely(domain_context_mapped(dev))) {
		//这里面通常不会执行到
		struct dmar_domain *old_domain;

		old_domain = find_domain(dev);
		//若设备已有domain,则移除原来的domain
		if (old_domain) {
			if (domain_type_is_vm_or_si(dmar_domain))
				domain_remove_one_dev_info(old_domain, dev);
			else
				domain_remove_dev_info(old_domain);

			if (!domain_type_is_vm_or_si(old_domain) &&
			     list_empty(&old_domain->devices))
				domain_exit(old_domain);
		}
	}

	iommu = device_to_iommu(dev, &bus, &devfn);
	if (!iommu)
		return -ENODEV;

	/* check if this iommu agaw is sufficient for max mapped address */
	//检查iommu的agaw是否足够做最大的映射地址
	//最大和硬件一样宽
	addr_width = agaw_to_width(iommu->agaw);
	if (addr_width > cap_mgaw(iommu->cap))
		addr_width = cap_mgaw(iommu->cap);

	//maximum mapped address 
	if (dmar_domain->max_addr > (1LL << addr_width)) {
		printk(KERN_ERR "%s: iommu width (%d) is not "
		       "sufficient for the mapped address (%llx)\n",
		       __func__, addr_width, dmar_domain->max_addr);
		return -EFAULT;
	}
	dmar_domain->gaw = addr_width;

	/*
	 * Knock out extra levels of page tables if necessary
	 */
	while (iommu->agaw < dmar_domain->agaw) {
		struct dma_pte *pte;

		pte = dmar_domain->pgd;
		if (dma_pte_present(pte)) {
			dmar_domain->pgd = (struct dma_pte *)
				phys_to_virt(dma_pte_addr(pte));
			free_pgtable_page(pte);
		}
		dmar_domain->agaw--;
	}

	//将设备和新的domain关联
	return domain_add_dev_info(dmar_domain, dev, CONTEXT_TT_MULTI_LEVEL);
}

static void intel_iommu_detach_device(struct iommu_domain *domain,
				      struct device *dev)
{
	domain_remove_one_dev_info(to_dmar_domain(domain), dev);
}

//domain，虚拟地址(gpa)，物理地址(hpa)，要映射的大小，
//权限
static int intel_iommu_map(struct iommu_domain *domain,
			   unsigned long iova, phys_addr_t hpa,
			   size_t size, int iommu_prot)
{
	struct dmar_domain *dmar_domain = to_dmar_domain(domain);
	u64 max_addr;
	int prot = 0;
	int ret;

	if (iommu_prot & IOMMU_READ)
		prot |= DMA_PTE_READ;
	if (iommu_prot & IOMMU_WRITE)
		prot |= DMA_PTE_WRITE;
	if ((iommu_prot & IOMMU_CACHE) && dmar_domain->iommu_snooping)
		prot |= DMA_PTE_SNP;

	//dmar_domain->max_addr如果比这个小就
	//更新dmar_domain->max_addr
	//的值
	max_addr = iova + size;//最大值
	
	if (dmar_domain->max_addr < max_addr) {
		u64 end;

		/* check if minimum agaw is sufficient for mapped address */
		//获取iommu的结尾地址
		end = __DOMAIN_MAX_ADDR(dmar_domain->gaw) + 1;
		if (end < max_addr) {
			printk(KERN_ERR "%s: iommu width (%d) is not "
			       "sufficient for the mapped address (%llx)\n",
			       __func__, dmar_domain->gaw, max_addr);
			return -EFAULT;
		}
		//更新
		dmar_domain->max_addr = max_addr;
	}
	/* Round up size to next multiple of PAGE_SIZE, if it and
	   the low bits of hpa would take us onto the next page */
	size = aligned_nrpages(hpa, size);//返回多少页，物理地址空间的

	//参数准备好了

	
	ret = domain_pfn_mapping(dmar_domain, iova >> VTD_PAGE_SHIFT,
				 hpa >> VTD_PAGE_SHIFT, size, prot);
	return ret;
}

static size_t intel_iommu_unmap(struct iommu_domain *domain,
				unsigned long iova, size_t size)
{
	struct dmar_domain *dmar_domain = to_dmar_domain(domain);
	struct page *freelist = NULL;
	struct intel_iommu *iommu;
	unsigned long start_pfn, last_pfn;
	unsigned int npages;
	int iommu_id, num, ndomains, level = 0;

	/* Cope with horrid API which requires us to unmap more than the
	   size argument if it happens to be a large-page mapping. */
	if (!pfn_to_dma_pte(dmar_domain, iova >> VTD_PAGE_SHIFT, &level))
		BUG();

	if (size < VTD_PAGE_SIZE << level_to_offset_bits(level))
		size = VTD_PAGE_SIZE << level_to_offset_bits(level);

	start_pfn = iova >> VTD_PAGE_SHIFT;
	last_pfn = (iova + size - 1) >> VTD_PAGE_SHIFT;

	freelist = domain_unmap(dmar_domain, start_pfn, last_pfn);

	npages = last_pfn - start_pfn + 1;

	for_each_set_bit(iommu_id, dmar_domain->iommu_bmp, g_num_of_iommus) {
               iommu = g_iommus[iommu_id];

               /*
                * find bit position of dmar_domain
                */
               ndomains = cap_ndoms(iommu->cap);
               for_each_set_bit(num, iommu->domain_ids, ndomains) {
                       if (iommu->domains[num] == dmar_domain)
                               iommu_flush_iotlb_psi(iommu, num, start_pfn,
						     npages, !freelist, 0);
	       }

	}

	dma_free_pagelist(freelist);

	if (dmar_domain->max_addr == iova + size)
		dmar_domain->max_addr = iova;

	return size;
}

static phys_addr_t intel_iommu_iova_to_phys(struct iommu_domain *domain,
					    dma_addr_t iova)
{
	struct dmar_domain *dmar_domain = to_dmar_domain(domain);
	struct dma_pte *pte;
	int level = 0;
	u64 phys = 0;

	pte = pfn_to_dma_pte(dmar_domain, iova >> VTD_PAGE_SHIFT, &level);
	if (pte)
		phys = dma_pte_addr(pte);

	return phys;
}
//判断iommu对应硬件的能力
static bool intel_iommu_capable(enum iommu_cap cap)
{
	//如果有cache一致性的能力，返回iommu硬件是否支持
	//snoop control
	if (cap == IOMMU_CAP_CACHE_COHERENCY)
		//返回所有的iommu是否都支持snoop control
		return domain_update_iommu_snooping(NULL) == 1;
	if (cap == IOMMU_CAP_INTR_REMAP)
		return irq_remapping_enabled == 1;

	return false;
}

static int intel_iommu_add_device(struct device *dev)
{
	struct intel_iommu *iommu;
	struct iommu_group *group;
	u8 bus, devfn;

	iommu = device_to_iommu(dev, &bus, &devfn);
	if (!iommu)
		return -ENODEV;

	iommu_device_link(iommu->iommu_dev, dev);

	group = iommu_group_get_for_dev(dev);

	if (IS_ERR(group))
		return PTR_ERR(group);

	iommu_group_put(group);
	return 0;
}

static void intel_iommu_remove_device(struct device *dev)
{
	struct intel_iommu *iommu;
	u8 bus, devfn;

	iommu = device_to_iommu(dev, &bus, &devfn);
	if (!iommu)
		return;

	iommu_group_remove_device(dev);

	iommu_device_unlink(iommu->iommu_dev, dev);
}

static const struct iommu_ops intel_iommu_ops = {
	//->capable .capable
	.capable	= intel_iommu_capable,
	//	申请的是dmar domain，返回的是dmar domain中的iommu domain
	.domain_alloc	= intel_iommu_domain_alloc,
	.domain_free	= intel_iommu_domain_free,
	//
	.attach_dev	= intel_iommu_attach_device,
	.detach_dev	= intel_iommu_detach_device,
	//主要函数是__domain_mapping
	.map		= intel_iommu_map,
	.unmap		= intel_iommu_unmap,
	
	.map_sg		= default_iommu_map_sg,
	//根据iova地址，找到物理地址
	.iova_to_phys	= intel_iommu_iova_to_phys,
	//添加一个device到iommu_group
	.add_device	= intel_iommu_add_device,
	.remove_device	= intel_iommu_remove_device,
	.pgsize_bitmap	= INTEL_IOMMU_PGSIZES,
};

static void quirk_iommu_g4x_gfx(struct pci_dev *dev)
{
	/* G4x/GM45 integrated gfx dmar support is totally busted. */
	printk(KERN_INFO "DMAR: Disabling IOMMU for graphics on this chipset\n");
	dmar_map_gfx = 0;
}

DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_INTEL, 0x2a40, quirk_iommu_g4x_gfx);
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_INTEL, 0x2e00, quirk_iommu_g4x_gfx);
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_INTEL, 0x2e10, quirk_iommu_g4x_gfx);
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_INTEL, 0x2e20, quirk_iommu_g4x_gfx);
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_INTEL, 0x2e30, quirk_iommu_g4x_gfx);
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_INTEL, 0x2e40, quirk_iommu_g4x_gfx);
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_INTEL, 0x2e90, quirk_iommu_g4x_gfx);

static void quirk_iommu_rwbf(struct pci_dev *dev)
{
	/*
	 * Mobile 4 Series Chipset neglects to set RWBF capability,
	 * but needs it. Same seems to hold for the desktop versions.
	 */
	printk(KERN_INFO "DMAR: Forcing write-buffer flush capability\n");
	rwbf_quirk = 1;
}

DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_INTEL, 0x2a40, quirk_iommu_rwbf);
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_INTEL, 0x2e00, quirk_iommu_rwbf);
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_INTEL, 0x2e10, quirk_iommu_rwbf);
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_INTEL, 0x2e20, quirk_iommu_rwbf);
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_INTEL, 0x2e30, quirk_iommu_rwbf);
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_INTEL, 0x2e40, quirk_iommu_rwbf);
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_INTEL, 0x2e90, quirk_iommu_rwbf);

#define GGC 0x52
#define GGC_MEMORY_SIZE_MASK	(0xf << 8)
#define GGC_MEMORY_SIZE_NONE	(0x0 << 8)
#define GGC_MEMORY_SIZE_1M	(0x1 << 8)
#define GGC_MEMORY_SIZE_2M	(0x3 << 8)
#define GGC_MEMORY_VT_ENABLED	(0x8 << 8)
#define GGC_MEMORY_SIZE_2M_VT	(0x9 << 8)
#define GGC_MEMORY_SIZE_3M_VT	(0xa << 8)
#define GGC_MEMORY_SIZE_4M_VT	(0xb << 8)

static void quirk_calpella_no_shadow_gtt(struct pci_dev *dev)
{
	unsigned short ggc;

	if (pci_read_config_word(dev, GGC, &ggc))
		return;

	if (!(ggc & GGC_MEMORY_VT_ENABLED)) {
		printk(KERN_INFO "DMAR: BIOS has allocated no shadow GTT; disabling IOMMU for graphics\n");
		dmar_map_gfx = 0;
	} else if (dmar_map_gfx) {
		/* we have to ensure the gfx device is idle before we flush */
		printk(KERN_INFO "DMAR: Disabling batched IOTLB flush on Ironlake\n");
		intel_iommu_strict = 1;
       }
}
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_INTEL, 0x0040, quirk_calpella_no_shadow_gtt);
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_INTEL, 0x0044, quirk_calpella_no_shadow_gtt);
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_INTEL, 0x0062, quirk_calpella_no_shadow_gtt);
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_INTEL, 0x006a, quirk_calpella_no_shadow_gtt);

/* On Tylersburg chipsets, some BIOSes have been known to enable the
   ISOCH DMAR unit for the Azalia sound device, but not give it any
   TLB entries, which causes it to deadlock. Check for that.  We do
   this in a function called from init_dmars(), instead of in a PCI
   quirk, because we don't want to print the obnoxious "BIOS broken"
   message if VT-d is actually disabled.
*/
//在芯片组，一些BIOS已经知道使能 ISOCH DMAR unit为Azalia声卡芯片设备
//但是没有提供任何的TLB入口，导致了死锁。
//检查后，我们在在init_dmars这个函数里代替了pci quirk ，因为
//如果vd-t是禁止的，我们不想打印的BIOS broken信息

//检查芯片组的isoch。dma路由到isoch dmar 单元，tlb是否为空的。
static void __init check_tylersburg_isoch(void)
{
	struct pci_dev *pdev;
	uint32_t vtisochctrl;

	/* If there's no Azalia in the system anyway, forget it. */
	//如果没有azalia设备
	pdev = pci_get_device(PCI_VENDOR_ID_INTEL, 0x3a3e, NULL);
	if (!pdev)
		return;//如果没有就返回了
	pci_dev_put(pdev);//说明有azalia设备

	/* System Management Registers. Might be hidden, in which case
	   we can't do the sanity check. But that's OK, because the
	   known-broken BIOSes _don't_ actually hide it, so far. */
	   //很可能隐藏了，这种情况下我们不能检查，
	   //但是到目前为止也行，因为已经知道没有真正的隐藏
	pdev = pci_get_device(PCI_VENDOR_ID_INTEL, 0x342e, NULL);
	if (!pdev)
		return;

	if (pci_read_config_dword(pdev, 0x188, &vtisochctrl)) {
		pci_dev_put(pdev);
		return;
	}

	pci_dev_put(pdev);

	/* If Azalia DMA is routed to the non-isoch DMAR unit, fine. */
	//如果azalia dma 路由到了 non-isoch DMAR unit ，那么很好。
	if (vtisochctrl & 1)
		return;

	/* Drop all bits other than the number of TLB entries */
	//只保留 the number of TLB entries
	vtisochctrl &= 0x1c;

	/* If we have the recommended number of TLB entries (16), fine. */
	if (vtisochctrl == 0x10)
		return;
//dma路由到isoch dmar 单元，但是tlb是空的
	/* Zero TLB entries? You get to ride the short bus to school. */
	if (!vtisochctrl) {
		WARN(1, "Your BIOS is broken; DMA routed to ISOCH DMAR unit but no TLB space.\n"
		     "BIOS vendor: %s; Ver: %s; Product Version: %s\n",
		     dmi_get_system_info(DMI_BIOS_VENDOR),
		     dmi_get_system_info(DMI_BIOS_VERSION),
		     dmi_get_system_info(DMI_PRODUCT_VERSION));
		iommu_identity_mapping |= IDENTMAP_AZALIA;
		return;
	}
	
	printk(KERN_WARNING "DMAR: Recommended TLB entries for ISOCH unit is 16; your BIOS set %d\n",
	       vtisochctrl);
}

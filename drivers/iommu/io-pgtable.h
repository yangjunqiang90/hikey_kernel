#ifndef __IO_PGTABLE_H
#define __IO_PGTABLE_H

/*
 * Public API for use by IOMMU drivers
 */
 //系统和硬件的属性值
enum io_pgtable_fmt {
	ARM_32_LPAE_S1,//一级stage 32位系统
	ARM_32_LPAE_S2,//二级stage 32位系统
	ARM_64_LPAE_S1,//一级stage 64位系统
	ARM_64_LPAE_S2,//二级stage 64位系统
	IO_PGTABLE_NUM_FMTS,
};

/**
 * struct iommu_gather_ops - IOMMU callbacks for TLB and page table management.
 *
 * @tlb_flush_all: Synchronously invalidate the entire TLB context.
 * @tlb_add_flush: Queue up a TLB invalidation for a virtual address range.
 * @tlb_sync:      Ensure any queue TLB invalidation has taken effect.
 * @flush_pgtable: Ensure page table updates are visible to the IOMMU.
 *
 * Note that these can all be called in atomic context and must therefore
 * not block.
 */
struct iommu_gather_ops {
	void (*tlb_flush_all)(void *cookie);
	void (*tlb_add_flush)(unsigned long iova, size_t size, bool leaf,
			      void *cookie);
	void (*tlb_sync)(void *cookie);
	void (*flush_pgtable)(void *ptr, size_t size, void *cookie);
};

/**
 * struct io_pgtable_cfg - Configuration data for a set of page tables.
 *
 * @quirks:        A bitmap of hardware quirks that require some special
 *                 action by the low-level page table allocator.
 * @pgsize_bitmap: A bitmap of page sizes supported by this set of page
 *                 tables.
 * @ias:           Input address (iova) size, in bits.
 * @oas:           Output address (paddr) size, in bits.
 * @tlb:           TLB management callbacks for this set of tables.
 */
 //为页表配置的数据
struct io_pgtable_cfg {
	#define IO_PGTABLE_QUIRK_ARM_NS	(1 << 0)	/* Set NS bit in PTEs */
	//一个位图，需要一些指定的动作，
	int				quirks;
	//根据系统页的大小，设置级数对应的位
	//系统是4k，那么[12]，[21]，[30]位置1
	unsigned long			pgsize_bitmap;//arm_smmu_ops.pgsize_bitmap
	//输入地址的大小，多少位，
	//小于等于48
	unsigned int			ias;
	//输出地址的大小，多少位
	////小于等于48
	unsigned int			oas;
	//tlb管理回调
	//就是arm_smmu_gather_ops函数
	const struct iommu_gather_ops	*tlb;

	/* Low-level data specific to the table format */
	//SMMU_CBn_CBA2R.VA64 is 1
	//
	union {
		struct {
			u64	ttbr[2];//这个页表的物理地址
			u64	tcr;//SMMU_CBn_TCR
			u64	mair[2];//SMMU_CBn_MAIRm，只设置了mair[0]
		} arm_lpae_s1_cfg;

		struct {
			u64	vttbr;
			u64	vtcr;
		} arm_lpae_s2_cfg;
	};
};

/**
 * struct io_pgtable_ops - Page table manipulation API for IOMMU drivers.
 *为iommu驱动的页表操作提供API
 * @map:          Map a physically contiguous memory region.
	映射一个连续的内存区域
 * @unmap:        Unmap a physically contiguous memory region.
	取消映射一个物理连续内存区域
 * @iova_to_phys: Translate iova to physical address.
	iova到物理地址的转换
* 
 * These functions map directly onto the iommu_ops member functions with
 * the same names.
 */
struct io_pgtable_ops {
	//映射一个连续的内存区域
	int (*map)(struct io_pgtable_ops *ops, unsigned long iova,
		   phys_addr_t paddr, size_t size, int prot);
	//取消映射一个物理连续内存区域
	int (*unmap)(struct io_pgtable_ops *ops, unsigned long iova,
		     size_t size);
	//iova到物理地址的转换
	phys_addr_t (*iova_to_phys)(struct io_pgtable_ops *ops,
				    unsigned long iova);
};

/**
 * alloc_io_pgtable_ops() - Allocate a page table allocator for use by an IOMMU.
 *
 * @fmt:    The page table format.
 * @cfg:    The page table configuration. This will be modified to represent
 *          the configuration actually provided by the allocator (e.g. the
 *          pgsize_bitmap may be restricted).
 * @cookie: An opaque token provided by the IOMMU driver and passed back to
 *          the callback routines in cfg->tlb.
 */
struct io_pgtable_ops *alloc_io_pgtable_ops(enum io_pgtable_fmt fmt,
					    struct io_pgtable_cfg *cfg,
					    void *cookie);

/**
 * free_io_pgtable_ops() - Free an io_pgtable_ops structure. The caller
 *                         *must* ensure that the page table is no longer
 *                         live, but the TLB can be dirty.
 *
 * @ops: The ops returned from alloc_io_pgtable_ops.
 */
void free_io_pgtable_ops(struct io_pgtable_ops *ops);


/*
 * Internal structures for page table allocator implementations.
 */

/**
 * struct io_pgtable - Internal structure describing a set of page tables.
 *
 * @fmt:    The page table format.
 * @cookie: An opaque token provided by the IOMMU driver and passed back to
 *          any callback routines.
 * @cfg:    A copy of the page table configuration.
 * @ops:    The page table operations in use for this set of page tables.
 */
 //描述一个页表的结构体
struct io_pgtable {
	enum io_pgtable_fmt	fmt;//描述系统和硬件属性
	//就是arm_smmu_domain
	void			*cookie;
	//为页表配置的数据
	struct io_pgtable_cfg	cfg;
	//操作函数
	struct io_pgtable_ops	ops;
};

/**
 * struct io_pgtable_init_fns - Alloc/free a set of page tables for a
 *                              particular format.
 *
 * @alloc: Allocate a set of page tables described by cfg.
 * @free:  Free the page tables associated with iop.
 */
 //申请释放特定格式的页表
struct io_pgtable_init_fns {
	//根据配置申请一个页表
	struct io_pgtable *(*alloc)(struct io_pgtable_cfg *cfg, void *cookie);
	//释放页表
	void (*free)(struct io_pgtable *iop);
};

#endif /* __IO_PGTABLE_H */

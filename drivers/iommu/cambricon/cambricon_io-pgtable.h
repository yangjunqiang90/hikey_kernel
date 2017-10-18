#ifndef __IO_PGTABLE_H
#define __IO_PGTABLE_H

/*
 * Public API for use by IOMMU drivers
 */
 //ϵͳ��Ӳ��������ֵ
enum io_pgtable_fmt {
	ARM_32_LPAE_S1,//һ��stage 32λϵͳ
	ARM_32_LPAE_S2,//����stage 32λϵͳ
	ARM_64_LPAE_S1,//һ��stage 64λϵͳ
	ARM_64_LPAE_S2,//����stage 64λϵͳ
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
 //Ϊҳ�����õ�����
struct io_pgtable_cfg {
	#define IO_PGTABLE_QUIRK_ARM_NS	(1 << 0)	/* Set NS bit in PTEs */
	//һ��λͼ����ҪһЩָ���Ķ�����
	int				quirks;
	//����ϵͳҳ�Ĵ�С�����ü�����Ӧ��λ
	//ϵͳ��4k����ô[12]��[21]��[30]λ��1
	unsigned long			pgsize_bitmap;//arm_smmu_ops.pgsize_bitmap
	//�����ַ�Ĵ�С������λ��
	//С�ڵ���48
	unsigned int			ias;
	//�����ַ�Ĵ�С������λ
	////С�ڵ���48
	unsigned int			oas;
	//tlb����ص�
	//����arm_smmu_gather_ops����
	const struct iommu_gather_ops	*tlb;

	/* Low-level data specific to the table format */
	//SMMU_CBn_CBA2R.VA64 is 1
	//
	union {
		struct {
			u64	ttbr[2];//���ҳ��������ַ
			u64	tcr;//SMMU_CBn_TCR
			u64	mair[2];//SMMU_CBn_MAIRm��ֻ������mair[0]
		} arm_lpae_s1_cfg;

		struct {
			u64	vttbr;
			u64	vtcr;
		} arm_lpae_s2_cfg;
	};
};

/**
 * struct io_pgtable_ops - Page table manipulation API for IOMMU drivers.
 *Ϊiommu������ҳ������ṩAPI
 * @map:          Map a physically contiguous memory region.
	ӳ��һ���������ڴ�����
 * @unmap:        Unmap a physically contiguous memory region.
	ȡ��ӳ��һ�����������ڴ�����
 * @iova_to_phys: Translate iova to physical address.
	iova�������ַ��ת��
* 
 * These functions map directly onto the iommu_ops member functions with
 * the same names.
 */
struct io_pgtable_ops {
	//ӳ��һ���������ڴ�����
	int (*map)(struct io_pgtable_ops *ops, unsigned long iova,
		   phys_addr_t paddr, size_t size, int prot);
	//ȡ��ӳ��һ�����������ڴ�����
	int (*unmap)(struct io_pgtable_ops *ops, unsigned long iova,
		     size_t size);
	//iova�������ַ��ת��
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
 //����һ��ҳ��Ľṹ��
struct io_pgtable {
	enum io_pgtable_fmt	fmt;//����ϵͳ��Ӳ������
	//����arm_smmu_domain
	void			*cookie;
	//Ϊҳ�����õ�����
	struct io_pgtable_cfg	cfg;
	//��������
	struct io_pgtable_ops	ops;
};

/**
 * struct io_pgtable_init_fns - Alloc/free a set of page tables for a
 *                              particular format.
 *
 * @alloc: Allocate a set of page tables described by cfg.
 * @free:  Free the page tables associated with iop.
 */
 //�����ͷ��ض���ʽ��ҳ��
struct io_pgtable_init_fns {
	//������������һ��ҳ��
	struct io_pgtable *(*alloc)(struct io_pgtable_cfg *cfg, void *cookie);
	//�ͷ�ҳ��
	void (*free)(struct io_pgtable *iop);
};

#endif /* __IO_PGTABLE_H */

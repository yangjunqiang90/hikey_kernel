
#define pr_fmt(fmt)	"cambricon-arm-lpae cambricomio-pgtable: " fmt

#include <linux/iommu.h>
#include <linux/kernel.h>
#include <linux/sizes.h>
#include <linux/slab.h>
#include <linux/types.h>

#include "cambricon_io-pgtable.h"

//��ַ���Ŀ��
#define ARM_LPAE_MAX_ADDR_BITS		48
#define ARM_LPAE_S2_MAX_CONCAT_PAGES	16
//��༸��ҳ�����
#define ARM_LPAE_MAX_LEVELS		4

/* Struct accessors */
//���ݽṹ��ĳ�Ա��ַ���ҵ��ṹ���һЩ��
//data����arm_lpae_io_pgtable��pgtable����io_pgtable

//����io_pgtable�õ�arm_lpae_io_pgtable
#define io_pgtable_to_data(x)						\
	container_of((x), struct arm_lpae_io_pgtable, iop)

//����io_pgtable_ops�ҵ�io_pgtable
#define io_pgtable_ops_to_pgtable(x)					\
	container_of((x), struct io_pgtable, ops)

//����io_pgtable_ops�ҵ�arm_lpae_io_pgtable
#define io_pgtable_ops_to_data(x)					\
	io_pgtable_to_data(io_pgtable_ops_to_pgtable(x))



/*
 * For consistency with the architecture, we always consider
 * ARM_LPAE_MAX_LEVELS levels, with the walk starting at level n >=0
 */
 //���ݼܹ���һ���ԣ��������ǿ���ARM_LPAE_MAX_LEVELS
 //����level��ȥd�е�level
//leves�Ŀ�ʼ��ţ�[0��3]���ߵ�ַ��Ӧ��level 0���͵�ַ��Ӧ��level 3
#define ARM_LPAE_START_LVL(d)		(ARM_LPAE_MAX_LEVELS - (d)->levels)

/*
 * Calculate the right shift amount to get to the portion describing level l
 * in a virtual address mapped by the pagetable in d.
 */
 //�������Ƶ�������
 //l ��lvl
 //d ��arm_lpae_io_pgtable
 
 //ÿ��level��ʼ��index
#define ARM_LPAE_LVL_SHIFT(l,d)						\
	((((d)->levels - ((l) - ARM_LPAE_START_LVL(d) + 1))		\
	  * (d)->bits_per_level) + (d)->pg_shift)

//����ҳ���м�ҳ�����϶��룬����������һҳ
#define  ARM_LPAE_PAGES_PER_PGD(d)					\
	DIV_ROUND_UP((d)->pgd_size, 1UL << (d)->pg_shift)

/*
 * Calculate the index at level l used to map virtual address a using the
 * pagetable in d.
 */
#define ARM_LPAE_PGD_IDX(l,d)						\
	((l) == ARM_LPAE_START_LVL(d) ? ilog2(ARM_LPAE_PAGES_PER_PGD(d)) : 0)

//Large Physical Address Extension (LPAE)
//iova, 
//lvl 
//data
#define ARM_LPAE_LVL_IDX(a,l,d)						\
	(((u64)(a) >> ARM_LPAE_LVL_SHIFT(l,d)) &			\
	 ((1 << ((d)->bits_per_level + ARM_LPAE_PGD_IDX(l,d))) - 1))

/* Calculate the block/page mapping size at level l for pagetable in d. */
//����d��������level l��ӳ���Ĵ�С
//һ������ָ������ݴ�С

//l ��level
//d ��arm_lpae_io_pgtable
//��l��ҳ��һ������Ĵ�С
#define ARM_LPAE_BLOCK_SIZE(l,d)					\
	(1 << (ilog2(sizeof(arm_lpae_iopte)) +				\
		((ARM_LPAE_MAX_LEVELS - (l)) * (d)->bits_per_level)))

/* Page table bits */
//�����λ�Ǹñ�����ָ���ַ�����ͣ�
//�հף����һ����ҳ���飬ָ����һ��ҳ��
#define ARM_LPAE_PTE_TYPE_SHIFT		0
#define ARM_LPAE_PTE_TYPE_MASK		0x3

//pte�����λ��ֵ
//�������һ��ҳ��
#define ARM_LPAE_PTE_TYPE_BLOCK		1
#define ARM_LPAE_PTE_TYPE_TABLE		3
//��ʾ�����һ��ҳ��ָ������ݲ�����ҳ���ˡ�
#define ARM_LPAE_PTE_TYPE_PAGE		3


#define ARM_LPAE_PTE_NSTABLE		(((arm_lpae_iopte)1) << 63)
#define ARM_LPAE_PTE_XN			(((arm_lpae_iopte)3) << 53)
#define ARM_LPAE_PTE_AF			(((arm_lpae_iopte)1) << 10)
#define ARM_LPAE_PTE_SH_NS		(((arm_lpae_iopte)0) << 8)
#define ARM_LPAE_PTE_SH_OS		(((arm_lpae_iopte)2) << 8)
#define ARM_LPAE_PTE_SH_IS		(((arm_lpae_iopte)3) << 8)
#define ARM_LPAE_PTE_NS			(((arm_lpae_iopte)1) << 5)
#define ARM_LPAE_PTE_VALID		(((arm_lpae_iopte)1) << 0)

#define ARM_LPAE_PTE_ATTR_LO_MASK	(((arm_lpae_iopte)0x3ff) << 2)
/* Ignore the contiguous bit for block splitting */
#define ARM_LPAE_PTE_ATTR_HI_MASK	(((arm_lpae_iopte)6) << 52)
#define ARM_LPAE_PTE_ATTR_MASK		(ARM_LPAE_PTE_ATTR_LO_MASK |	\
					 ARM_LPAE_PTE_ATTR_HI_MASK)

/* Stage-1 PTE */
#define ARM_LPAE_PTE_AP_UNPRIV		(((arm_lpae_iopte)1) << 6)
#define ARM_LPAE_PTE_AP_RDONLY		(((arm_lpae_iopte)2) << 6)
//������λ��dma cache corehent
#define ARM_LPAE_PTE_ATTRINDX_SHIFT	2
#define ARM_LPAE_PTE_nG			(((arm_lpae_iopte)1) << 11)

/* Stage-2 PTE */
#define ARM_LPAE_PTE_HAP_FAULT		(((arm_lpae_iopte)0) << 6)
#define ARM_LPAE_PTE_HAP_READ		(((arm_lpae_iopte)1) << 6)
#define ARM_LPAE_PTE_HAP_WRITE		(((arm_lpae_iopte)2) << 6)
#define ARM_LPAE_PTE_MEMATTR_OIWB	(((arm_lpae_iopte)0xf) << 2)
#define ARM_LPAE_PTE_MEMATTR_NC		(((arm_lpae_iopte)0x5) << 2)
#define ARM_LPAE_PTE_MEMATTR_DEV	(((arm_lpae_iopte)0x1) << 2)

//end of pte bits

/* Register bits */
#define ARM_32_LPAE_TCR_EAE		(1 << 31)
#define ARM_64_LPAE_S2_TCR_RES1		(1 << 31)


//SMMU_CBn_TCR�Ĵ���
#define ARM_LPAE_TCR_EPD1		(1 << 23)

#define ARM_LPAE_TCR_TG0_4K		(0 << 14)
#define ARM_LPAE_TCR_TG0_64K		(1 << 14)
#define ARM_LPAE_TCR_TG0_16K		(2 << 14)

#define ARM_LPAE_TCR_SH0_SHIFT		12
#define ARM_LPAE_TCR_SH0_MASK		0x3
#define ARM_LPAE_TCR_SH_NS		0
#define ARM_LPAE_TCR_SH_OS		2
//Inner Shareable
#define ARM_LPAE_TCR_SH_IS		3

#define ARM_LPAE_TCR_ORGN0_SHIFT	10
#define ARM_LPAE_TCR_IRGN0_SHIFT	8
#define ARM_LPAE_TCR_RGN_MASK		0x3
#define ARM_LPAE_TCR_RGN_NC		0
#define ARM_LPAE_TCR_RGN_WBWA		1
#define ARM_LPAE_TCR_RGN_WT		2
#define ARM_LPAE_TCR_RGN_WB		3

#define ARM_LPAE_TCR_SL0_SHIFT		6
#define ARM_LPAE_TCR_SL0_MASK		0x3

#define ARM_LPAE_TCR_T0SZ_SHIFT		0
#define ARM_LPAE_TCR_SZ_MASK		0xf

#define ARM_LPAE_TCR_PS_SHIFT		16
#define ARM_LPAE_TCR_PS_MASK		0x7

#define ARM_LPAE_TCR_IPS_SHIFT		32
#define ARM_LPAE_TCR_IPS_MASK		0x7

#define ARM_LPAE_TCR_PS_32_BIT		0x0ULL
#define ARM_LPAE_TCR_PS_36_BIT		0x1ULL
#define ARM_LPAE_TCR_PS_40_BIT		0x2ULL
#define ARM_LPAE_TCR_PS_42_BIT		0x3ULL
#define ARM_LPAE_TCR_PS_44_BIT		0x4ULL
#define ARM_LPAE_TCR_PS_48_BIT		0x5ULL

#define ARM_LPAE_MAIR_ATTR_SHIFT(n)	((n) << 3)
#define ARM_LPAE_MAIR_ATTR_MASK		0xff
#define ARM_LPAE_MAIR_ATTR_DEVICE	0x04
//uter Non-cacheable Normal memory
#define ARM_LPAE_MAIR_ATTR_NC		0x44
#define ARM_LPAE_MAIR_ATTR_WBRWA	0xff
#define ARM_LPAE_MAIR_ATTR_IDX_NC	0
#define ARM_LPAE_MAIR_ATTR_IDX_CACHE	1
#define ARM_LPAE_MAIR_ATTR_IDX_DEV	2

/* IOPTE accessors */
#define iopte_deref(pte,d)					\
	(__va((pte) & ((1ULL << ARM_LPAE_MAX_ADDR_BITS) - 1)	\
	& ~((1ULL << (d)->pg_shift) - 1)))

//����pte�ĵ���λ
#define iopte_type(pte,l)					\
	(((pte) >> ARM_LPAE_PTE_TYPE_SHIFT) & ARM_LPAE_PTE_TYPE_MASK)

#define iopte_prot(pte)	((pte) & ARM_LPAE_PTE_ATTR_MASK)

//��������һ��ҳ���ж�pte�������Ƿ���ҳ
//����������һ��ҳ���ж�pte�ĵ���λ�Ƿ��ǿ�
#define iopte_leaf(pte,l)					\
	(l == (ARM_LPAE_MAX_LEVELS - 1) ?			\
		(iopte_type(pte,l) == ARM_LPAE_PTE_TYPE_PAGE) :	\
		(iopte_type(pte,l) == ARM_LPAE_PTE_TYPE_BLOCK))

#define iopte_to_pfn(pte,d)					\
	(((pte) & ((1ULL << ARM_LPAE_MAX_ADDR_BITS) - 1)) >> (d)->pg_shift)

#define pfn_to_iopte(pfn,d)					\
	(((pfn) << (d)->pg_shift) & ((1ULL << ARM_LPAE_MAX_ADDR_BITS) - 1))

//lpae �������ַ��չ
struct arm_lpae_io_pgtable {
	struct io_pgtable	iop;

	//����м���ҳ��
	int			levels;
	//����ҳ��Ĵ�С
	//��������ַ��45λ��ϵͳҳ��4k��
	//45-12=33��33��ÿ��9λ����ȡ����4
	//��ô������λ������33-9*3=5λ��
	//��������5λ��ÿ������λ�����5+3=8
	//���Զ����Ĵ�С����256�ֽ�
	//void			*pgdָ���ڴ�Ĵ�С
	size_t			pgd_size;
	
	//���Եõ�һҳ�ж��4k/16k/64k
	//ϵͳҳ�Ĵ�С�������12λ����ôlevel����9
	//�����13λ����ôlevel����10λ
	unsigned long		pg_shift;
	//ÿ��level�ö���λ��ʾ
	unsigned long		bits_per_level;

	void			*pgd;
};

//����pte��ҳ���������
typedef u64 arm_lpae_iopte;

static bool selftest_running = false;

static int __arm_lpae_unmap(struct arm_lpae_io_pgtable *data,
			    unsigned long iova, size_t size, int lvl,
			    arm_lpae_iopte *ptep);

//ptep ���Ƕ�Ӧ��ҳ����
static int arm_lpae_init_pte(struct arm_lpae_io_pgtable *data,
			     unsigned long iova, phys_addr_t paddr,
			     arm_lpae_iopte prot, int lvl,
			     arm_lpae_iopte *ptep)
{
	arm_lpae_iopte pte = prot;

	//��������һ��ҳ���ж�pte�������Ƿ���ҳ
	//����������һ��ҳ���ж�pte�������Ƿ��ǿ�
	//���������һ��ҳ������*ptep��������0���µ�
	//�����û��������д�����ء�
	if (iopte_leaf(*ptep, lvl)) {
		//��������һ��д�룬���Ҹñ�����ֵ�ˣ����ֵ�϶���page
		//����������һ��д�룬���Ҹñ�����ֵ�ˣ����ֵ�϶���block
		//ֻҪ���������ֵ�ˣ��ͷ��ش����롣
		/* We require an unmap first */
		WARN_ON(!selftest_running);
		return -EEXIST;//���ش����룬���Ѿ������ˡ�
	} else if (iopte_type(*ptep, lvl) == ARM_LPAE_PTE_TYPE_TABLE) {
		//
		/*
		 * We need to unmap and free the old table before
		 * overwriting it with a block entry.
		 */
		 //������д��֮ǰ���ͷ���ǰ�ľɵı��
		arm_lpae_iopte *tblp;
		size_t sz = ARM_LPAE_BLOCK_SIZE(lvl, data);

		tblp = ptep - ARM_LPAE_LVL_IDX(iova, lvl, data);
		if (WARN_ON(__arm_lpae_unmap(data, iova, sz, lvl, tblp) != sz))
			return -EINVAL;
	}

	if (data->iop.cfg.quirks & IO_PGTABLE_QUIRK_ARM_NS)
		pte |= ARM_LPAE_PTE_NS;

	if (lvl == ARM_LPAE_MAX_LEVELS - 1)//��������һ��ҳ��
		pte |= ARM_LPAE_PTE_TYPE_PAGE;
	else
		pte |= ARM_LPAE_PTE_TYPE_BLOCK;

	pte |= ARM_LPAE_PTE_AF | ARM_LPAE_PTE_SH_IS;
	pte |= pfn_to_iopte(paddr >> data->pg_shift, data);

	*ptep = pte;
	data->iop.cfg.tlb->flush_pgtable(ptep, sizeof(*ptep), data->iop.cookie);
	return 0;
}

//ptep ��ʼ����ҳ��
//lvl  ��ǰ��lvl ����СΪ1�����Ϊ3
// arm_lpae_iopte *ptep = data->pgd
//size ӳ��Ĵ�С����Ҫ��ĳһ������Ĵ�С���
static int __arm_lpae_map(struct arm_lpae_io_pgtable *data, unsigned long iova,
			  phys_addr_t paddr, size_t size, arm_lpae_iopte prot,
			  int lvl, arm_lpae_iopte *ptep)
{
	arm_lpae_iopte *cptep, pte;
	void *cookie = data->iop.cookie;
	//��lvl����һ�������ж��
	size_t block_size = ARM_LPAE_BLOCK_SIZE(lvl, data);

	/* Find our entry at the current level */
	//�ڵ�ǰ�������ҵ���Ӧ��ֵ
	//iova����һ����λ���������;�λ
	//data�е�levelsΪ3�����lvl����1
	ptep += ARM_LPAE_LVL_IDX(iova, lvl, data);

	/* If we can install a leaf entry at this level, then do so */
	//size ��Ҫ��4k/2M/1G
	if (size == block_size && (size & data->iop.cfg.pgsize_bitmap))
		return arm_lpae_init_pte(data, iova, paddr, prot, lvl, ptep);

	/* We can't allocate tables at the final level */
	//���һҳ��ʱ����������ҳ����
	if (WARN_ON(lvl >= ARM_LPAE_MAX_LEVELS - 1))
		return -EINVAL;

	/* Grab a pointer to the next level */
	//��ȡ��һ����ָ��
	pte = *ptep;
	if (!pte) {
		//ҳ���������Ϊ�գ�����һ���ڴ�ռ�
		cptep = alloc_pages_exact(1UL << data->pg_shift,
					 GFP_ATOMIC | __GFP_ZERO);
		if (!cptep)
			return -ENOMEM;

		data->iop.cfg.tlb->flush_pgtable(cptep, 1UL << data->pg_shift,
						 cookie);
		pte = __pa(cptep) | ARM_LPAE_PTE_TYPE_TABLE;
		if (data->iop.cfg.quirks & IO_PGTABLE_QUIRK_ARM_NS)
			pte |= ARM_LPAE_PTE_NSTABLE;
		*ptep = pte;
		data->iop.cfg.tlb->flush_pgtable(ptep, sizeof(*ptep), cookie);
	} else {
		//ҳ��������ݲ�Ϊ��
		cptep = iopte_deref(pte, data);
	}

	/* Rinse, repeat */
	return __arm_lpae_map(data, iova, paddr, size, prot, lvl + 1, cptep);
}
//��prot�����Ȩ��ת����arm_lpae_iopte��
//arm_lpae_io_pgtable��iop.fmt ������ϵͳ��Ӳ���Ĳ�ͬ��ת����ʽҲ��ͬ
//����ת����Ȩ������ֵ
static arm_lpae_iopte arm_lpae_prot_to_pte(struct arm_lpae_io_pgtable *data,
					   int prot)
{
	arm_lpae_iopte pte;
	//
	if (data->iop.fmt == ARM_64_LPAE_S1 ||
	    data->iop.fmt == ARM_32_LPAE_S1) {
		pte = ARM_LPAE_PTE_AP_UNPRIV | ARM_LPAE_PTE_nG;//[6]��[11]λΪ1

		
		if (!(prot & IOMMU_WRITE) && (prot & IOMMU_READ))
			//���ֻ�ܶ�������д
			pte |= ARM_LPAE_PTE_AP_RDONLY;

		if (prot & IOMMU_CACHE)
			pte |= (ARM_LPAE_MAIR_ATTR_IDX_CACHE
				<< ARM_LPAE_PTE_ATTRINDX_SHIFT);
	} else {
		//����stage��ʱ��֧�֣��Ȳ�����
		pte = ARM_LPAE_PTE_HAP_FAULT;
		if (prot & IOMMU_READ)
			pte |= ARM_LPAE_PTE_HAP_READ;
		if (prot & IOMMU_WRITE)
			pte |= ARM_LPAE_PTE_HAP_WRITE;
		if (prot & IOMMU_CACHE)
			pte |= ARM_LPAE_PTE_MEMATTR_OIWB;
		else
			pte |= ARM_LPAE_PTE_MEMATTR_NC;
	}

	if (prot & IOMMU_NOEXEC)
		pte |= ARM_LPAE_PTE_XN;

	return pte;
}

//������map����
static int arm_lpae_map(struct io_pgtable_ops *ops, unsigned long iova,
			phys_addr_t paddr, size_t size, int iommu_prot)
{
	struct arm_lpae_io_pgtable *data = io_pgtable_ops_to_data(ops);
	arm_lpae_iopte *ptep = data->pgd;
	//��arm_lpae_io_pgtable�л�ÿ�ʼ��lvlֵ����ʾarm_lpae_io_pgtable
	//��¼���ǵڼ���ҳ��ġ�
	//���levels ��3����ʾ������ҳ����ô��ǰ���ǵ�1��
	//���levels ��2����ʾ�ж���ҳ����ô��ǰ���ǵ�2��
	//���levels ��1����ʾ��һ��ҳ����ô��ǰ���ǵ�3��
	int lvl = ARM_LPAE_START_LVL(data);//����level��ȥdata�е�levels
	arm_lpae_iopte prot;

	/* If no access, then nothing to do */
	if (!(iommu_prot & (IOMMU_READ | IOMMU_WRITE)))
		return 0;//û�ж���дȨ�ޣ�ֱ�ӷ���0
	//�õ�iommu_prot��Ȩ�����á�
	//ֻ��ϵͳ��Ӳ������ֵ����дֵ��DMA cache coherency��
	//ARM_LPAE_PTE_XN��ֵ
	//����[2]��[7:6]��[11],[54:53]
	prot = arm_lpae_prot_to_pte(data, iommu_prot);
	return __arm_lpae_map(data, iova, paddr, size, prot, lvl, ptep);
}

static void __arm_lpae_free_pgtable(struct arm_lpae_io_pgtable *data, int lvl,
				    arm_lpae_iopte *ptep)
{
	arm_lpae_iopte *start, *end;
	unsigned long table_size;

	/* Only leaf entries at the last level */
	if (lvl == ARM_LPAE_MAX_LEVELS - 1)
		return;

	if (lvl == ARM_LPAE_START_LVL(data))
		table_size = data->pgd_size;
	else
		table_size = 1UL << data->pg_shift;

	start = ptep;
	end = (void *)ptep + table_size;

	while (ptep != end) {
		arm_lpae_iopte pte = *ptep++;

		if (!pte || iopte_leaf(pte, lvl))
			continue;

		__arm_lpae_free_pgtable(data, lvl + 1, iopte_deref(pte, data));
	}

	free_pages_exact(start, table_size);
}

static void arm_lpae_free_pgtable(struct io_pgtable *iop)
{
	struct arm_lpae_io_pgtable *data = io_pgtable_to_data(iop);

	__arm_lpae_free_pgtable(data, ARM_LPAE_START_LVL(data), data->pgd);
	kfree(data);
}

static int arm_lpae_split_blk_unmap(struct arm_lpae_io_pgtable *data,
				    unsigned long iova, size_t size,
				    arm_lpae_iopte prot, int lvl,
				    arm_lpae_iopte *ptep, size_t blk_size)
{
	unsigned long blk_start, blk_end;
	phys_addr_t blk_paddr;
	arm_lpae_iopte table = 0;
	void *cookie = data->iop.cookie;
	const struct iommu_gather_ops *tlb = data->iop.cfg.tlb;

	blk_start = iova & ~(blk_size - 1);
	blk_end = blk_start + blk_size;
	blk_paddr = iopte_to_pfn(*ptep, data) << data->pg_shift;

	for (; blk_start < blk_end; blk_start += size, blk_paddr += size) {
		arm_lpae_iopte *tablep;

		/* Unmap! */
		if (blk_start == iova)
			continue;

		/* __arm_lpae_map expects a pointer to the start of the table */
		tablep = &table - ARM_LPAE_LVL_IDX(blk_start, lvl, data);
		if (__arm_lpae_map(data, blk_start, blk_paddr, size, prot, lvl,
				   tablep) < 0) {
			if (table) {
				/* Free the table we allocated */
				tablep = iopte_deref(table, data);
				__arm_lpae_free_pgtable(data, lvl + 1, tablep);
			}
			return 0; /* Bytes unmapped */
		}
	}

	*ptep = table;
	tlb->flush_pgtable(ptep, sizeof(*ptep), cookie);
	iova &= ~(blk_size - 1);
	tlb->tlb_add_flush(iova, blk_size, true, cookie);
	return size;
}
//ptep�����ʼ��ַ
static int __arm_lpae_unmap(struct arm_lpae_io_pgtable *data,
			    unsigned long iova, size_t size, int lvl,
			    arm_lpae_iopte *ptep)
{
	arm_lpae_iopte pte;
	const struct iommu_gather_ops *tlb = data->iop.cfg.tlb;
	void *cookie = data->iop.cookie;
	size_t blk_size = ARM_LPAE_BLOCK_SIZE(lvl, data);

	ptep += ARM_LPAE_LVL_IDX(iova, lvl, data);
	pte = *ptep;

	/* Something went horribly wrong and we ran out of page table */
	if (WARN_ON(!pte || (lvl == ARM_LPAE_MAX_LEVELS)))
		return 0;

	/* If the size matches this level, we're in the right place */
	if (size == blk_size) {
		*ptep = 0;
		tlb->flush_pgtable(ptep, sizeof(*ptep), cookie);

		if (!iopte_leaf(pte, lvl)) {
			/* Also flush any partial walks */
			tlb->tlb_add_flush(iova, size, false, cookie);
			tlb->tlb_sync(data->iop.cookie);
			ptep = iopte_deref(pte, data);
			__arm_lpae_free_pgtable(data, lvl + 1, ptep);
		} else {
			tlb->tlb_add_flush(iova, size, true, cookie);
		}

		return size;
	} else if (iopte_leaf(pte, lvl)) {
		/*
		 * Insert a table at the next level to map the old region,
		 * minus the part we want to unmap
		 */
		return arm_lpae_split_blk_unmap(data, iova, size,
						iopte_prot(pte), lvl, ptep,
						blk_size);
	}

	/* Keep on walkin' */
	ptep = iopte_deref(pte, data);
	return __arm_lpae_unmap(data, iova, size, lvl + 1, ptep);
}

static int arm_lpae_unmap(struct io_pgtable_ops *ops, unsigned long iova,
			  size_t size)
{
	size_t unmapped;
	struct arm_lpae_io_pgtable *data = io_pgtable_ops_to_data(ops);
	struct io_pgtable *iop = &data->iop;
	arm_lpae_iopte *ptep = data->pgd;
	int lvl = ARM_LPAE_START_LVL(data);

	unmapped = __arm_lpae_unmap(data, iova, size, lvl, ptep);
	if (unmapped)
		iop->cfg.tlb->tlb_sync(iop->cookie);

	return unmapped;
}

static phys_addr_t arm_lpae_iova_to_phys(struct io_pgtable_ops *ops,
					 unsigned long iova)
{
	struct arm_lpae_io_pgtable *data = io_pgtable_ops_to_data(ops);
	arm_lpae_iopte pte, *ptep = data->pgd;
	int lvl = ARM_LPAE_START_LVL(data);

	do {
		/* Valid IOPTE pointer? */
		if (!ptep)
			return 0;//ҳ��û�У�����ʲô��

		/* Grab the IOPTE we're interested in */
		pte = *(ptep + ARM_LPAE_LVL_IDX(iova, lvl, data));

		/* Valid entry? */
		if (!pte)
			return 0;//ҳ��������Ϊ�ա�

		/* Leaf entry? */
		if (iopte_leaf(pte,lvl))
			goto found_translation;

		/* Take it to the next level */
		ptep = iopte_deref(pte, data);
	} while (++lvl < ARM_LPAE_MAX_LEVELS);

	/* Ran out of page tables to walk */
	return 0;

found_translation:
	iova &= ((1 << data->pg_shift) - 1);
	return ((phys_addr_t)iopte_to_pfn(pte,data) << data->pg_shift) | iova;
}

//����ҳ�Ĵ�С������cfg->pgsize_bitmap��ֵ
static void arm_lpae_restrict_pgsizes(struct io_pgtable_cfg *cfg)
{
	unsigned long granule;//������С

	/*
	 * We need to restrict the supported page sizes to match the
	 * translation regime for a particular granule. Aim to match
	 * the CPU page size if possible, otherwise prefer smaller sizes.
	 * While we're at it, restrict the block sizes to match the
	 * chosen granule.
	 */
	if (cfg->pgsize_bitmap & PAGE_SIZE)
		granule = PAGE_SIZE;//4K/8k����ϵͳҳ��Сһ��
	else if (cfg->pgsize_bitmap & ~PAGE_MASK)
		//��ߵ���λ
		granule = 1UL << __fls(cfg->pgsize_bitmap & ~PAGE_MASK);
	else if (cfg->pgsize_bitmap & PAGE_MASK)
		//����͵���λ
		granule = 1UL << __ffs(cfg->pgsize_bitmap & PAGE_MASK);
	else
		granule = 0;

	switch (granule) {
	case SZ_4K:
		cfg->pgsize_bitmap &= (SZ_4K | SZ_2M | SZ_1G);
		break;
	case SZ_16K:
		cfg->pgsize_bitmap &= (SZ_16K | SZ_32M);
		break;
	case SZ_64K:
		cfg->pgsize_bitmap &= (SZ_64K | SZ_512M);
		break;
	default:
		cfg->pgsize_bitmap = 0;
	}
}

static struct arm_lpae_io_pgtable *
arm_lpae_alloc_pgtable(struct io_pgtable_cfg *cfg)
{
	unsigned long va_bits, pgd_bits;
	struct arm_lpae_io_pgtable *data;
	//cfg ����
	//	pgtbl_cfg = (struct io_pgtable_cfg) {
	//		.pgsize_bitmap	= arm_smmu_ops.pgsize_bitmap,
	//		.ias		= ias,
	//		.oas		= oas,
	//		.tlb		= &arm_smmu_gather_ops,
	//  };
	//����ϵͳҳ�Ĵ�С����cfg->pgsize_bitmap��ֵ
	arm_lpae_restrict_pgsizes(cfg);

	if (!(cfg->pgsize_bitmap & (SZ_4K | SZ_16K | SZ_64K)))
		return NULL;//�������4k/16k/64/�е�һ�֣�ֱ�ӷ���

	if (cfg->ias > ARM_LPAE_MAX_ADDR_BITS)
		return NULL;

	if (cfg->oas > ARM_LPAE_MAX_ADDR_BITS)
		return NULL;

	data = kmalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return NULL;

	data->pg_shift = __ffs(cfg->pgsize_bitmap);
	data->bits_per_level = data->pg_shift - ilog2(sizeof(arm_lpae_iopte));

	//�����ַҳ�ŵķ�Χ
	va_bits = cfg->ias - data->pg_shift;
	data->levels = DIV_ROUND_UP(va_bits, data->bits_per_level);

	/* Calculate the actual size of our pgd (without concatenation) */
	pgd_bits = va_bits - (data->bits_per_level * (data->levels - 1));
	data->pgd_size = 1UL << (pgd_bits + ilog2(sizeof(arm_lpae_iopte)));

	data->iop.ops = (struct io_pgtable_ops) {
		.map		= arm_lpae_map,
		.unmap		= arm_lpae_unmap,
		.iova_to_phys	= arm_lpae_iova_to_phys,
	};

	return data;
}

static struct io_pgtable *
arm_64_lpae_alloc_pgtable_s1(struct io_pgtable_cfg *cfg, void *cookie)
{
	u64 reg;
	//����iop�ˣ�iop������io_pgtable_ops
	struct arm_lpae_io_pgtable *data = arm_lpae_alloc_pgtable(cfg);

	if (!data)
		return NULL;

	/* TCR */
	//SMMU_CBn_TCR�Ĵ�����ֵ����SMMU_CBn_CBA2R.VA64 is 1
	//�������
	reg = (ARM_LPAE_TCR_SH_IS << ARM_LPAE_TCR_SH0_SHIFT) |
	      (ARM_LPAE_TCR_RGN_WBWA << ARM_LPAE_TCR_IRGN0_SHIFT) |
	      (ARM_LPAE_TCR_RGN_WBWA << ARM_LPAE_TCR_ORGN0_SHIFT);

	switch (1 << data->pg_shift) {//[15-14]
	case SZ_4K:
		reg |= ARM_LPAE_TCR_TG0_4K;
		break;
	case SZ_16K:
		reg |= ARM_LPAE_TCR_TG0_16K;
		break;
	case SZ_64K:
		reg |= ARM_LPAE_TCR_TG0_64K;
		break;
	}

	//�����ַ��λ��
	switch (cfg->oas) {
	case 32:
		reg |= (ARM_LPAE_TCR_PS_32_BIT << ARM_LPAE_TCR_IPS_SHIFT);
		break;
	case 36:
		reg |= (ARM_LPAE_TCR_PS_36_BIT << ARM_LPAE_TCR_IPS_SHIFT);
		break;
	case 40:
		reg |= (ARM_LPAE_TCR_PS_40_BIT << ARM_LPAE_TCR_IPS_SHIFT);
		break;
	case 42:
		reg |= (ARM_LPAE_TCR_PS_42_BIT << ARM_LPAE_TCR_IPS_SHIFT);
		break;
	case 44:
		reg |= (ARM_LPAE_TCR_PS_44_BIT << ARM_LPAE_TCR_IPS_SHIFT);
		break;
	case 48:
		reg |= (ARM_LPAE_TCR_PS_48_BIT << ARM_LPAE_TCR_IPS_SHIFT);
		break;
	default:
		goto out_free_data;
	}

	//�����ַ��λ��
	reg |= (64ULL - cfg->ias) << ARM_LPAE_TCR_T0SZ_SHIFT;//[5-0]

	/* Disable speculative walks through TTBR1 */
	reg |= ARM_LPAE_TCR_EPD1;//[23]
	cfg->arm_lpae_s1_cfg.tcr = reg;

	/* MAIRs */
	//[7-0] ARM_LPAE_MAIR_ATTR_NC
	//[15-8] ARM_LPAE_MAIR_ATTR_WBRWA
	//[23-16] ARM_LPAE_MAIR_ATTR_DEVICE
	reg = (ARM_LPAE_MAIR_ATTR_NC
	       << ARM_LPAE_MAIR_ATTR_SHIFT(ARM_LPAE_MAIR_ATTR_IDX_NC)) |
	      (ARM_LPAE_MAIR_ATTR_WBRWA
	       << ARM_LPAE_MAIR_ATTR_SHIFT(ARM_LPAE_MAIR_ATTR_IDX_CACHE)) |
	      (ARM_LPAE_MAIR_ATTR_DEVICE
	       << ARM_LPAE_MAIR_ATTR_SHIFT(ARM_LPAE_MAIR_ATTR_IDX_DEV));

	cfg->arm_lpae_s1_cfg.mair[0] = reg;
	cfg->arm_lpae_s1_cfg.mair[1] = 0;

	/* Looking good; allocate a pgd */
	data->pgd = alloc_pages_exact(data->pgd_size, GFP_KERNEL | __GFP_ZERO);
	if (!data->pgd)
		goto out_free_data;

	cfg->tlb->flush_pgtable(data->pgd, data->pgd_size, cookie);

	/* TTBRs */
	cfg->arm_lpae_s1_cfg.ttbr[0] = virt_to_phys(data->pgd);
	cfg->arm_lpae_s1_cfg.ttbr[1] = 0;
	return &data->iop;

out_free_data:
	kfree(data);
	return NULL;
}

static struct io_pgtable *
arm_32_lpae_alloc_pgtable_s1(struct io_pgtable_cfg *cfg, void *cookie)
{
	struct io_pgtable *iop;

	if (cfg->ias > 32 || cfg->oas > 40)
		return NULL;

	cfg->pgsize_bitmap &= (SZ_4K | SZ_2M | SZ_1G);
	iop = arm_64_lpae_alloc_pgtable_s1(cfg, cookie);
	if (iop) {
		cfg->arm_lpae_s1_cfg.tcr |= ARM_32_LPAE_TCR_EAE;
		cfg->arm_lpae_s1_cfg.tcr &= 0xffffffff;
	}

	return iop;
}


struct io_pgtable_init_fns io_pgtable_arm_64_lpae_s1_init_fns = {
	.alloc	= arm_64_lpae_alloc_pgtable_s1,
	.free	= arm_lpae_free_pgtable,
};


struct io_pgtable_init_fns io_pgtable_arm_32_lpae_s1_init_fns = {
	.alloc	= arm_32_lpae_alloc_pgtable_s1,
	.free	= arm_lpae_free_pgtable,
};


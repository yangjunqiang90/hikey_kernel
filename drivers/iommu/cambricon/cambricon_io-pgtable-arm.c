
#define pr_fmt(fmt)	"cambricon-arm-lpae cambricomio-pgtable: " fmt

#include <linux/iommu.h>
#include <linux/kernel.h>
#include <linux/sizes.h>
#include <linux/slab.h>
#include <linux/types.h>

#include "cambricon_io-pgtable.h"

//地址最大的宽度
#define ARM_LPAE_MAX_ADDR_BITS		48
#define ARM_LPAE_S2_MAX_CONCAT_PAGES	16
//最多几级页表查找
#define ARM_LPAE_MAX_LEVELS		4

/* Struct accessors */
//根据结构体的成员地址，找到结构体的一些宏
//data就是arm_lpae_io_pgtable。pgtable就是io_pgtable

//根据io_pgtable得到arm_lpae_io_pgtable
#define io_pgtable_to_data(x)						\
	container_of((x), struct arm_lpae_io_pgtable, iop)

//根据io_pgtable_ops找到io_pgtable
#define io_pgtable_ops_to_pgtable(x)					\
	container_of((x), struct io_pgtable, ops)

//根据io_pgtable_ops找到arm_lpae_io_pgtable
#define io_pgtable_ops_to_data(x)					\
	io_pgtable_to_data(io_pgtable_ops_to_pgtable(x))



/*
 * For consistency with the architecture, we always consider
 * ARM_LPAE_MAX_LEVELS levels, with the walk starting at level n >=0
 */
 //根据架构的一致性，我们总是考虑ARM_LPAE_MAX_LEVELS
 //最大的level减去d中的level
//leves的开始编号；[0，3]，高地址对应的level 0，低地址对应的level 3
#define ARM_LPAE_START_LVL(d)		(ARM_LPAE_MAX_LEVELS - (d)->levels)

/*
 * Calculate the right shift amount to get to the portion describing level l
 * in a virtual address mapped by the pagetable in d.
 */
 //计算右移的数量，
 //l 是lvl
 //d 是arm_lpae_io_pgtable
 
 //每级level开始的index
#define ARM_LPAE_LVL_SHIFT(l,d)						\
	((((d)->levels - ((l) - ARM_LPAE_START_LVL(d) + 1))		\
	  * (d)->bits_per_level) + (d)->pg_shift)

//顶级页表有几页，向上对齐，所以最少是一页
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
//根据d，计算在level l上映射块的大小
//一个表项指向的内容大小

//l 是level
//d 是arm_lpae_io_pgtable
//第l级页表一个表项的大小
#define ARM_LPAE_BLOCK_SIZE(l,d)					\
	(1 << (ilog2(sizeof(arm_lpae_iopte)) +				\
		((ARM_LPAE_MAX_LEVELS - (l)) * (d)->bits_per_level)))

/* Page table bits */
//最后两位是该表项所指向地址的类型，
//空白，最后一级的页，块，指向下一级页表
#define ARM_LPAE_PTE_TYPE_SHIFT		0
#define ARM_LPAE_PTE_TYPE_MASK		0x3

//pte最后两位的值
//不是最后一级页表
#define ARM_LPAE_PTE_TYPE_BLOCK		1
#define ARM_LPAE_PTE_TYPE_TABLE		3
//表示是最后一级页表，指向的内容不会是页表了。
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
//左移两位是dma cache corehent
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


//SMMU_CBn_TCR寄存器
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

//保留pte的低两位
#define iopte_type(pte,l)					\
	(((pte) >> ARM_LPAE_PTE_TYPE_SHIFT) & ARM_LPAE_PTE_TYPE_MASK)

#define iopte_prot(pte)	((pte) & ARM_LPAE_PTE_ATTR_MASK)

//如果是最后一级页表，判断pte的内容是否是页
//如果不是最后一级页表，判断pte的低两位是否是块
#define iopte_leaf(pte,l)					\
	(l == (ARM_LPAE_MAX_LEVELS - 1) ?			\
		(iopte_type(pte,l) == ARM_LPAE_PTE_TYPE_PAGE) :	\
		(iopte_type(pte,l) == ARM_LPAE_PTE_TYPE_BLOCK))

#define iopte_to_pfn(pte,d)					\
	(((pte) & ((1ULL << ARM_LPAE_MAX_ADDR_BITS) - 1)) >> (d)->pg_shift)

#define pfn_to_iopte(pfn,d)					\
	(((pfn) << (d)->pg_shift) & ((1ULL << ARM_LPAE_MAX_ADDR_BITS) - 1))

//lpae 大物理地址扩展
struct arm_lpae_io_pgtable {
	struct io_pgtable	iop;

	//最多有几级页表
	int			levels;
	//顶级页表的大小
	//如果输入地址是45位，系统页是4k，
	//45-12=33，33与每级9位向上取整是4
	//那么顶级的位数就是33-9*3=5位，
	//索引项有5位，每项有三位。因此5+3=8
	//所以顶级的大小就是256字节
	//void			*pgd指向内存的大小
	size_t			pgd_size;
	
	//可以得到一页有多大4k/16k/64k
	//系统页的大小，如果是12位，那么level就是9
	//如果是13位，那么level就是10位
	unsigned long		pg_shift;
	//每级level用多少位表示
	unsigned long		bits_per_level;

	void			*pgd;
};

//就是pte，页表项的内容
typedef u64 arm_lpae_iopte;

static bool selftest_running = false;

static int __arm_lpae_unmap(struct arm_lpae_io_pgtable *data,
			    unsigned long iova, size_t size, int lvl,
			    arm_lpae_iopte *ptep);

//ptep 就是对应的页表项
static int arm_lpae_init_pte(struct arm_lpae_io_pgtable *data,
			     unsigned long iova, phys_addr_t paddr,
			     arm_lpae_iopte prot, int lvl,
			     arm_lpae_iopte *ptep)
{
	arm_lpae_iopte pte = prot;

	//如果是最后一级页表，判断pte的内容是否是页
	//如果不是最后一级页表，判断pte的内容是否是块
	//这里是最后一级页表，但是*ptep的内容是0，新的
	//表项，还没有往里面写内容呢。
	if (iopte_leaf(*ptep, lvl)) {
		//如果在最后一级写入，并且该表项有值了，这个值肯定是page
		//如果不是最后一级写入，并且该表项有值了，这个值肯定是block
		//只要这个表项有值了，就返回错误码。
		/* We require an unmap first */
		WARN_ON(!selftest_running);
		return -EEXIST;//返回错误码，是已经存在了。
	} else if (iopte_type(*ptep, lvl) == ARM_LPAE_PTE_TYPE_TABLE) {
		//
		/*
		 * We need to unmap and free the old table before
		 * overwriting it with a block entry.
		 */
		 //在重新写入之前，释放以前的旧的表项。
		arm_lpae_iopte *tblp;
		size_t sz = ARM_LPAE_BLOCK_SIZE(lvl, data);

		tblp = ptep - ARM_LPAE_LVL_IDX(iova, lvl, data);
		if (WARN_ON(__arm_lpae_unmap(data, iova, sz, lvl, tblp) != sz))
			return -EINVAL;
	}

	if (data->iop.cfg.quirks & IO_PGTABLE_QUIRK_ARM_NS)
		pte |= ARM_LPAE_PTE_NS;

	if (lvl == ARM_LPAE_MAX_LEVELS - 1)//如果是最后一级页表
		pte |= ARM_LPAE_PTE_TYPE_PAGE;
	else
		pte |= ARM_LPAE_PTE_TYPE_BLOCK;

	pte |= ARM_LPAE_PTE_AF | ARM_LPAE_PTE_SH_IS;
	pte |= pfn_to_iopte(paddr >> data->pg_shift, data);

	*ptep = pte;
	data->iop.cfg.tlb->flush_pgtable(ptep, sizeof(*ptep), data->iop.cookie);
	return 0;
}

//ptep 开始级的页表
//lvl  当前的lvl ，最小为1，最大为3
// arm_lpae_iopte *ptep = data->pgd
//size 映射的大小，需要和某一级表项的大小相等
static int __arm_lpae_map(struct arm_lpae_io_pgtable *data, unsigned long iova,
			  phys_addr_t paddr, size_t size, arm_lpae_iopte prot,
			  int lvl, arm_lpae_iopte *ptep)
{
	arm_lpae_iopte *cptep, pte;
	void *cookie = data->iop.cookie;
	//第lvl级，一个表项有多大。
	size_t block_size = ARM_LPAE_BLOCK_SIZE(lvl, data);

	/* Find our entry at the current level */
	//在当前级里面找到对应的值
	//iova右移一定的位数，保留低九位
	//data中的levels为3，这个lvl就是1
	ptep += ARM_LPAE_LVL_IDX(iova, lvl, data);

	/* If we can install a leaf entry at this level, then do so */
	//size 需要是4k/2M/1G
	if (size == block_size && (size & data->iop.cfg.pgsize_bitmap))
		return arm_lpae_init_pte(data, iova, paddr, prot, lvl, ptep);

	/* We can't allocate tables at the final level */
	//最后一页的时候不能在申请页表了
	if (WARN_ON(lvl >= ARM_LPAE_MAX_LEVELS - 1))
		return -EINVAL;

	/* Grab a pointer to the next level */
	//获取下一级的指针
	pte = *ptep;
	if (!pte) {
		//页表项的内容为空，申请一个内存空间
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
		//页表项的内容不为空
		cptep = iopte_deref(pte, data);
	}

	/* Rinse, repeat */
	return __arm_lpae_map(data, iova, paddr, size, prot, lvl + 1, cptep);
}
//将prot代表的权限转换成arm_lpae_iopte中
//arm_lpae_io_pgtable中iop.fmt 描述的系统和硬件的不同，转换方式也不同
//返回转换的权限设置值
static arm_lpae_iopte arm_lpae_prot_to_pte(struct arm_lpae_io_pgtable *data,
					   int prot)
{
	arm_lpae_iopte pte;
	//
	if (data->iop.fmt == ARM_64_LPAE_S1 ||
	    data->iop.fmt == ARM_32_LPAE_S1) {
		pte = ARM_LPAE_PTE_AP_UNPRIV | ARM_LPAE_PTE_nG;//[6]和[11]位为1

		
		if (!(prot & IOMMU_WRITE) && (prot & IOMMU_READ))
			//如果只能读，不能写
			pte |= ARM_LPAE_PTE_AP_RDONLY;

		if (prot & IOMMU_CACHE)
			pte |= (ARM_LPAE_MAIR_ATTR_IDX_CACHE
				<< ARM_LPAE_PTE_ATTRINDX_SHIFT);
	} else {
		//二级stage暂时不支持，先不分析
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

//真正的map函数
static int arm_lpae_map(struct io_pgtable_ops *ops, unsigned long iova,
			phys_addr_t paddr, size_t size, int iommu_prot)
{
	struct arm_lpae_io_pgtable *data = io_pgtable_ops_to_data(ops);
	arm_lpae_iopte *ptep = data->pgd;
	//从arm_lpae_io_pgtable中获得开始的lvl值，表示arm_lpae_io_pgtable
	//记录的是第几级页表的。
	//如果levels 是3，表示有三级页表，那么当前就是第1级
	//如果levels 是2，表示有二级页表，那么当前就是第2级
	//如果levels 是1，表示有一级页表，那么当前就是第3级
	int lvl = ARM_LPAE_START_LVL(data);//最大的level减去data中的levels
	arm_lpae_iopte prot;

	/* If no access, then nothing to do */
	if (!(iommu_prot & (IOMMU_READ | IOMMU_WRITE)))
		return 0;//没有读和写权限，直接返回0
	//得到iommu_prot的权限设置。
	//只有系统和硬件属性值，读写值，DMA cache coherency，
	//ARM_LPAE_PTE_XN的值
	//设置[2]，[7:6]，[11],[54:53]
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
//ptep表的起始地址
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
			return 0;//页表都没有，还玩什么。

		/* Grab the IOPTE we're interested in */
		pte = *(ptep + ARM_LPAE_LVL_IDX(iova, lvl, data));

		/* Valid entry? */
		if (!pte)
			return 0;//页表项内容为空。

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

//限制页的大小，设置cfg->pgsize_bitmap的值
static void arm_lpae_restrict_pgsizes(struct io_pgtable_cfg *cfg)
{
	unsigned long granule;//颗粒大小

	/*
	 * We need to restrict the supported page sizes to match the
	 * translation regime for a particular granule. Aim to match
	 * the CPU page size if possible, otherwise prefer smaller sizes.
	 * While we're at it, restrict the block sizes to match the
	 * chosen granule.
	 */
	if (cfg->pgsize_bitmap & PAGE_SIZE)
		granule = PAGE_SIZE;//4K/8k，和系统页大小一样
	else if (cfg->pgsize_bitmap & ~PAGE_MASK)
		//最高的置位
		granule = 1UL << __fls(cfg->pgsize_bitmap & ~PAGE_MASK);
	else if (cfg->pgsize_bitmap & PAGE_MASK)
		//找最低的置位
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
	//cfg 就是
	//	pgtbl_cfg = (struct io_pgtable_cfg) {
	//		.pgsize_bitmap	= arm_smmu_ops.pgsize_bitmap,
	//		.ias		= ias,
	//		.oas		= oas,
	//		.tlb		= &arm_smmu_gather_ops,
	//  };
	//根据系统页的大小设置cfg->pgsize_bitmap的值
	arm_lpae_restrict_pgsizes(cfg);

	if (!(cfg->pgsize_bitmap & (SZ_4K | SZ_16K | SZ_64K)))
		return NULL;//如果不是4k/16k/64/中的一种，直接返回

	if (cfg->ias > ARM_LPAE_MAX_ADDR_BITS)
		return NULL;

	if (cfg->oas > ARM_LPAE_MAX_ADDR_BITS)
		return NULL;

	data = kmalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return NULL;

	data->pg_shift = __ffs(cfg->pgsize_bitmap);
	data->bits_per_level = data->pg_shift - ilog2(sizeof(arm_lpae_iopte));

	//输入地址页号的范围
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
	//设置iop了，iop里面有io_pgtable_ops
	struct arm_lpae_io_pgtable *data = arm_lpae_alloc_pgtable(cfg);

	if (!data)
		return NULL;

	/* TCR */
	//SMMU_CBn_TCR寄存器的值，在SMMU_CBn_CBA2R.VA64 is 1
	//的情况下
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

	//输出地址的位数
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

	//输入地址的位数
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


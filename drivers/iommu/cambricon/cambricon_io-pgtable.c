#include <linux/bug.h>
#include <linux/kernel.h>
#include <linux/types.h>

#include "cambricon_io-pgtable.h"

extern struct io_pgtable_init_fns io_pgtable_arm_32_lpae_s1_init_fns;
extern struct io_pgtable_init_fns io_pgtable_arm_64_lpae_s1_init_fns;

//函数指针数组
static const struct io_pgtable_init_fns *
io_pgtable_init_table[IO_PGTABLE_NUM_FMTS] =
{
#ifdef CONFIG_IOMMU_IO_PGTABLE_LPAE
	[ARM_32_LPAE_S1] = &io_pgtable_arm_32_lpae_s1_init_fns,
	[ARM_64_LPAE_S1] = &io_pgtable_arm_64_lpae_s1_init_fns,//一级stage，64位系统
#endif
};
//fmt 是 数组下标
//cookie  是arm_smmu_domain
struct io_pgtable_ops *alloc_io_pgtable_ops(enum io_pgtable_fmt fmt,
					    struct io_pgtable_cfg *cfg,
					    void *cookie)
{
	struct io_pgtable *iop;
	const struct io_pgtable_init_fns *fns;

	if (fmt >= IO_PGTABLE_NUM_FMTS)
		return NULL;//参数不合法
//必须定义了，CONFIG_IOMMU_IO_PGTABLE_LPAE
	fns = io_pgtable_init_table[fmt];
	if (!fns)
		return NULL;

	iop = fns->alloc(cfg, cookie);
	if (!iop)
		return NULL;

	iop->fmt	= fmt;
	iop->cookie	= cookie;
	iop->cfg	= *cfg;

	return &iop->ops;
}

/*
 * It is the IOMMU driver's responsibility to ensure that the page table
 * is no longer accessible to the walker by this point.
 */
void free_io_pgtable_ops(struct io_pgtable_ops *ops)
{
	struct io_pgtable *iop;

	if (!ops)
		return;

	iop = container_of(ops, struct io_pgtable, ops);
	iop->cfg.tlb->tlb_flush_all(iop->cookie);
	io_pgtable_init_table[iop->fmt]->free(iop);
}

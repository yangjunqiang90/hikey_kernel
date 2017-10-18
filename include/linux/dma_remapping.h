#ifndef _DMA_REMAPPING_H
#define _DMA_REMAPPING_H

/*
 * VT-d hardware uses 4KiB page size regardless of host page size.
 */
 //VT-d永远是4k对其的，无论主机页的大小是多少。
 //也是iommu 的page shift
 //iommu支持的页大小
#define VTD_PAGE_SHIFT		(12)
#define VTD_PAGE_SIZE		(1UL << VTD_PAGE_SHIFT)
//低VTD_PAGE_SHIFT位都为0，其余为1
#define VTD_PAGE_MASK		(((u64)-1) << VTD_PAGE_SHIFT)
#define VTD_PAGE_ALIGN(addr)	(((addr) + VTD_PAGE_SIZE - 1) & VTD_PAGE_MASK)

//增加一级需要多少位
#define VTD_STRIDE_SHIFT        (9)
//低VTD_STRIDE_SHIFT位都为0，其余为1
#define VTD_STRIDE_MASK         (((u64)-1) << VTD_STRIDE_SHIFT)

//地址中的读位置为1
#define DMA_PTE_READ (1)

//地址中的写位置为1
#define DMA_PTE_WRITE (2)

//地址中的super page位置为1，表示是大页
#define DMA_PTE_LARGE_PAGE (1 << 7)

//地址中的snoop behavior位置为1
#define DMA_PTE_SNP (1 << 11)

#define CONTEXT_TT_MULTI_LEVEL	0
#define CONTEXT_TT_DEV_IOTLB	1
#define CONTEXT_TT_PASS_THROUGH 2

struct intel_iommu;
struct dmar_domain;
struct root_entry;


#ifdef CONFIG_INTEL_IOMMU
extern int iommu_calculate_agaw(struct intel_iommu *iommu);
extern int iommu_calculate_max_sagaw(struct intel_iommu *iommu);
extern int dmar_disabled;
extern int intel_iommu_enabled;
#else
static inline int iommu_calculate_agaw(struct intel_iommu *iommu)
{
	return 0;
}
static inline int iommu_calculate_max_sagaw(struct intel_iommu *iommu)
{
	return 0;
}
#define dmar_disabled	(1)
#define intel_iommu_enabled (0)
#endif


#endif

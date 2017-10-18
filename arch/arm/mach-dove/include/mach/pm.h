/*
 * arch/arm/mach-dove/include/mach/pm.h
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef __ASM_ARCH_PM_H
#define __ASM_ARCH_PM_H

#include <asm/errno.h>
#include <mach/irqs.h>

#define CLOCK_GATING_CONTROL	(DOVE_PMU_VIRT_BASE + 0x38)
#define  CLOCK_GATING_BIT_USB0		0
#define  CLOCK_GATING_BIT_USB1		1
#define  CLOCK_GATING_BIT_GBE		2
#define  CLOCK_GATING_BIT_SATA		3
#define  CLOCK_GATING_BIT_PCIE0		4
#define  CLOCK_GATING_BIT_PCIE1		5
#define  CLOCK_GATING_BIT_SDIO0		8
#define  CLOCK_GATING_BIT_SDIO1		9
#define  CLOCK_GATING_BIT_NAND		10
#define  CLOCK_GATING_BIT_CAMERA	11
#define  CLOCK_GATING_BIT_I2S0		12
#define  CLOCK_GATING_BIT_I2S1		13
#define  CLOCK_GATING_BIT_CRYPTO	15
#define  CLOCK_GATING_BIT_AC97		21
#define  CLOCK_GATING_BIT_PDMA		22
#define  CLOCK_GATING_BIT_XOR0		23
#define  CLOCK_GATING_BIT_XOR1		24
#define  CLOCK_GATING_BIT_GIGA_PHY	30
#define  CLOCK_GATING_USB0_MASK		(1 << CLOCK_GATING_BIT_USB0)
#define  CLOCK_GATING_USB1_MASK		(1 << CLOCK_GATING_BIT_USB1)
#define  CLOCK_GATING_GBE_MASK		(1 << CLOCK_GATING_BIT_GBE)
#define  CLOCK_GATING_SATA_MASK		(1 << CLOCK_GATING_BIT_SATA)
#define  CLOCK_GATING_PCIE0_MASK	(1 << CLOCK_GATING_BIT_PCIE0)
#define  CLOCK_GATING_PCIE1_MASK	(1 << CLOCK_GATING_BIT_PCIE1)
#define  CLOCK_GATING_SDIO0_MASK	(1 << CLOCK_GATING_BIT_SDIO0)
#define  CLOCK_GATING_SDIO1_MASK	(1 << CLOCK_GATING_BIT_SDIO1)
#define  CLOCK_GATING_NAND_MASK		(1 << CLOCK_GATING_BIT_NAND)
#define  CLOCK_GATING_CAMERA_MASK	(1 << CLOCK_GATING_BIT_CAMERA)
#define  CLOCK_GATING_I2S0_MASK		(1 << CLOCK_GATING_BIT_I2S0)
#define  CLOCK_GATING_I2S1_MASK		(1 << CLOCK_GATING_BIT_I2S1)
#define  CLOCK_GATING_CRYPTO_MASK	(1 << CLOCK_GATING_BIT_CRYPTO)
#define  CLOCK_GATING_AC97_MASK		(1 << CLOCK_GATING_BIT_AC97)
#define  CLOCK_GATING_PDMA_MASK		(1 << CLOCK_GATING_BIT_PDMA)
#define  CLOCK_GATING_XOR0_MASK		(1 << CLOCK_GATING_BIT_XOR0)
#define  CLOCK_GATING_XOR1_MASK		(1 << CLOCK_GATING_BIT_XOR1)
#define  CLOCK_GATING_GIGA_PHY_MASK	(1 << CLOCK_GATING_BIT_GIGA_PHY)

#define PMU_INTERRUPT_CAUSE	(DOVE_PMU_VIRT_BASE + 0x50)
#define PMU_INTERRUPT_MASK	(DOVE_PMU_VIRT_BASE + 0x54)

static inline int pmu_to_irq(int pin)
{
	if (pin < NR_PMU_IRQS)
		return pin + IRQ_DOVE_PMU_START;

	return -EINVAL;
}

static inline int irq_to_pmu(int irq)
{
	if (IRQ_DOVE_PMU_START <= irq && irq < NR_IRQS)
		return irq - IRQ_DOVE_PMU_START;

	return -EINVAL;
}

#endif

/*
 * DRA7xx Clock domains framework
 *
 * Copyright (C) 2009-2013 Texas Instruments, Inc.
 * Copyright (C) 2009-2011 Nokia Corporation
 *
 * Generated by code originally written by:
 * Abhijit Pagare (abhijitpagare@ti.com)
 * Benoit Cousson (b-cousson@ti.com)
 * Paul Walmsley (paul@pwsan.com)
 *
 * This file is automatically generated from the OMAP hardware databases.
 * We respectfully ask that any modifications to this file be coordinated
 * with the public linux-omap@vger.kernel.org mailing list and the
 * authors above to ensure that the autogeneration scripts are kept
 * up-to-date with the file contents.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/io.h>

#include "clockdomain.h"
#include "cm1_7xx.h"
#include "cm2_7xx.h"

#include "cm-regbits-7xx.h"
#include "prm7xx.h"
#include "prcm44xx.h"
#include "prcm_mpu7xx.h"

/* Static Dependencies for DRA7xx Clock Domains */

static struct clkdm_dep cam_wkup_sleep_deps[] = {
	{ .clkdm_name = "emif_clkdm" },
	{ NULL },
};

static struct clkdm_dep dma_wkup_sleep_deps[] = {
	{ .clkdm_name = "dss_clkdm" },
	{ .clkdm_name = "emif_clkdm" },
	{ .clkdm_name = "ipu_clkdm" },
	{ .clkdm_name = "ipu1_clkdm" },
	{ .clkdm_name = "ipu2_clkdm" },
	{ .clkdm_name = "iva_clkdm" },
	{ .clkdm_name = "l3init_clkdm" },
	{ .clkdm_name = "l4cfg_clkdm" },
	{ .clkdm_name = "l4per_clkdm" },
	{ .clkdm_name = "l4per2_clkdm" },
	{ .clkdm_name = "l4per3_clkdm" },
	{ .clkdm_name = "l4sec_clkdm" },
	{ .clkdm_name = "pcie_clkdm" },
	{ .clkdm_name = "wkupaon_clkdm" },
	{ NULL },
};

static struct clkdm_dep dsp1_wkup_sleep_deps[] = {
	{ .clkdm_name = "atl_clkdm" },
	{ .clkdm_name = "cam_clkdm" },
	{ .clkdm_name = "dsp2_clkdm" },
	{ .clkdm_name = "dss_clkdm" },
	{ .clkdm_name = "emif_clkdm" },
	{ .clkdm_name = "eve1_clkdm" },
	{ .clkdm_name = "eve2_clkdm" },
	{ .clkdm_name = "eve3_clkdm" },
	{ .clkdm_name = "eve4_clkdm" },
	{ .clkdm_name = "gmac_clkdm" },
	{ .clkdm_name = "gpu_clkdm" },
	{ .clkdm_name = "ipu_clkdm" },
	{ .clkdm_name = "ipu1_clkdm" },
	{ .clkdm_name = "ipu2_clkdm" },
	{ .clkdm_name = "iva_clkdm" },
	{ .clkdm_name = "l3init_clkdm" },
	{ .clkdm_name = "l4per_clkdm" },
	{ .clkdm_name = "l4per2_clkdm" },
	{ .clkdm_name = "l4per3_clkdm" },
	{ .clkdm_name = "l4sec_clkdm" },
	{ .clkdm_name = "pcie_clkdm" },
	{ .clkdm_name = "vpe_clkdm" },
	{ .clkdm_name = "wkupaon_clkdm" },
	{ NULL },
};

static struct clkdm_dep dsp2_wkup_sleep_deps[] = {
	{ .clkdm_name = "atl_clkdm" },
	{ .clkdm_name = "cam_clkdm" },
	{ .clkdm_name = "dsp1_clkdm" },
	{ .clkdm_name = "dss_clkdm" },
	{ .clkdm_name = "emif_clkdm" },
	{ .clkdm_name = "eve1_clkdm" },
	{ .clkdm_name = "eve2_clkdm" },
	{ .clkdm_name = "eve3_clkdm" },
	{ .clkdm_name = "eve4_clkdm" },
	{ .clkdm_name = "gmac_clkdm" },
	{ .clkdm_name = "gpu_clkdm" },
	{ .clkdm_name = "ipu_clkdm" },
	{ .clkdm_name = "ipu1_clkdm" },
	{ .clkdm_name = "ipu2_clkdm" },
	{ .clkdm_name = "iva_clkdm" },
	{ .clkdm_name = "l3init_clkdm" },
	{ .clkdm_name = "l4per_clkdm" },
	{ .clkdm_name = "l4per2_clkdm" },
	{ .clkdm_name = "l4per3_clkdm" },
	{ .clkdm_name = "l4sec_clkdm" },
	{ .clkdm_name = "pcie_clkdm" },
	{ .clkdm_name = "vpe_clkdm" },
	{ .clkdm_name = "wkupaon_clkdm" },
	{ NULL },
};

static struct clkdm_dep dss_wkup_sleep_deps[] = {
	{ .clkdm_name = "emif_clkdm" },
	{ .clkdm_name = "iva_clkdm" },
	{ NULL },
};

static struct clkdm_dep eve1_wkup_sleep_deps[] = {
	{ .clkdm_name = "emif_clkdm" },
	{ .clkdm_name = "eve2_clkdm" },
	{ .clkdm_name = "eve3_clkdm" },
	{ .clkdm_name = "eve4_clkdm" },
	{ .clkdm_name = "iva_clkdm" },
	{ NULL },
};

static struct clkdm_dep eve2_wkup_sleep_deps[] = {
	{ .clkdm_name = "emif_clkdm" },
	{ .clkdm_name = "eve1_clkdm" },
	{ .clkdm_name = "eve3_clkdm" },
	{ .clkdm_name = "eve4_clkdm" },
	{ .clkdm_name = "iva_clkdm" },
	{ NULL },
};

static struct clkdm_dep eve3_wkup_sleep_deps[] = {
	{ .clkdm_name = "emif_clkdm" },
	{ .clkdm_name = "eve1_clkdm" },
	{ .clkdm_name = "eve2_clkdm" },
	{ .clkdm_name = "eve4_clkdm" },
	{ .clkdm_name = "iva_clkdm" },
	{ NULL },
};

static struct clkdm_dep eve4_wkup_sleep_deps[] = {
	{ .clkdm_name = "emif_clkdm" },
	{ .clkdm_name = "eve1_clkdm" },
	{ .clkdm_name = "eve2_clkdm" },
	{ .clkdm_name = "eve3_clkdm" },
	{ .clkdm_name = "iva_clkdm" },
	{ NULL },
};

static struct clkdm_dep gmac_wkup_sleep_deps[] = {
	{ .clkdm_name = "emif_clkdm" },
	{ .clkdm_name = "l4per2_clkdm" },
	{ NULL },
};

static struct clkdm_dep gpu_wkup_sleep_deps[] = {
	{ .clkdm_name = "emif_clkdm" },
	{ .clkdm_name = "iva_clkdm" },
	{ NULL },
};

static struct clkdm_dep ipu1_wkup_sleep_deps[] = {
	{ .clkdm_name = "atl_clkdm" },
	{ .clkdm_name = "dsp1_clkdm" },
	{ .clkdm_name = "dsp2_clkdm" },
	{ .clkdm_name = "dss_clkdm" },
	{ .clkdm_name = "emif_clkdm" },
	{ .clkdm_name = "eve1_clkdm" },
	{ .clkdm_name = "eve2_clkdm" },
	{ .clkdm_name = "eve3_clkdm" },
	{ .clkdm_name = "eve4_clkdm" },
	{ .clkdm_name = "gmac_clkdm" },
	{ .clkdm_name = "gpu_clkdm" },
	{ .clkdm_name = "ipu_clkdm" },
	{ .clkdm_name = "ipu2_clkdm" },
	{ .clkdm_name = "iva_clkdm" },
	{ .clkdm_name = "l3init_clkdm" },
	{ .clkdm_name = "l3main1_clkdm" },
	{ .clkdm_name = "l4cfg_clkdm" },
	{ .clkdm_name = "l4per_clkdm" },
	{ .clkdm_name = "l4per2_clkdm" },
	{ .clkdm_name = "l4per3_clkdm" },
	{ .clkdm_name = "l4sec_clkdm" },
	{ .clkdm_name = "pcie_clkdm" },
	{ .clkdm_name = "vpe_clkdm" },
	{ .clkdm_name = "wkupaon_clkdm" },
	{ NULL },
};

static struct clkdm_dep ipu2_wkup_sleep_deps[] = {
	{ .clkdm_name = "atl_clkdm" },
	{ .clkdm_name = "dsp1_clkdm" },
	{ .clkdm_name = "dsp2_clkdm" },
	{ .clkdm_name = "dss_clkdm" },
	{ .clkdm_name = "emif_clkdm" },
	{ .clkdm_name = "eve1_clkdm" },
	{ .clkdm_name = "eve2_clkdm" },
	{ .clkdm_name = "eve3_clkdm" },
	{ .clkdm_name = "eve4_clkdm" },
	{ .clkdm_name = "gmac_clkdm" },
	{ .clkdm_name = "gpu_clkdm" },
	{ .clkdm_name = "ipu_clkdm" },
	{ .clkdm_name = "ipu1_clkdm" },
	{ .clkdm_name = "iva_clkdm" },
	{ .clkdm_name = "l3init_clkdm" },
	{ .clkdm_name = "l3main1_clkdm" },
	{ .clkdm_name = "l4cfg_clkdm" },
	{ .clkdm_name = "l4per_clkdm" },
	{ .clkdm_name = "l4per2_clkdm" },
	{ .clkdm_name = "l4per3_clkdm" },
	{ .clkdm_name = "l4sec_clkdm" },
	{ .clkdm_name = "pcie_clkdm" },
	{ .clkdm_name = "vpe_clkdm" },
	{ .clkdm_name = "wkupaon_clkdm" },
	{ NULL },
};

static struct clkdm_dep iva_wkup_sleep_deps[] = {
	{ .clkdm_name = "emif_clkdm" },
	{ NULL },
};

static struct clkdm_dep l3init_wkup_sleep_deps[] = {
	{ .clkdm_name = "emif_clkdm" },
	{ .clkdm_name = "iva_clkdm" },
	{ .clkdm_name = "l4cfg_clkdm" },
	{ .clkdm_name = "l4per_clkdm" },
	{ .clkdm_name = "l4per3_clkdm" },
	{ .clkdm_name = "l4sec_clkdm" },
	{ .clkdm_name = "wkupaon_clkdm" },
	{ NULL },
};

static struct clkdm_dep l4per2_wkup_sleep_deps[] = {
	{ .clkdm_name = "dsp1_clkdm" },
	{ .clkdm_name = "dsp2_clkdm" },
	{ .clkdm_name = "ipu1_clkdm" },
	{ .clkdm_name = "ipu2_clkdm" },
	{ NULL },
};

static struct clkdm_dep l4sec_wkup_sleep_deps[] = {
	{ .clkdm_name = "emif_clkdm" },
	{ .clkdm_name = "l4per_clkdm" },
	{ NULL },
};

static struct clkdm_dep mpu_wkup_sleep_deps[] = {
	{ .clkdm_name = "cam_clkdm" },
	{ .clkdm_name = "dsp1_clkdm" },
	{ .clkdm_name = "dsp2_clkdm" },
	{ .clkdm_name = "dss_clkdm" },
	{ .clkdm_name = "emif_clkdm" },
	{ .clkdm_name = "eve1_clkdm" },
	{ .clkdm_name = "eve2_clkdm" },
	{ .clkdm_name = "eve3_clkdm" },
	{ .clkdm_name = "eve4_clkdm" },
	{ .clkdm_name = "gmac_clkdm" },
	{ .clkdm_name = "gpu_clkdm" },
	{ .clkdm_name = "ipu_clkdm" },
	{ .clkdm_name = "ipu1_clkdm" },
	{ .clkdm_name = "ipu2_clkdm" },
	{ .clkdm_name = "iva_clkdm" },
	{ .clkdm_name = "l3init_clkdm" },
	{ .clkdm_name = "l3main1_clkdm" },
	{ .clkdm_name = "l4cfg_clkdm" },
	{ .clkdm_name = "l4per_clkdm" },
	{ .clkdm_name = "l4per2_clkdm" },
	{ .clkdm_name = "l4per3_clkdm" },
	{ .clkdm_name = "l4sec_clkdm" },
	{ .clkdm_name = "pcie_clkdm" },
	{ .clkdm_name = "vpe_clkdm" },
	{ .clkdm_name = "wkupaon_clkdm" },
	{ NULL },
};

static struct clkdm_dep pcie_wkup_sleep_deps[] = {
	{ .clkdm_name = "atl_clkdm" },
	{ .clkdm_name = "cam_clkdm" },
	{ .clkdm_name = "dsp1_clkdm" },
	{ .clkdm_name = "dsp2_clkdm" },
	{ .clkdm_name = "dss_clkdm" },
	{ .clkdm_name = "emif_clkdm" },
	{ .clkdm_name = "eve1_clkdm" },
	{ .clkdm_name = "eve2_clkdm" },
	{ .clkdm_name = "eve3_clkdm" },
	{ .clkdm_name = "eve4_clkdm" },
	{ .clkdm_name = "gmac_clkdm" },
	{ .clkdm_name = "gpu_clkdm" },
	{ .clkdm_name = "ipu_clkdm" },
	{ .clkdm_name = "ipu1_clkdm" },
	{ .clkdm_name = "iva_clkdm" },
	{ .clkdm_name = "l3init_clkdm" },
	{ .clkdm_name = "l4cfg_clkdm" },
	{ .clkdm_name = "l4per_clkdm" },
	{ .clkdm_name = "l4per2_clkdm" },
	{ .clkdm_name = "l4per3_clkdm" },
	{ .clkdm_name = "l4sec_clkdm" },
	{ .clkdm_name = "vpe_clkdm" },
	{ NULL },
};

static struct clkdm_dep vpe_wkup_sleep_deps[] = {
	{ .clkdm_name = "emif_clkdm" },
	{ .clkdm_name = "l4per3_clkdm" },
	{ NULL },
};

static struct clockdomain l4per3_7xx_clkdm = {
	.name		  = "l4per3_clkdm",
	.pwrdm		  = { .name = "l4per_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_L4PER_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_L4PER_L4PER3_CDOFFS,
	.dep_bit	  = DRA7XX_L4PER3_STATDEP_SHIFT,
	.flags		  = CLKDM_CAN_HWSUP_SWSUP,
};

static struct clockdomain l4per2_7xx_clkdm = {
	.name		  = "l4per2_clkdm",
	.pwrdm		  = { .name = "l4per_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_L4PER_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_L4PER_L4PER2_CDOFFS,
	.dep_bit	  = DRA7XX_L4PER2_STATDEP_SHIFT,
	.wkdep_srcs	  = l4per2_wkup_sleep_deps,
	.sleepdep_srcs	  = l4per2_wkup_sleep_deps,
	.flags		  = CLKDM_CAN_SWSUP,
};

static struct clockdomain mpu0_7xx_clkdm = {
	.name		  = "mpu0_clkdm",
	.pwrdm		  = { .name = "cpu0_pwrdm" },
	.prcm_partition	  = DRA7XX_MPU_PRCM_PARTITION,
	.cm_inst	  = DRA7XX_MPU_PRCM_CM_C0_INST,
	.clkdm_offs	  = DRA7XX_MPU_PRCM_CM_C0_CPU0_CDOFFS,
	.flags		  = CLKDM_CAN_FORCE_WAKEUP | CLKDM_CAN_HWSUP,
};

static struct clockdomain iva_7xx_clkdm = {
	.name		  = "iva_clkdm",
	.pwrdm		  = { .name = "iva_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_IVA_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_IVA_IVA_CDOFFS,
	.dep_bit	  = DRA7XX_IVA_STATDEP_SHIFT,
	.wkdep_srcs	  = iva_wkup_sleep_deps,
	.sleepdep_srcs	  = iva_wkup_sleep_deps,
	.flags		  = CLKDM_CAN_HWSUP_SWSUP,
};

static struct clockdomain coreaon_7xx_clkdm = {
	.name		  = "coreaon_clkdm",
	.pwrdm		  = { .name = "coreaon_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_COREAON_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_COREAON_COREAON_CDOFFS,
	.flags		  = CLKDM_CAN_FORCE_WAKEUP | CLKDM_CAN_HWSUP,
};

static struct clockdomain ipu1_7xx_clkdm = {
	.name		  = "ipu1_clkdm",
	.pwrdm		  = { .name = "ipu_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_AON_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_AON_IPU_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_AON_IPU_IPU1_CDOFFS,
	.dep_bit	  = DRA7XX_IPU1_STATDEP_SHIFT,
	.wkdep_srcs	  = ipu1_wkup_sleep_deps,
	.sleepdep_srcs	  = ipu1_wkup_sleep_deps,
	.flags		  = CLKDM_CAN_HWSUP_SWSUP,
};

static struct clockdomain ipu2_7xx_clkdm = {
	.name		  = "ipu2_clkdm",
	.pwrdm		  = { .name = "core_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_CORE_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_CORE_IPU2_CDOFFS,
	.dep_bit	  = DRA7XX_IPU2_STATDEP_SHIFT,
	.wkdep_srcs	  = ipu2_wkup_sleep_deps,
	.sleepdep_srcs	  = ipu2_wkup_sleep_deps,
	.flags		  = CLKDM_CAN_HWSUP_SWSUP,
};

static struct clockdomain l3init_7xx_clkdm = {
	.name		  = "l3init_clkdm",
	.pwrdm		  = { .name = "l3init_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_L3INIT_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_L3INIT_L3INIT_CDOFFS,
	.dep_bit	  = DRA7XX_L3INIT_STATDEP_SHIFT,
	.wkdep_srcs	  = l3init_wkup_sleep_deps,
	.sleepdep_srcs	  = l3init_wkup_sleep_deps,
	.flags		  = CLKDM_CAN_HWSUP_SWSUP,
};

static struct clockdomain l4sec_7xx_clkdm = {
	.name		  = "l4sec_clkdm",
	.pwrdm		  = { .name = "l4per_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_L4PER_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_L4PER_L4SEC_CDOFFS,
	.dep_bit	  = DRA7XX_L4SEC_STATDEP_SHIFT,
	.wkdep_srcs	  = l4sec_wkup_sleep_deps,
	.sleepdep_srcs	  = l4sec_wkup_sleep_deps,
	.flags		  = CLKDM_CAN_HWSUP_SWSUP,
};

static struct clockdomain l3main1_7xx_clkdm = {
	.name		  = "l3main1_clkdm",
	.pwrdm		  = { .name = "core_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_CORE_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_CORE_L3MAIN1_CDOFFS,
	.dep_bit	  = DRA7XX_L3MAIN1_STATDEP_SHIFT,
	.flags		  = CLKDM_CAN_HWSUP,
};

static struct clockdomain vpe_7xx_clkdm = {
	.name		  = "vpe_clkdm",
	.pwrdm		  = { .name = "vpe_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_AON_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_AON_VPE_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_AON_VPE_VPE_CDOFFS,
	.dep_bit	  = DRA7XX_VPE_STATDEP_SHIFT,
	.wkdep_srcs	  = vpe_wkup_sleep_deps,
	.sleepdep_srcs	  = vpe_wkup_sleep_deps,
	.flags		  = CLKDM_CAN_HWSUP_SWSUP,
};

static struct clockdomain mpu_7xx_clkdm = {
	.name		  = "mpu_clkdm",
	.pwrdm		  = { .name = "mpu_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_AON_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_AON_MPU_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_AON_MPU_MPU_CDOFFS,
	.wkdep_srcs	  = mpu_wkup_sleep_deps,
	.sleepdep_srcs	  = mpu_wkup_sleep_deps,
	.flags		  = CLKDM_CAN_FORCE_WAKEUP | CLKDM_CAN_HWSUP,
};

static struct clockdomain custefuse_7xx_clkdm = {
	.name		  = "custefuse_clkdm",
	.pwrdm		  = { .name = "custefuse_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_CUSTEFUSE_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_CUSTEFUSE_CUSTEFUSE_CDOFFS,
	.flags		  = CLKDM_CAN_FORCE_WAKEUP | CLKDM_CAN_HWSUP,
};

static struct clockdomain ipu_7xx_clkdm = {
	.name		  = "ipu_clkdm",
	.pwrdm		  = { .name = "ipu_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_AON_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_AON_IPU_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_AON_IPU_IPU_CDOFFS,
	.dep_bit	  = DRA7XX_IPU_STATDEP_SHIFT,
	.flags		  = CLKDM_CAN_HWSUP_SWSUP,
};

static struct clockdomain mpu1_7xx_clkdm = {
	.name		  = "mpu1_clkdm",
	.pwrdm		  = { .name = "cpu1_pwrdm" },
	.prcm_partition	  = DRA7XX_MPU_PRCM_PARTITION,
	.cm_inst	  = DRA7XX_MPU_PRCM_CM_C1_INST,
	.clkdm_offs	  = DRA7XX_MPU_PRCM_CM_C1_CPU1_CDOFFS,
	.flags		  = CLKDM_CAN_FORCE_WAKEUP | CLKDM_CAN_HWSUP,
};

static struct clockdomain gmac_7xx_clkdm = {
	.name		  = "gmac_clkdm",
	.pwrdm		  = { .name = "l3init_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_L3INIT_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_L3INIT_GMAC_CDOFFS,
	.dep_bit	  = DRA7XX_GMAC_STATDEP_SHIFT,
	.wkdep_srcs	  = gmac_wkup_sleep_deps,
	.sleepdep_srcs	  = gmac_wkup_sleep_deps,
	.flags		  = CLKDM_CAN_HWSUP_SWSUP,
};

static struct clockdomain l4cfg_7xx_clkdm = {
	.name		  = "l4cfg_clkdm",
	.pwrdm		  = { .name = "core_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_CORE_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_CORE_L4CFG_CDOFFS,
	.dep_bit	  = DRA7XX_L4CFG_STATDEP_SHIFT,
	.flags		  = CLKDM_CAN_HWSUP,
};

static struct clockdomain dma_7xx_clkdm = {
	.name		  = "dma_clkdm",
	.pwrdm		  = { .name = "core_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_CORE_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_CORE_DMA_CDOFFS,
	.wkdep_srcs	  = dma_wkup_sleep_deps,
	.sleepdep_srcs	  = dma_wkup_sleep_deps,
	.flags		  = CLKDM_CAN_FORCE_WAKEUP | CLKDM_CAN_HWSUP,
};

static struct clockdomain rtc_7xx_clkdm = {
	.name		  = "rtc_clkdm",
	.pwrdm		  = { .name = "rtc_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_AON_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_AON_RTC_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_AON_RTC_RTC_CDOFFS,
	.flags		  = CLKDM_CAN_FORCE_WAKEUP | CLKDM_CAN_HWSUP,
};

static struct clockdomain pcie_7xx_clkdm = {
	.name		  = "pcie_clkdm",
	.pwrdm		  = { .name = "l3init_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_L3INIT_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_L3INIT_PCIE_CDOFFS,
	.dep_bit	  = DRA7XX_PCIE_STATDEP_SHIFT,
	.wkdep_srcs	  = pcie_wkup_sleep_deps,
	.sleepdep_srcs	  = pcie_wkup_sleep_deps,
	.flags		  = CLKDM_CAN_HWSUP_SWSUP,
};

static struct clockdomain atl_7xx_clkdm = {
	.name		  = "atl_clkdm",
	.pwrdm		  = { .name = "core_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_CORE_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_CORE_ATL_CDOFFS,
	.dep_bit	  = DRA7XX_ATL_STATDEP_SHIFT,
	.flags		  = CLKDM_CAN_FORCE_WAKEUP | CLKDM_CAN_HWSUP,
};

static struct clockdomain l3instr_7xx_clkdm = {
	.name		  = "l3instr_clkdm",
	.pwrdm		  = { .name = "core_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_CORE_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_CORE_L3INSTR_CDOFFS,
};

static struct clockdomain dss_7xx_clkdm = {
	.name		  = "dss_clkdm",
	.pwrdm		  = { .name = "dss_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_DSS_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_DSS_DSS_CDOFFS,
	.dep_bit	  = DRA7XX_DSS_STATDEP_SHIFT,
	.wkdep_srcs	  = dss_wkup_sleep_deps,
	.sleepdep_srcs	  = dss_wkup_sleep_deps,
	.flags		  = CLKDM_CAN_HWSUP_SWSUP,
};

static struct clockdomain emif_7xx_clkdm = {
	.name		  = "emif_clkdm",
	.pwrdm		  = { .name = "core_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_CORE_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_CORE_EMIF_CDOFFS,
	.dep_bit	  = DRA7XX_EMIF_STATDEP_SHIFT,
	.flags		  = CLKDM_CAN_FORCE_WAKEUP | CLKDM_CAN_HWSUP,
};

static struct clockdomain emu_7xx_clkdm = {
	.name		  = "emu_clkdm",
	.pwrdm		  = { .name = "emu_pwrdm" },
	.prcm_partition	  = DRA7XX_PRM_PARTITION,
	.cm_inst	  = DRA7XX_PRM_EMU_CM_INST,
	.clkdm_offs	  = DRA7XX_PRM_EMU_CM_EMU_CDOFFS,
	.flags		  = CLKDM_CAN_FORCE_WAKEUP | CLKDM_CAN_HWSUP,
};

static struct clockdomain dsp2_7xx_clkdm = {
	.name		  = "dsp2_clkdm",
	.pwrdm		  = { .name = "dsp2_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_AON_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_AON_DSP2_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_AON_DSP2_DSP2_CDOFFS,
	.dep_bit	  = DRA7XX_DSP2_STATDEP_SHIFT,
	.wkdep_srcs	  = dsp2_wkup_sleep_deps,
	.sleepdep_srcs	  = dsp2_wkup_sleep_deps,
	.flags		  = CLKDM_CAN_HWSUP_SWSUP,
};

static struct clockdomain dsp1_7xx_clkdm = {
	.name		  = "dsp1_clkdm",
	.pwrdm		  = { .name = "dsp1_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_AON_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_AON_DSP1_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_AON_DSP1_DSP1_CDOFFS,
	.dep_bit	  = DRA7XX_DSP1_STATDEP_SHIFT,
	.wkdep_srcs	  = dsp1_wkup_sleep_deps,
	.sleepdep_srcs	  = dsp1_wkup_sleep_deps,
	.flags		  = CLKDM_CAN_HWSUP_SWSUP,
};

static struct clockdomain cam_7xx_clkdm = {
	.name		  = "cam_clkdm",
	.pwrdm		  = { .name = "cam_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_CAM_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_CAM_CAM_CDOFFS,
	.dep_bit	  = DRA7XX_CAM_STATDEP_SHIFT,
	.wkdep_srcs	  = cam_wkup_sleep_deps,
	.sleepdep_srcs	  = cam_wkup_sleep_deps,
	.flags		  = CLKDM_CAN_HWSUP_SWSUP,
};

static struct clockdomain l4per_7xx_clkdm = {
	.name		  = "l4per_clkdm",
	.pwrdm		  = { .name = "l4per_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_L4PER_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_L4PER_L4PER_CDOFFS,
	.dep_bit	  = DRA7XX_L4PER_STATDEP_SHIFT,
	.flags		  = CLKDM_CAN_HWSUP_SWSUP,
};

static struct clockdomain gpu_7xx_clkdm = {
	.name		  = "gpu_clkdm",
	.pwrdm		  = { .name = "gpu_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_GPU_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_GPU_GPU_CDOFFS,
	.dep_bit	  = DRA7XX_GPU_STATDEP_SHIFT,
	.wkdep_srcs	  = gpu_wkup_sleep_deps,
	.sleepdep_srcs	  = gpu_wkup_sleep_deps,
	.flags		  = CLKDM_CAN_HWSUP_SWSUP,
};

static struct clockdomain eve4_7xx_clkdm = {
	.name		  = "eve4_clkdm",
	.pwrdm		  = { .name = "eve4_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_AON_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_AON_EVE4_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_AON_EVE4_EVE4_CDOFFS,
	.dep_bit	  = DRA7XX_EVE4_STATDEP_SHIFT,
	.wkdep_srcs	  = eve4_wkup_sleep_deps,
	.sleepdep_srcs	  = eve4_wkup_sleep_deps,
	.flags		  = CLKDM_CAN_HWSUP_SWSUP,
};

static struct clockdomain eve2_7xx_clkdm = {
	.name		  = "eve2_clkdm",
	.pwrdm		  = { .name = "eve2_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_AON_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_AON_EVE2_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_AON_EVE2_EVE2_CDOFFS,
	.dep_bit	  = DRA7XX_EVE2_STATDEP_SHIFT,
	.wkdep_srcs	  = eve2_wkup_sleep_deps,
	.sleepdep_srcs	  = eve2_wkup_sleep_deps,
	.flags		  = CLKDM_CAN_HWSUP_SWSUP,
};

static struct clockdomain eve3_7xx_clkdm = {
	.name		  = "eve3_clkdm",
	.pwrdm		  = { .name = "eve3_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_AON_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_AON_EVE3_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_AON_EVE3_EVE3_CDOFFS,
	.dep_bit	  = DRA7XX_EVE3_STATDEP_SHIFT,
	.wkdep_srcs	  = eve3_wkup_sleep_deps,
	.sleepdep_srcs	  = eve3_wkup_sleep_deps,
	.flags		  = CLKDM_CAN_HWSUP_SWSUP,
};

static struct clockdomain wkupaon_7xx_clkdm = {
	.name		  = "wkupaon_clkdm",
	.pwrdm		  = { .name = "wkupaon_pwrdm" },
	.prcm_partition	  = DRA7XX_PRM_PARTITION,
	.cm_inst	  = DRA7XX_PRM_WKUPAON_CM_INST,
	.clkdm_offs	  = DRA7XX_PRM_WKUPAON_CM_WKUPAON_CDOFFS,
	.dep_bit	  = DRA7XX_WKUPAON_STATDEP_SHIFT,
	.flags		  = CLKDM_CAN_FORCE_WAKEUP | CLKDM_CAN_HWSUP,
};

static struct clockdomain eve1_7xx_clkdm = {
	.name		  = "eve1_clkdm",
	.pwrdm		  = { .name = "eve1_pwrdm" },
	.prcm_partition	  = DRA7XX_CM_CORE_AON_PARTITION,
	.cm_inst	  = DRA7XX_CM_CORE_AON_EVE1_INST,
	.clkdm_offs	  = DRA7XX_CM_CORE_AON_EVE1_EVE1_CDOFFS,
	.dep_bit	  = DRA7XX_EVE1_STATDEP_SHIFT,
	.wkdep_srcs	  = eve1_wkup_sleep_deps,
	.sleepdep_srcs	  = eve1_wkup_sleep_deps,
	.flags		  = CLKDM_CAN_HWSUP_SWSUP,
};

/* As clockdomains are added or removed above, this list must also be changed */
static struct clockdomain *clockdomains_dra7xx[] __initdata = {
	&l4per3_7xx_clkdm,
	&l4per2_7xx_clkdm,
	&mpu0_7xx_clkdm,
	&iva_7xx_clkdm,
	&coreaon_7xx_clkdm,
	&ipu1_7xx_clkdm,
	&ipu2_7xx_clkdm,
	&l3init_7xx_clkdm,
	&l4sec_7xx_clkdm,
	&l3main1_7xx_clkdm,
	&vpe_7xx_clkdm,
	&mpu_7xx_clkdm,
	&custefuse_7xx_clkdm,
	&ipu_7xx_clkdm,
	&mpu1_7xx_clkdm,
	&gmac_7xx_clkdm,
	&l4cfg_7xx_clkdm,
	&dma_7xx_clkdm,
	&rtc_7xx_clkdm,
	&pcie_7xx_clkdm,
	&atl_7xx_clkdm,
	&l3instr_7xx_clkdm,
	&dss_7xx_clkdm,
	&emif_7xx_clkdm,
	&emu_7xx_clkdm,
	&dsp2_7xx_clkdm,
	&dsp1_7xx_clkdm,
	&cam_7xx_clkdm,
	&l4per_7xx_clkdm,
	&gpu_7xx_clkdm,
	&eve4_7xx_clkdm,
	&eve2_7xx_clkdm,
	&eve3_7xx_clkdm,
	&wkupaon_7xx_clkdm,
	&eve1_7xx_clkdm,
	NULL
};

void __init dra7xx_clockdomains_init(void)
{
	clkdm_register_platform_funcs(&omap4_clkdm_operations);
	clkdm_register_clkdms(clockdomains_dra7xx);
	clkdm_complete_init();
}

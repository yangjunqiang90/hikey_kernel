/*
 * This header provides constants for the reset controller
 * based peripheral powerdown requests on the STMicroelectronics
 * STiH407 SoC.
 */
#ifndef _DT_BINDINGS_RESET_CONTROLLER_STIH407
#define _DT_BINDINGS_RESET_CONTROLLER_STIH407

/* Powerdown requests control 0 */
#define STIH407_EMISS_POWERDOWN		0
#define STIH407_NAND_POWERDOWN		1

/* Synp GMAC PowerDown */
#define STIH407_ETH1_POWERDOWN		2

/* Powerdown requests control 1 */
#define STIH407_USB3_POWERDOWN		3
#define STIH407_USB2_PORT1_POWERDOWN	4
#define STIH407_USB2_PORT0_POWERDOWN	5
#define STIH407_PCIE1_POWERDOWN		6
#define STIH407_PCIE0_POWERDOWN		7
#define STIH407_SATA1_POWERDOWN		8
#define STIH407_SATA0_POWERDOWN		9

/* Reset defines */
#define STIH407_ETH1_SOFTRESET		0
#define STIH407_MMC1_SOFTRESET		1
#define STIH407_PICOPHY_SOFTRESET	2
#define STIH407_IRB_SOFTRESET		3
#define STIH407_PCIE0_SOFTRESET		4
#define STIH407_PCIE1_SOFTRESET		5
#define STIH407_SATA0_SOFTRESET		6
#define STIH407_SATA1_SOFTRESET		7
#define STIH407_MIPHY0_SOFTRESET	8
#define STIH407_MIPHY1_SOFTRESET	9
#define STIH407_MIPHY2_SOFTRESET	10
#define STIH407_SATA0_PWR_SOFTRESET	11
#define STIH407_SATA1_PWR_SOFTRESET	12
#define STIH407_DELTA_SOFTRESET		13
#define STIH407_BLITTER_SOFTRESET	14
#define STIH407_HDTVOUT_SOFTRESET	15
#define STIH407_HDQVDP_SOFTRESET	16
#define STIH407_VDP_AUX_SOFTRESET	17
#define STIH407_COMPO_SOFTRESET		18
#define STIH407_HDMI_TX_PHY_SOFTRESET	19
#define STIH407_JPEG_DEC_SOFTRESET	20
#define STIH407_VP8_DEC_SOFTRESET	21
#define STIH407_GPU_SOFTRESET		22
#define STIH407_HVA_SOFTRESET		23
#define STIH407_ERAM_HVA_SOFTRESET	24
#define STIH407_LPM_SOFTRESET		25
#define STIH407_KEYSCAN_SOFTRESET	26
#define STIH407_USB2_PORT0_SOFTRESET	27
#define STIH407_USB2_PORT1_SOFTRESET	28

/* Picophy reset defines */
#define STIH407_PICOPHY0_RESET		0
#define STIH407_PICOPHY1_RESET		1
#define STIH407_PICOPHY2_RESET		2

#endif /* _DT_BINDINGS_RESET_CONTROLLER_STIH407 */

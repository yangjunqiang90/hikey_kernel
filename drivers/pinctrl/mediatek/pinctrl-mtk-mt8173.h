/*
 * Copyright (c) 2014 MediaTek Inc.
 * Author: Hongzhou.Yang <hongzhou.yang@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __PINCTRL_MTK_MT8173_H
#define __PINCTRL_MTK_MT8173_H

#include <linux/pinctrl/pinctrl.h>
#include "pinctrl-mtk-common.h"

static const struct mtk_desc_pin mtk_pins_mt8173[] = {
	MTK_PIN(
		PINCTRL_PIN(0, "EINT0"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 0),
		MTK_FUNCTION(0, "GPIO0"),
		MTK_FUNCTION(1, "IRDA_PDN"),
		MTK_FUNCTION(2, "I2S1_WS"),
		MTK_FUNCTION(3, "AUD_SPDIF"),
		MTK_FUNCTION(4, "UTXD0"),
		MTK_FUNCTION(7, "DBG_MON_A_20_")
	),
	MTK_PIN(
		PINCTRL_PIN(1, "EINT1"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 1),
		MTK_FUNCTION(0, "GPIO1"),
		MTK_FUNCTION(1, "IRDA_RXD"),
		MTK_FUNCTION(2, "I2S1_BCK"),
		MTK_FUNCTION(3, "SDA5"),
		MTK_FUNCTION(4, "URXD0"),
		MTK_FUNCTION(7, "DBG_MON_A_21_")
	),
	MTK_PIN(
		PINCTRL_PIN(2, "EINT2"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 2),
		MTK_FUNCTION(0, "GPIO2"),
		MTK_FUNCTION(1, "IRDA_TXD"),
		MTK_FUNCTION(2, "I2S1_MCK"),
		MTK_FUNCTION(3, "SCL5"),
		MTK_FUNCTION(4, "UTXD3"),
		MTK_FUNCTION(7, "DBG_MON_A_22_")
	),
	MTK_PIN(
		PINCTRL_PIN(3, "EINT3"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 3),
		MTK_FUNCTION(0, "GPIO3"),
		MTK_FUNCTION(1, "DSI1_TE"),
		MTK_FUNCTION(2, "I2S1_DO_1"),
		MTK_FUNCTION(3, "SDA3"),
		MTK_FUNCTION(4, "URXD3"),
		MTK_FUNCTION(7, "DBG_MON_A_23_")
	),
	MTK_PIN(
		PINCTRL_PIN(4, "EINT4"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 4),
		MTK_FUNCTION(0, "GPIO4"),
		MTK_FUNCTION(1, "DISP_PWM1"),
		MTK_FUNCTION(2, "I2S1_DO_2"),
		MTK_FUNCTION(3, "SCL3"),
		MTK_FUNCTION(4, "UCTS3"),
		MTK_FUNCTION(6, "SFWP_B")
	),
	MTK_PIN(
		PINCTRL_PIN(5, "EINT5"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 5),
		MTK_FUNCTION(0, "GPIO5"),
		MTK_FUNCTION(1, "PCM1_CLK"),
		MTK_FUNCTION(2, "I2S2_WS"),
		MTK_FUNCTION(3, "SPI_CK_3_"),
		MTK_FUNCTION(4, "URTS3"),
		MTK_FUNCTION(5, "AP_MD32_JTAG_TMS"),
		MTK_FUNCTION(6, "SFOUT")
	),
	MTK_PIN(
		PINCTRL_PIN(6, "EINT6"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 6),
		MTK_FUNCTION(0, "GPIO6"),
		MTK_FUNCTION(1, "PCM1_SYNC"),
		MTK_FUNCTION(2, "I2S2_BCK"),
		MTK_FUNCTION(3, "SPI_MI_3_"),
		MTK_FUNCTION(5, "AP_MD32_JTAG_TCK"),
		MTK_FUNCTION(6, "SFCS0")
	),
	MTK_PIN(
		PINCTRL_PIN(7, "EINT7"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 7),
		MTK_FUNCTION(0, "GPIO7"),
		MTK_FUNCTION(1, "PCM1_DI"),
		MTK_FUNCTION(2, "I2S2_DI_1"),
		MTK_FUNCTION(3, "SPI_MO_3_"),
		MTK_FUNCTION(5, "AP_MD32_JTAG_TDI"),
		MTK_FUNCTION(6, "SFHOLD")
	),
	MTK_PIN(
		PINCTRL_PIN(8, "EINT8"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 8),
		MTK_FUNCTION(0, "GPIO8"),
		MTK_FUNCTION(1, "PCM1_DO"),
		MTK_FUNCTION(2, "I2S2_DI_2"),
		MTK_FUNCTION(3, "SPI_CS_3_"),
		MTK_FUNCTION(4, "AUD_SPDIF"),
		MTK_FUNCTION(5, "AP_MD32_JTAG_TDO"),
		MTK_FUNCTION(6, "SFIN")
	),
	MTK_PIN(
		PINCTRL_PIN(9, "EINT9"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 9),
		MTK_FUNCTION(0, "GPIO9"),
		MTK_FUNCTION(1, "USB_DRVVBUS_P0"),
		MTK_FUNCTION(2, "I2S2_MCK"),
		MTK_FUNCTION(4, "USB_DRVVBUS_P1"),
		MTK_FUNCTION(5, "AP_MD32_JTAG_TRST"),
		MTK_FUNCTION(6, "SFCK")
	),
	MTK_PIN(
		PINCTRL_PIN(10, "EINT10"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 10),
		MTK_FUNCTION(0, "GPIO10"),
		MTK_FUNCTION(1, "CLKM0"),
		MTK_FUNCTION(2, "DSI1_TE"),
		MTK_FUNCTION(3, "DISP_PWM1"),
		MTK_FUNCTION(4, "PWM4"),
		MTK_FUNCTION(5, "IRDA_RXD")
	),
	MTK_PIN(
		PINCTRL_PIN(11, "EINT11"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 11),
		MTK_FUNCTION(0, "GPIO11"),
		MTK_FUNCTION(1, "CLKM1"),
		MTK_FUNCTION(2, "I2S3_WS"),
		MTK_FUNCTION(3, "USB_DRVVBUS_P0"),
		MTK_FUNCTION(4, "PWM5"),
		MTK_FUNCTION(5, "IRDA_TXD"),
		MTK_FUNCTION(6, "USB_DRVVBUS_P1"),
		MTK_FUNCTION(7, "DBG_MON_B_30_")
	),
	MTK_PIN(
		PINCTRL_PIN(12, "EINT12"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 12),
		MTK_FUNCTION(0, "GPIO12"),
		MTK_FUNCTION(1, "CLKM2"),
		MTK_FUNCTION(2, "I2S3_BCK"),
		MTK_FUNCTION(3, "SRCLKENA0"),
		MTK_FUNCTION(5, "I2S2_WS"),
		MTK_FUNCTION(7, "DBG_MON_B_32_")
	),
	MTK_PIN(
		PINCTRL_PIN(13, "EINT13"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 13),
		MTK_FUNCTION(0, "GPIO13"),
		MTK_FUNCTION(1, "CLKM3"),
		MTK_FUNCTION(2, "I2S3_MCK"),
		MTK_FUNCTION(3, "SRCLKENA0"),
		MTK_FUNCTION(5, "I2S2_BCK"),
		MTK_FUNCTION(7, "DBG_MON_A_32_")
	),
	MTK_PIN(
		PINCTRL_PIN(14, "EINT14"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 14),
		MTK_FUNCTION(0, "GPIO14"),
		MTK_FUNCTION(1, "CMDAT0"),
		MTK_FUNCTION(2, "CMCSD0"),
		MTK_FUNCTION(4, "CLKM2"),
		MTK_FUNCTION(7, "DBG_MON_B_6_")
	),
	MTK_PIN(
		PINCTRL_PIN(15, "EINT15"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 15),
		MTK_FUNCTION(0, "GPIO15"),
		MTK_FUNCTION(1, "CMDAT1"),
		MTK_FUNCTION(2, "CMCSD1"),
		MTK_FUNCTION(3, "CMFLASH"),
		MTK_FUNCTION(4, "CLKM3"),
		MTK_FUNCTION(7, "DBG_MON_B_29_")
	),
	MTK_PIN(
		PINCTRL_PIN(16, "IDDIG"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 16),
		MTK_FUNCTION(0, "GPIO16"),
		MTK_FUNCTION(1, "IDDIG"),
		MTK_FUNCTION(2, "CMFLASH"),
		MTK_FUNCTION(4, "PWM5")
	),
	MTK_PIN(
		PINCTRL_PIN(17, "WATCHDOG"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 17),
		MTK_FUNCTION(0, "GPIO17"),
		MTK_FUNCTION(1, "WATCHDOG_AO")
	),
	MTK_PIN(
		PINCTRL_PIN(18, "CEC"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 18),
		MTK_FUNCTION(0, "GPIO18"),
		MTK_FUNCTION(1, "CEC")
	),
	MTK_PIN(
		PINCTRL_PIN(19, "HDMISCK"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 19),
		MTK_FUNCTION(0, "GPIO19"),
		MTK_FUNCTION(1, "HDMISCK"),
		MTK_FUNCTION(2, "HDCP_SCL")
	),
	MTK_PIN(
		PINCTRL_PIN(20, "HDMISD"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 20),
		MTK_FUNCTION(0, "GPIO20"),
		MTK_FUNCTION(1, "HDMISD"),
		MTK_FUNCTION(2, "HDCP_SDA")
	),
	MTK_PIN(
		PINCTRL_PIN(21, "HTPLG"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 21),
		MTK_FUNCTION(0, "GPIO21"),
		MTK_FUNCTION(1, "HTPLG")
	),
	MTK_PIN(
		PINCTRL_PIN(22, "MSDC3_DAT0"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 22),
		MTK_FUNCTION(0, "GPIO22"),
		MTK_FUNCTION(1, "MSDC3_DAT0")
	),
	MTK_PIN(
		PINCTRL_PIN(23, "MSDC3_DAT1"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 23),
		MTK_FUNCTION(0, "GPIO23"),
		MTK_FUNCTION(1, "MSDC3_DAT1")
	),
	MTK_PIN(
		PINCTRL_PIN(24, "MSDC3_DAT2"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 24),
		MTK_FUNCTION(0, "GPIO24"),
		MTK_FUNCTION(1, "MSDC3_DAT2")
	),
	MTK_PIN(
		PINCTRL_PIN(25, "MSDC3_DAT3"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 25),
		MTK_FUNCTION(0, "GPIO25"),
		MTK_FUNCTION(1, "MSDC3_DAT3")
	),
	MTK_PIN(
		PINCTRL_PIN(26, "MSDC3_CLK"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 26),
		MTK_FUNCTION(0, "GPIO26"),
		MTK_FUNCTION(1, "MSDC3_CLK")
	),
	MTK_PIN(
		PINCTRL_PIN(27, "MSDC3_CMD"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 27),
		MTK_FUNCTION(0, "GPIO27"),
		MTK_FUNCTION(1, "MSDC3_CMD")
	),
	MTK_PIN(
		PINCTRL_PIN(28, "MSDC3_DSL"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 28),
		MTK_FUNCTION(0, "GPIO28"),
		MTK_FUNCTION(1, "MSDC3_DSL")
	),
	MTK_PIN(
		PINCTRL_PIN(29, "UCTS2"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 29),
		MTK_FUNCTION(0, "GPIO29"),
		MTK_FUNCTION(1, "UCTS2")
	),
	MTK_PIN(
		PINCTRL_PIN(30, "URTS2"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 30),
		MTK_FUNCTION(0, "GPIO30"),
		MTK_FUNCTION(1, "URTS2")
	),
	MTK_PIN(
		PINCTRL_PIN(31, "URXD2"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 31),
		MTK_FUNCTION(0, "GPIO31"),
		MTK_FUNCTION(1, "URXD2"),
		MTK_FUNCTION(2, "UTXD2")
	),
	MTK_PIN(
		PINCTRL_PIN(32, "UTXD2"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 32),
		MTK_FUNCTION(0, "GPIO32"),
		MTK_FUNCTION(1, "UTXD2"),
		MTK_FUNCTION(2, "URXD2")
	),
	MTK_PIN(
		PINCTRL_PIN(33, "DAICLK"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 33),
		MTK_FUNCTION(0, "GPIO33"),
		MTK_FUNCTION(1, " MRG_CLK"),
		MTK_FUNCTION(2, "PCM0_CLK")
	),
	MTK_PIN(
		PINCTRL_PIN(34, "DAIPCMIN"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 34),
		MTK_FUNCTION(0, "GPIO34"),
		MTK_FUNCTION(1, " MRG_DI"),
		MTK_FUNCTION(2, "PCM0_DI")
	),
	MTK_PIN(
		PINCTRL_PIN(35, "DAIPCMOUT"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 35),
		MTK_FUNCTION(0, "GPIO35"),
		MTK_FUNCTION(1, " MRG_DO"),
		MTK_FUNCTION(2, "PCM0_DO")
	),
	MTK_PIN(
		PINCTRL_PIN(36, "DAISYNC"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 36),
		MTK_FUNCTION(0, "GPIO36"),
		MTK_FUNCTION(1, " MRG_SYNC"),
		MTK_FUNCTION(2, "PCM0_SYNC")
	),
	MTK_PIN(
		PINCTRL_PIN(37, "EINT16"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 37),
		MTK_FUNCTION(0, "GPIO37"),
		MTK_FUNCTION(1, "USB_DRVVBUS_P0"),
		MTK_FUNCTION(2, "USB_DRVVBUS_P1"),
		MTK_FUNCTION(3, "PWM0"),
		MTK_FUNCTION(4, "PWM1"),
		MTK_FUNCTION(5, "PWM2"),
		MTK_FUNCTION(6, "CLKM0")
	),
	MTK_PIN(
		PINCTRL_PIN(38, "CONN_RST"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 38),
		MTK_FUNCTION(0, "GPIO38"),
		MTK_FUNCTION(1, "USB_DRVVBUS_P0"),
		MTK_FUNCTION(2, "USB_DRVVBUS_P1"),
		MTK_FUNCTION(6, "CLKM1")
	),
	MTK_PIN(
		PINCTRL_PIN(39, "CM2MCLK"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 39),
		MTK_FUNCTION(0, "GPIO39"),
		MTK_FUNCTION(1, "CM2MCLK"),
		MTK_FUNCTION(2, "CMCSD0"),
		MTK_FUNCTION(7, "DBG_MON_A_17_")
	),
	MTK_PIN(
		PINCTRL_PIN(40, "CMPCLK"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 40),
		MTK_FUNCTION(0, "GPIO40"),
		MTK_FUNCTION(1, "CMPCLK"),
		MTK_FUNCTION(2, "CMCSK"),
		MTK_FUNCTION(3, "CMCSD2"),
		MTK_FUNCTION(7, "DBG_MON_A_18_")
	),
	MTK_PIN(
		PINCTRL_PIN(41, "CMMCLK"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 41),
		MTK_FUNCTION(0, "GPIO41"),
		MTK_FUNCTION(1, "CMMCLK"),
		MTK_FUNCTION(7, "DBG_MON_A_19_")
	),
	MTK_PIN(
		PINCTRL_PIN(42, "DSI_TE"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 42),
		MTK_FUNCTION(0, "GPIO42"),
		MTK_FUNCTION(1, "DSI_TE")
	),
	MTK_PIN(
		PINCTRL_PIN(43, "SDA2"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 43),
		MTK_FUNCTION(0, "GPIO43"),
		MTK_FUNCTION(1, "SDA2")
	),
	MTK_PIN(
		PINCTRL_PIN(44, "SCL2"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 44),
		MTK_FUNCTION(0, "GPIO44"),
		MTK_FUNCTION(1, "SCL2")
	),
	MTK_PIN(
		PINCTRL_PIN(45, "SDA0"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 45),
		MTK_FUNCTION(0, "GPIO45"),
		MTK_FUNCTION(1, "SDA0")
	),
	MTK_PIN(
		PINCTRL_PIN(46, "SCL0"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 46),
		MTK_FUNCTION(0, "GPIO46"),
		MTK_FUNCTION(1, "SCL0")
	),
	MTK_PIN(
		PINCTRL_PIN(47, "RDN0_A"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 47),
		MTK_FUNCTION(0, "GPIO47"),
		MTK_FUNCTION(1, "CMDAT2")
	),
	MTK_PIN(
		PINCTRL_PIN(48, "RDP0_A"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 48),
		MTK_FUNCTION(0, "GPIO48"),
		MTK_FUNCTION(1, "CMDAT3")
	),
	MTK_PIN(
		PINCTRL_PIN(49, "RDN1_A"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 49),
		MTK_FUNCTION(0, "GPIO49"),
		MTK_FUNCTION(1, "CMDAT4")
	),
	MTK_PIN(
		PINCTRL_PIN(50, "RDP1_A"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 50),
		MTK_FUNCTION(0, "GPIO50"),
		MTK_FUNCTION(1, "CMDAT5")
	),
	MTK_PIN(
		PINCTRL_PIN(51, "RCN_A"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 51),
		MTK_FUNCTION(0, "GPIO51"),
		MTK_FUNCTION(1, "CMDAT6")
	),
	MTK_PIN(
		PINCTRL_PIN(52, "RCP_A"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 52),
		MTK_FUNCTION(0, "GPIO52"),
		MTK_FUNCTION(1, "CMDAT7")
	),
	MTK_PIN(
		PINCTRL_PIN(53, "RDN2_A"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 53),
		MTK_FUNCTION(0, "GPIO53"),
		MTK_FUNCTION(1, "CMDAT8"),
		MTK_FUNCTION(2, "CMCSD3")
	),
	MTK_PIN(
		PINCTRL_PIN(54, "RDP2_A"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 54),
		MTK_FUNCTION(0, "GPIO54"),
		MTK_FUNCTION(1, "CMDAT9"),
		MTK_FUNCTION(2, "CMCSD2")
	),
	MTK_PIN(
		PINCTRL_PIN(55, "RDN3_A"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 55),
		MTK_FUNCTION(0, "GPIO55"),
		MTK_FUNCTION(1, "CMHSYNC"),
		MTK_FUNCTION(2, "CMCSD1")
	),
	MTK_PIN(
		PINCTRL_PIN(56, "RDP3_A"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 56),
		MTK_FUNCTION(0, "GPIO56"),
		MTK_FUNCTION(1, "CMVSYNC"),
		MTK_FUNCTION(2, "CMCSD0")
	),
	MTK_PIN(
		PINCTRL_PIN(57, "MSDC0_DAT0"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 57),
		MTK_FUNCTION(0, "GPIO57"),
		MTK_FUNCTION(1, "MSDC0_DAT0"),
		MTK_FUNCTION(2, "I2S1_WS"),
		MTK_FUNCTION(7, "DBG_MON_B_7_")
	),
	MTK_PIN(
		PINCTRL_PIN(58, "MSDC0_DAT1"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 58),
		MTK_FUNCTION(0, "GPIO58"),
		MTK_FUNCTION(1, "MSDC0_DAT1"),
		MTK_FUNCTION(2, "I2S1_BCK"),
		MTK_FUNCTION(7, "DBG_MON_B_8_")
	),
	MTK_PIN(
		PINCTRL_PIN(59, "MSDC0_DAT2"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 59),
		MTK_FUNCTION(0, "GPIO59"),
		MTK_FUNCTION(1, "MSDC0_DAT2"),
		MTK_FUNCTION(2, "I2S1_MCK"),
		MTK_FUNCTION(7, "DBG_MON_B_9_")
	),
	MTK_PIN(
		PINCTRL_PIN(60, "MSDC0_DAT3"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 60),
		MTK_FUNCTION(0, "GPIO60"),
		MTK_FUNCTION(1, "MSDC0_DAT3"),
		MTK_FUNCTION(2, "I2S1_DO_1"),
		MTK_FUNCTION(7, "DBG_MON_B_10_")
	),
	MTK_PIN(
		PINCTRL_PIN(61, "MSDC0_DAT4"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 61),
		MTK_FUNCTION(0, "GPIO61"),
		MTK_FUNCTION(1, "MSDC0_DAT4"),
		MTK_FUNCTION(2, "I2S1_DO_2"),
		MTK_FUNCTION(7, "DBG_MON_B_11_")
	),
	MTK_PIN(
		PINCTRL_PIN(62, "MSDC0_DAT5"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 62),
		MTK_FUNCTION(0, "GPIO62"),
		MTK_FUNCTION(1, "MSDC0_DAT5"),
		MTK_FUNCTION(2, "I2S2_WS"),
		MTK_FUNCTION(7, "DBG_MON_B_12_")
	),
	MTK_PIN(
		PINCTRL_PIN(63, "MSDC0_DAT6"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 63),
		MTK_FUNCTION(0, "GPIO63"),
		MTK_FUNCTION(1, "MSDC0_DAT6"),
		MTK_FUNCTION(2, "I2S2_BCK"),
		MTK_FUNCTION(7, "DBG_MON_B_13_")
	),
	MTK_PIN(
		PINCTRL_PIN(64, "MSDC0_DAT7"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 64),
		MTK_FUNCTION(0, "GPIO64"),
		MTK_FUNCTION(1, "MSDC0_DAT7"),
		MTK_FUNCTION(2, "I2S2_DI_1"),
		MTK_FUNCTION(7, "DBG_MON_B_14_")
	),
	MTK_PIN(
		PINCTRL_PIN(65, "MSDC0_CLK"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 65),
		MTK_FUNCTION(0, "GPIO65"),
		MTK_FUNCTION(1, "MSDC0_CLK"),
		MTK_FUNCTION(7, "DBG_MON_B_16_")
	),
	MTK_PIN(
		PINCTRL_PIN(66, "MSDC0_CMD"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 66),
		MTK_FUNCTION(0, "GPIO66"),
		MTK_FUNCTION(1, "MSDC0_CMD"),
		MTK_FUNCTION(2, "I2S2_DI_2"),
		MTK_FUNCTION(7, "DBG_MON_B_15_")
	),
	MTK_PIN(
		PINCTRL_PIN(67, "MSDC0_DSL"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 67),
		MTK_FUNCTION(0, "GPIO67"),
		MTK_FUNCTION(1, "MSDC0_DSL"),
		MTK_FUNCTION(7, "DBG_MON_B_17_")
	),
	MTK_PIN(
		PINCTRL_PIN(68, "MSDC0_RST_"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 68),
		MTK_FUNCTION(0, "GPIO68"),
		MTK_FUNCTION(1, "MSDC0_RSTB"),
		MTK_FUNCTION(2, "I2S2_MCK"),
		MTK_FUNCTION(7, "DBG_MON_B_18_")
	),
	MTK_PIN(
		PINCTRL_PIN(69, "SPI_CK"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 69),
		MTK_FUNCTION(0, "GPIO69"),
		MTK_FUNCTION(1, "SPI_CK_0_"),
		MTK_FUNCTION(2, "I2S3_DO_1"),
		MTK_FUNCTION(3, "PWM0"),
		MTK_FUNCTION(4, "PWM5"),
		MTK_FUNCTION(5, "I2S2_MCK"),
		MTK_FUNCTION(7, "DBG_MON_B_19_")
	),
	MTK_PIN(
		PINCTRL_PIN(70, "SPI_MI"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 70),
		MTK_FUNCTION(0, "GPIO70"),
		MTK_FUNCTION(1, "SPI_MI_0_"),
		MTK_FUNCTION(2, "I2S3_DO_2"),
		MTK_FUNCTION(3, "PWM1"),
		MTK_FUNCTION(4, "SPI_MO_0_"),
		MTK_FUNCTION(5, "I2S2_DI_1"),
		MTK_FUNCTION(6, "DSI1_TE"),
		MTK_FUNCTION(7, "DBG_MON_B_20_")
	),
	MTK_PIN(
		PINCTRL_PIN(71, "SPI_MO"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 71),
		MTK_FUNCTION(0, "GPIO71"),
		MTK_FUNCTION(1, "SPI_MO_0_"),
		MTK_FUNCTION(2, "I2S3_DO_3"),
		MTK_FUNCTION(3, "PWM2"),
		MTK_FUNCTION(4, "SPI_MI_0_"),
		MTK_FUNCTION(5, "I2S2_DI_2"),
		MTK_FUNCTION(7, "DBG_MON_B_21_")
	),
	MTK_PIN(
		PINCTRL_PIN(72, "SPI_CS"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 72),
		MTK_FUNCTION(0, "GPIO72"),
		MTK_FUNCTION(1, "SPI_CS_0_"),
		MTK_FUNCTION(2, "I2S3_DO_4"),
		MTK_FUNCTION(3, "PWM3"),
		MTK_FUNCTION(4, "PWM6"),
		MTK_FUNCTION(5, "DISP_PWM1"),
		MTK_FUNCTION(7, "DBG_MON_B_22_")
	),
	MTK_PIN(
		PINCTRL_PIN(73, "MSDC1_DAT0"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 73),
		MTK_FUNCTION(0, "GPIO73"),
		MTK_FUNCTION(1, "MSDC1_DAT0"),
		MTK_FUNCTION(7, "DBG_MON_B_24_")
	),
	MTK_PIN(
		PINCTRL_PIN(74, "MSDC1_DAT1"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 74),
		MTK_FUNCTION(0, "GPIO74"),
		MTK_FUNCTION(1, "MSDC1_DAT1"),
		MTK_FUNCTION(7, "DBG_MON_B_25_")
	),
	MTK_PIN(
		PINCTRL_PIN(75, "MSDC1_DAT2"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 75),
		MTK_FUNCTION(0, "GPIO75"),
		MTK_FUNCTION(1, "MSDC1_DAT2"),
		MTK_FUNCTION(7, "DBG_MON_B_26_")
	),
	MTK_PIN(
		PINCTRL_PIN(76, "MSDC1_DAT3"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 76),
		MTK_FUNCTION(0, "GPIO76"),
		MTK_FUNCTION(1, "MSDC1_DAT3"),
		MTK_FUNCTION(7, "DBG_MON_B_27_")
	),
	MTK_PIN(
		PINCTRL_PIN(77, "MSDC1_CLK"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 77),
		MTK_FUNCTION(0, "GPIO77"),
		MTK_FUNCTION(1, "MSDC1_CLK"),
		MTK_FUNCTION(7, "DBG_MON_B_28_")
	),
	MTK_PIN(
		PINCTRL_PIN(78, "MSDC1_CMD"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 78),
		MTK_FUNCTION(0, "GPIO78"),
		MTK_FUNCTION(1, "MSDC1_CMD"),
		MTK_FUNCTION(7, "DBG_MON_B_23_")
	),
	MTK_PIN(
		PINCTRL_PIN(79, "PWRAP_SPI0_MI"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 79),
		MTK_FUNCTION(0, "GPIO79"),
		MTK_FUNCTION(1, "PWRAP_SPIMI"),
		MTK_FUNCTION(2, "PWRAP_SPIMO")
	),
	MTK_PIN(
		PINCTRL_PIN(80, "PWRAP_SPI0_MO"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 80),
		MTK_FUNCTION(0, "GPIO80"),
		MTK_FUNCTION(1, "PWRAP_SPIMO"),
		MTK_FUNCTION(2, "PWRAP_SPIMI")
	),
	MTK_PIN(
		PINCTRL_PIN(81, "PWRAP_SPI0_CK"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 81),
		MTK_FUNCTION(0, "GPIO81"),
		MTK_FUNCTION(1, "PWRAP_SPICK")
	),
	MTK_PIN(
		PINCTRL_PIN(82, "PWRAP_SPI0_CSN"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 82),
		MTK_FUNCTION(0, "GPIO82"),
		MTK_FUNCTION(1, "PWRAP_SPICS")
	),
	MTK_PIN(
		PINCTRL_PIN(83, "AUD_CLK_MOSI"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 83),
		MTK_FUNCTION(0, "GPIO83"),
		MTK_FUNCTION(1, "AUD_CLK_MOSI")
	),
	MTK_PIN(
		PINCTRL_PIN(84, "AUD_DAT_MISO"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 84),
		MTK_FUNCTION(0, "GPIO84"),
		MTK_FUNCTION(1, "AUD_DAT_MISO"),
		MTK_FUNCTION(2, "AUD_DAT_MOSI")
	),
	MTK_PIN(
		PINCTRL_PIN(85, "AUD_DAT_MOSI"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 85),
		MTK_FUNCTION(0, "GPIO85"),
		MTK_FUNCTION(1, "AUD_DAT_MOSI"),
		MTK_FUNCTION(2, "AUD_DAT_MISO")
	),
	MTK_PIN(
		PINCTRL_PIN(86, "RTC32K_CK"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 86),
		MTK_FUNCTION(0, "GPIO86"),
		MTK_FUNCTION(1, "RTC32K_CK")
	),
	MTK_PIN(
		PINCTRL_PIN(87, "DISP_PWM0"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 87),
		MTK_FUNCTION(0, "GPIO87"),
		MTK_FUNCTION(1, "DISP_PWM0"),
		MTK_FUNCTION(2, "DISP_PWM1"),
		MTK_FUNCTION(7, "DBG_MON_B_31_")
	),
	MTK_PIN(
		PINCTRL_PIN(88, "SRCLKENAI"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 88),
		MTK_FUNCTION(0, "GPIO88"),
		MTK_FUNCTION(1, "SRCLKENAI")
	),
	MTK_PIN(
		PINCTRL_PIN(89, "SRCLKENAI2"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 89),
		MTK_FUNCTION(0, "GPIO89"),
		MTK_FUNCTION(1, "SRCLKENAI2")
	),
	MTK_PIN(
		PINCTRL_PIN(90, "SRCLKENA0"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 90),
		MTK_FUNCTION(0, "GPIO90"),
		MTK_FUNCTION(1, "SRCLKENA0")
	),
	MTK_PIN(
		PINCTRL_PIN(91, "SRCLKENA1"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 91),
		MTK_FUNCTION(0, "GPIO91"),
		MTK_FUNCTION(1, "SRCLKENA1")
	),
	MTK_PIN(
		PINCTRL_PIN(92, "PCM_CLK"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 92),
		MTK_FUNCTION(0, "GPIO92"),
		MTK_FUNCTION(1, "PCM1_CLK"),
		MTK_FUNCTION(2, "I2S0_BCK"),
		MTK_FUNCTION(7, "DBG_MON_A_24_")
	),
	MTK_PIN(
		PINCTRL_PIN(93, "PCM_SYNC"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 93),
		MTK_FUNCTION(0, "GPIO93"),
		MTK_FUNCTION(1, "PCM1_SYNC"),
		MTK_FUNCTION(2, "I2S0_WS"),
		MTK_FUNCTION(7, "DBG_MON_A_25_")
	),
	MTK_PIN(
		PINCTRL_PIN(94, "PCM_RX"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 94),
		MTK_FUNCTION(0, "GPIO94"),
		MTK_FUNCTION(1, "PCM1_DI"),
		MTK_FUNCTION(2, "I2S0_DI"),
		MTK_FUNCTION(7, "DBG_MON_A_26_")
	),
	MTK_PIN(
		PINCTRL_PIN(95, "PCM_TX"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 95),
		MTK_FUNCTION(0, "GPIO95"),
		MTK_FUNCTION(1, "PCM1_DO"),
		MTK_FUNCTION(2, "I2S0_DO"),
		MTK_FUNCTION(7, "DBG_MON_A_27_")
	),
	MTK_PIN(
		PINCTRL_PIN(96, "URXD1"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 96),
		MTK_FUNCTION(0, "GPIO96"),
		MTK_FUNCTION(1, "URXD1"),
		MTK_FUNCTION(2, "UTXD1"),
		MTK_FUNCTION(7, "DBG_MON_A_28_")
	),
	MTK_PIN(
		PINCTRL_PIN(97, "UTXD1"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 97),
		MTK_FUNCTION(0, "GPIO97"),
		MTK_FUNCTION(1, "UTXD1"),
		MTK_FUNCTION(2, "URXD1"),
		MTK_FUNCTION(7, "DBG_MON_A_29_")
	),
	MTK_PIN(
		PINCTRL_PIN(98, "URTS1"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 98),
		MTK_FUNCTION(0, "GPIO98"),
		MTK_FUNCTION(1, "URTS1"),
		MTK_FUNCTION(2, "UCTS1"),
		MTK_FUNCTION(7, "DBG_MON_A_30_")
	),
	MTK_PIN(
		PINCTRL_PIN(99, "UCTS1"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 99),
		MTK_FUNCTION(0, "GPIO99"),
		MTK_FUNCTION(1, "UCTS1"),
		MTK_FUNCTION(2, "URTS1"),
		MTK_FUNCTION(7, "DBG_MON_A_31_")
	),
	MTK_PIN(
		PINCTRL_PIN(100, "MSDC2_DAT0"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 100),
		MTK_FUNCTION(0, "GPIO100"),
		MTK_FUNCTION(1, "MSDC2_DAT0"),
		MTK_FUNCTION(3, "USB_DRVVBUS_P0"),
		MTK_FUNCTION(4, "SDA5"),
		MTK_FUNCTION(5, "USB_DRVVBUS_P1"),
		MTK_FUNCTION(7, "DBG_MON_B_0_")
	),
	MTK_PIN(
		PINCTRL_PIN(101, "MSDC2_DAT1"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 101),
		MTK_FUNCTION(0, "GPIO101"),
		MTK_FUNCTION(1, "MSDC2_DAT1"),
		MTK_FUNCTION(3, "AUD_SPDIF"),
		MTK_FUNCTION(4, "SCL5"),
		MTK_FUNCTION(7, "DBG_MON_B_1_")
	),
	MTK_PIN(
		PINCTRL_PIN(102, "MSDC2_DAT2"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 102),
		MTK_FUNCTION(0, "GPIO102"),
		MTK_FUNCTION(1, "MSDC2_DAT2"),
		MTK_FUNCTION(3, "UTXD0"),
		MTK_FUNCTION(5, "PWM0"),
		MTK_FUNCTION(6, "SPI_CK_1_"),
		MTK_FUNCTION(7, "DBG_MON_B_2_")
	),
	MTK_PIN(
		PINCTRL_PIN(103, "MSDC2_DAT3"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 103),
		MTK_FUNCTION(0, "GPIO103"),
		MTK_FUNCTION(1, "MSDC2_DAT3"),
		MTK_FUNCTION(3, "URXD0"),
		MTK_FUNCTION(5, "PWM1"),
		MTK_FUNCTION(6, "SPI_MI_1_"),
		MTK_FUNCTION(7, "DBG_MON_B_3_")
	),
	MTK_PIN(
		PINCTRL_PIN(104, "MSDC2_CLK"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 104),
		MTK_FUNCTION(0, "GPIO104"),
		MTK_FUNCTION(1, "MSDC2_CLK"),
		MTK_FUNCTION(3, "UTXD3"),
		MTK_FUNCTION(4, "SDA3"),
		MTK_FUNCTION(5, "PWM2"),
		MTK_FUNCTION(6, "SPI_MO_1_"),
		MTK_FUNCTION(7, "DBG_MON_B_4_")
	),
	MTK_PIN(
		PINCTRL_PIN(105, "MSDC2_CMD"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 105),
		MTK_FUNCTION(0, "GPIO105"),
		MTK_FUNCTION(1, "MSDC2_CMD"),
		MTK_FUNCTION(3, "URXD3"),
		MTK_FUNCTION(4, "SCL3"),
		MTK_FUNCTION(5, "PWM3"),
		MTK_FUNCTION(6, "SPI_CS_1_"),
		MTK_FUNCTION(7, "DBG_MON_B_5_")
	),
	MTK_PIN(
		PINCTRL_PIN(106, "SDA3"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 106),
		MTK_FUNCTION(0, "GPIO106"),
		MTK_FUNCTION(1, "SDA3")
	),
	MTK_PIN(
		PINCTRL_PIN(107, "SCL3"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 107),
		MTK_FUNCTION(0, "GPIO107"),
		MTK_FUNCTION(1, "SCL3")
	),
	MTK_PIN(
		PINCTRL_PIN(108, "JTMS"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 108),
		MTK_FUNCTION(0, "GPIO108"),
		MTK_FUNCTION(1, "JTMS"),
		MTK_FUNCTION(2, " MFG_JTAG_TMS"),
		MTK_FUNCTION(5, "AP_MD32_JTAG_TMS"),
		MTK_FUNCTION(6, "DFD_TMS")
	),
	MTK_PIN(
		PINCTRL_PIN(109, "JTCK"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 109),
		MTK_FUNCTION(0, "GPIO109"),
		MTK_FUNCTION(1, "JTCK"),
		MTK_FUNCTION(2, " MFG_JTAG_TCK"),
		MTK_FUNCTION(5, "AP_MD32_JTAG_TCK"),
		MTK_FUNCTION(6, "DFD_TCK")
	),
	MTK_PIN(
		PINCTRL_PIN(110, "JTDI"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 110),
		MTK_FUNCTION(0, "GPIO110"),
		MTK_FUNCTION(1, "JTDI"),
		MTK_FUNCTION(2, " MFG_JTAG_TDI"),
		MTK_FUNCTION(5, "AP_MD32_JTAG_TDI"),
		MTK_FUNCTION(6, "DFD_TDI")
	),
	MTK_PIN(
		PINCTRL_PIN(111, "JTDO"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 111),
		MTK_FUNCTION(0, "GPIO111"),
		MTK_FUNCTION(1, "JTDO"),
		MTK_FUNCTION(2, "MFG_JTAG_TDO"),
		MTK_FUNCTION(5, "AP_MD32_JTAG_TDO"),
		MTK_FUNCTION(6, "DFD_TDO")
	),
	MTK_PIN(
		PINCTRL_PIN(112, "JTRST_B"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 112),
		MTK_FUNCTION(0, "GPIO112"),
		MTK_FUNCTION(1, "JTRST_B"),
		MTK_FUNCTION(2, " MFG_JTAG_TRSTN"),
		MTK_FUNCTION(5, "AP_MD32_JTAG_TRST"),
		MTK_FUNCTION(6, "DFD_NTRST")
	),
	MTK_PIN(
		PINCTRL_PIN(113, "URXD0"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 113),
		MTK_FUNCTION(0, "GPIO113"),
		MTK_FUNCTION(1, "URXD0"),
		MTK_FUNCTION(2, "UTXD0"),
		MTK_FUNCTION(6, "I2S2_WS"),
		MTK_FUNCTION(7, "DBG_MON_A_0_")
	),
	MTK_PIN(
		PINCTRL_PIN(114, "UTXD0"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 114),
		MTK_FUNCTION(0, "GPIO114"),
		MTK_FUNCTION(1, "UTXD0"),
		MTK_FUNCTION(2, "URXD0"),
		MTK_FUNCTION(6, "I2S2_BCK"),
		MTK_FUNCTION(7, "DBG_MON_A_1_")
	),
	MTK_PIN(
		PINCTRL_PIN(115, "URTS0"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 115),
		MTK_FUNCTION(0, "GPIO115"),
		MTK_FUNCTION(1, "URTS0"),
		MTK_FUNCTION(2, "UCTS0"),
		MTK_FUNCTION(6, "I2S2_MCK"),
		MTK_FUNCTION(7, "DBG_MON_A_2_")
	),
	MTK_PIN(
		PINCTRL_PIN(116, "UCTS0"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 116),
		MTK_FUNCTION(0, "GPIO116"),
		MTK_FUNCTION(1, "UCTS0"),
		MTK_FUNCTION(2, "URTS0"),
		MTK_FUNCTION(6, "I2S2_DI_1"),
		MTK_FUNCTION(7, "DBG_MON_A_3_")
	),
	MTK_PIN(
		PINCTRL_PIN(117, "URXD3"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 117),
		MTK_FUNCTION(0, "GPIO117"),
		MTK_FUNCTION(1, "URXD3"),
		MTK_FUNCTION(2, "UTXD3"),
		MTK_FUNCTION(7, "DBG_MON_A_9_")
	),
	MTK_PIN(
		PINCTRL_PIN(118, "UTXD3"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 118),
		MTK_FUNCTION(0, "GPIO118"),
		MTK_FUNCTION(1, "UTXD3"),
		MTK_FUNCTION(2, "URXD3"),
		MTK_FUNCTION(7, "DBG_MON_A_10_")
	),
	MTK_PIN(
		PINCTRL_PIN(119, "KPROW0"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 119),
		MTK_FUNCTION(0, "GPIO119"),
		MTK_FUNCTION(1, "KROW0"),
		MTK_FUNCTION(7, "DBG_MON_A_11_")
	),
	MTK_PIN(
		PINCTRL_PIN(120, "KPROW1"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 120),
		MTK_FUNCTION(0, "GPIO120"),
		MTK_FUNCTION(1, "KROW1"),
		MTK_FUNCTION(3, "PWM6"),
		MTK_FUNCTION(7, "DBG_MON_A_12_")
	),
	MTK_PIN(
		PINCTRL_PIN(121, "KPROW2"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 121),
		MTK_FUNCTION(0, "GPIO121"),
		MTK_FUNCTION(1, "KROW2"),
		MTK_FUNCTION(2, "IRDA_PDN"),
		MTK_FUNCTION(3, "USB_DRVVBUS_P0"),
		MTK_FUNCTION(4, "PWM4"),
		MTK_FUNCTION(5, "USB_DRVVBUS_P1"),
		MTK_FUNCTION(7, "DBG_MON_A_13_")
	),
	MTK_PIN(
		PINCTRL_PIN(122, "KPCOL0"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 122),
		MTK_FUNCTION(0, "GPIO122"),
		MTK_FUNCTION(1, "KCOL0"),
		MTK_FUNCTION(7, "DBG_MON_A_14_")
	),
	MTK_PIN(
		PINCTRL_PIN(123, "KPCOL1"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 123),
		MTK_FUNCTION(0, "GPIO123"),
		MTK_FUNCTION(1, "KCOL1"),
		MTK_FUNCTION(2, "IRDA_RXD"),
		MTK_FUNCTION(3, "PWM5"),
		MTK_FUNCTION(7, "DBG_MON_A_15_")
	),
	MTK_PIN(
		PINCTRL_PIN(124, "KPCOL2"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 124),
		MTK_FUNCTION(0, "GPIO124"),
		MTK_FUNCTION(1, "KCOL2"),
		MTK_FUNCTION(2, "IRDA_TXD"),
		MTK_FUNCTION(3, "USB_DRVVBUS_P0"),
		MTK_FUNCTION(4, "PWM3"),
		MTK_FUNCTION(5, "USB_DRVVBUS_P1"),
		MTK_FUNCTION(7, "DBG_MON_A_16_")
	),
	MTK_PIN(
		PINCTRL_PIN(125, "SDA1"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 125),
		MTK_FUNCTION(0, "GPIO125"),
		MTK_FUNCTION(1, "SDA1")
	),
	MTK_PIN(
		PINCTRL_PIN(126, "SCL1"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 126),
		MTK_FUNCTION(0, "GPIO126"),
		MTK_FUNCTION(1, "SCL1")
	),
	MTK_PIN(
		PINCTRL_PIN(127, "LCM_RST"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 127),
		MTK_FUNCTION(0, "GPIO127"),
		MTK_FUNCTION(1, "LCM_RST")
	),
	MTK_PIN(
		PINCTRL_PIN(128, "I2S0_LRCK"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 128),
		MTK_FUNCTION(0, "GPIO128"),
		MTK_FUNCTION(1, "I2S0_WS"),
		MTK_FUNCTION(2, "I2S1_WS"),
		MTK_FUNCTION(3, "I2S2_WS"),
		MTK_FUNCTION(5, "SPI_CK_2_"),
		MTK_FUNCTION(7, "DBG_MON_A_4_")
	),
	MTK_PIN(
		PINCTRL_PIN(129, "I2S0_BCK"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 129),
		MTK_FUNCTION(0, "GPIO129"),
		MTK_FUNCTION(1, "I2S0_BCK"),
		MTK_FUNCTION(2, "I2S1_BCK"),
		MTK_FUNCTION(3, "I2S2_BCK"),
		MTK_FUNCTION(5, "SPI_MI_2_"),
		MTK_FUNCTION(7, "DBG_MON_A_5_")
	),
	MTK_PIN(
		PINCTRL_PIN(130, "I2S0_MCK"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 130),
		MTK_FUNCTION(0, "GPIO130"),
		MTK_FUNCTION(1, "I2S0_MCK"),
		MTK_FUNCTION(2, "I2S1_MCK"),
		MTK_FUNCTION(3, "I2S2_MCK"),
		MTK_FUNCTION(5, "SPI_MO_2_"),
		MTK_FUNCTION(7, "DBG_MON_A_6_")
	),
	MTK_PIN(
		PINCTRL_PIN(131, "I2S0_DATA0"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 131),
		MTK_FUNCTION(0, "GPIO131"),
		MTK_FUNCTION(1, "I2S0_DO"),
		MTK_FUNCTION(2, "I2S1_DO_1"),
		MTK_FUNCTION(3, "I2S2_DI_1"),
		MTK_FUNCTION(5, "SPI_CS_2_"),
		MTK_FUNCTION(7, "DBG_MON_A_7_")
	),
	MTK_PIN(
		PINCTRL_PIN(132, "I2S0_DATA1"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 132),
		MTK_FUNCTION(0, "GPIO132"),
		MTK_FUNCTION(1, "I2S0_DI"),
		MTK_FUNCTION(2, "I2S1_DO_2"),
		MTK_FUNCTION(3, "I2S2_DI_2"),
		MTK_FUNCTION(7, "DBG_MON_A_8_")
	),
	MTK_PIN(
		PINCTRL_PIN(133, "SDA4"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 133),
		MTK_FUNCTION(0, "GPIO133"),
		MTK_FUNCTION(1, "SDA4")
	),
	MTK_PIN(
		PINCTRL_PIN(134, "SCL4"),
		NULL, "mt8173",
		MTK_EINT_FUNCTION(0, 134),
		MTK_FUNCTION(0, "GPIO134"),
		MTK_FUNCTION(1, "SCL4")
	),
};

#endif /* __PINCTRL_MTK_MT8173_H */

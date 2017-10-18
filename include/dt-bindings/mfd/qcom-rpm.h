/*
 * This header provides constants for the Qualcomm RPM bindings.
 */

#ifndef _DT_BINDINGS_MFD_QCOM_RPM_H
#define _DT_BINDINGS_MFD_QCOM_RPM_H

/*
 * Constants use to identify individual resources in the RPM.
 */
#define QCOM_RPM_APPS_FABRIC_ARB		1
#define QCOM_RPM_APPS_FABRIC_CLK		2
#define QCOM_RPM_APPS_FABRIC_HALT		3
#define QCOM_RPM_APPS_FABRIC_IOCTL		4
#define QCOM_RPM_APPS_FABRIC_MODE		5
#define QCOM_RPM_APPS_L2_CACHE_CTL		6
#define QCOM_RPM_CFPB_CLK			7
#define QCOM_RPM_CXO_BUFFERS			8
#define QCOM_RPM_CXO_CLK			9
#define QCOM_RPM_DAYTONA_FABRIC_CLK		10
#define QCOM_RPM_DDR_DMM			11
#define QCOM_RPM_EBI1_CLK			12
#define QCOM_RPM_HDMI_SWITCH			13
#define QCOM_RPM_MMFPB_CLK			14
#define QCOM_RPM_MM_FABRIC_ARB			15
#define QCOM_RPM_MM_FABRIC_CLK			16
#define QCOM_RPM_MM_FABRIC_HALT			17
#define QCOM_RPM_MM_FABRIC_IOCTL		18
#define QCOM_RPM_MM_FABRIC_MODE			19
#define QCOM_RPM_PLL_4				20
#define QCOM_RPM_PM8058_LDO0			21
#define QCOM_RPM_PM8058_LDO1			22
#define QCOM_RPM_PM8058_LDO2			23
#define QCOM_RPM_PM8058_LDO3			24
#define QCOM_RPM_PM8058_LDO4			25
#define QCOM_RPM_PM8058_LDO5			26
#define QCOM_RPM_PM8058_LDO6			27
#define QCOM_RPM_PM8058_LDO7			28
#define QCOM_RPM_PM8058_LDO8			29
#define QCOM_RPM_PM8058_LDO9			30
#define QCOM_RPM_PM8058_LDO10			31
#define QCOM_RPM_PM8058_LDO11			32
#define QCOM_RPM_PM8058_LDO12			33
#define QCOM_RPM_PM8058_LDO13			34
#define QCOM_RPM_PM8058_LDO14			35
#define QCOM_RPM_PM8058_LDO15			36
#define QCOM_RPM_PM8058_LDO16			37
#define QCOM_RPM_PM8058_LDO17			38
#define QCOM_RPM_PM8058_LDO18			39
#define QCOM_RPM_PM8058_LDO19			40
#define QCOM_RPM_PM8058_LDO20			41
#define QCOM_RPM_PM8058_LDO21			42
#define QCOM_RPM_PM8058_LDO22			43
#define QCOM_RPM_PM8058_LDO23			44
#define QCOM_RPM_PM8058_LDO24			45
#define QCOM_RPM_PM8058_LDO25			46
#define QCOM_RPM_PM8058_LVS0			47
#define QCOM_RPM_PM8058_LVS1			48
#define QCOM_RPM_PM8058_NCP			49
#define QCOM_RPM_PM8058_SMPS0			50
#define QCOM_RPM_PM8058_SMPS1			51
#define QCOM_RPM_PM8058_SMPS2			52
#define QCOM_RPM_PM8058_SMPS3			53
#define QCOM_RPM_PM8058_SMPS4			54
#define QCOM_RPM_PM8821_LDO1			55
#define QCOM_RPM_PM8821_SMPS1			56
#define QCOM_RPM_PM8821_SMPS2			57
#define QCOM_RPM_PM8901_LDO0			58
#define QCOM_RPM_PM8901_LDO1			59
#define QCOM_RPM_PM8901_LDO2			60
#define QCOM_RPM_PM8901_LDO3			61
#define QCOM_RPM_PM8901_LDO4			62
#define QCOM_RPM_PM8901_LDO5			63
#define QCOM_RPM_PM8901_LDO6			64
#define QCOM_RPM_PM8901_LVS0			65
#define QCOM_RPM_PM8901_LVS1			66
#define QCOM_RPM_PM8901_LVS2			67
#define QCOM_RPM_PM8901_LVS3			68
#define QCOM_RPM_PM8901_MVS			69
#define QCOM_RPM_PM8901_SMPS0			70
#define QCOM_RPM_PM8901_SMPS1			71
#define QCOM_RPM_PM8901_SMPS2			72
#define QCOM_RPM_PM8901_SMPS3			73
#define QCOM_RPM_PM8901_SMPS4			74
#define QCOM_RPM_PM8921_CLK1			75
#define QCOM_RPM_PM8921_CLK2			76
#define QCOM_RPM_PM8921_LDO1			77
#define QCOM_RPM_PM8921_LDO2			78
#define QCOM_RPM_PM8921_LDO3			79
#define QCOM_RPM_PM8921_LDO4			80
#define QCOM_RPM_PM8921_LDO5			81
#define QCOM_RPM_PM8921_LDO6			82
#define QCOM_RPM_PM8921_LDO7			83
#define QCOM_RPM_PM8921_LDO8			84
#define QCOM_RPM_PM8921_LDO9			85
#define QCOM_RPM_PM8921_LDO10			86
#define QCOM_RPM_PM8921_LDO11			87
#define QCOM_RPM_PM8921_LDO12			88
#define QCOM_RPM_PM8921_LDO13			89
#define QCOM_RPM_PM8921_LDO14			90
#define QCOM_RPM_PM8921_LDO15			91
#define QCOM_RPM_PM8921_LDO16			92
#define QCOM_RPM_PM8921_LDO17			93
#define QCOM_RPM_PM8921_LDO18			94
#define QCOM_RPM_PM8921_LDO19			95
#define QCOM_RPM_PM8921_LDO20			96
#define QCOM_RPM_PM8921_LDO21			97
#define QCOM_RPM_PM8921_LDO22			98
#define QCOM_RPM_PM8921_LDO23			99
#define QCOM_RPM_PM8921_LDO24			100
#define QCOM_RPM_PM8921_LDO25			101
#define QCOM_RPM_PM8921_LDO26			102
#define QCOM_RPM_PM8921_LDO27			103
#define QCOM_RPM_PM8921_LDO28			104
#define QCOM_RPM_PM8921_LDO29			105
#define QCOM_RPM_PM8921_LVS1			106
#define QCOM_RPM_PM8921_LVS2			107
#define QCOM_RPM_PM8921_LVS3			108
#define QCOM_RPM_PM8921_LVS4			109
#define QCOM_RPM_PM8921_LVS5			110
#define QCOM_RPM_PM8921_LVS6			111
#define QCOM_RPM_PM8921_LVS7			112
#define QCOM_RPM_PM8921_MVS			113
#define QCOM_RPM_PM8921_NCP			114
#define QCOM_RPM_PM8921_SMPS1			115
#define QCOM_RPM_PM8921_SMPS2			116
#define QCOM_RPM_PM8921_SMPS3			117
#define QCOM_RPM_PM8921_SMPS4			118
#define QCOM_RPM_PM8921_SMPS5			119
#define QCOM_RPM_PM8921_SMPS6			120
#define QCOM_RPM_PM8921_SMPS7			121
#define QCOM_RPM_PM8921_SMPS8			122
#define QCOM_RPM_PXO_CLK			123
#define QCOM_RPM_QDSS_CLK			124
#define QCOM_RPM_SFPB_CLK			125
#define QCOM_RPM_SMI_CLK			126
#define QCOM_RPM_SYS_FABRIC_ARB			127
#define QCOM_RPM_SYS_FABRIC_CLK			128
#define QCOM_RPM_SYS_FABRIC_HALT		129
#define QCOM_RPM_SYS_FABRIC_IOCTL		130
#define QCOM_RPM_SYS_FABRIC_MODE		131
#define QCOM_RPM_USB_OTG_SWITCH			132
#define QCOM_RPM_VDDMIN_GPIO			133
#define QCOM_RPM_NSS_FABRIC_0_CLK		134
#define QCOM_RPM_NSS_FABRIC_1_CLK		135
#define QCOM_RPM_SMB208_S1a			136
#define QCOM_RPM_SMB208_S1b			137
#define QCOM_RPM_SMB208_S2a			138
#define QCOM_RPM_SMB208_S2b			139

/*
 * Constants used to select force mode for regulators.
 */
#define QCOM_RPM_FORCE_MODE_NONE		0
#define QCOM_RPM_FORCE_MODE_LPM			1
#define QCOM_RPM_FORCE_MODE_HPM			2
#define QCOM_RPM_FORCE_MODE_AUTO		3
#define QCOM_RPM_FORCE_MODE_BYPASS		4

#endif

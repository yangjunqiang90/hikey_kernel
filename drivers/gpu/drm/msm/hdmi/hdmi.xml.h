#ifndef HDMI_XML
#define HDMI_XML

/* Autogenerated file, DO NOT EDIT manually!

This file was generated by the rules-ng-ng headergen tool in this git repository:
http://github.com/freedreno/envytools/
git clone https://github.com/freedreno/envytools.git

The rules-ng-ng source files this header was generated from are:
- /home/robclark/src/freedreno/envytools/rnndb/msm.xml                 (    676 bytes, from 2014-12-05 15:34:49)
- /home/robclark/src/freedreno/envytools/rnndb/freedreno_copyright.xml (   1453 bytes, from 2013-03-31 16:51:27)
- /home/robclark/src/freedreno/envytools/rnndb/mdp/mdp4.xml            (  20908 bytes, from 2014-12-08 16:13:00)
- /home/robclark/src/freedreno/envytools/rnndb/mdp/mdp_common.xml      (   2357 bytes, from 2014-12-08 16:13:00)
- /home/robclark/src/freedreno/envytools/rnndb/mdp/mdp5.xml            (  27208 bytes, from 2015-01-13 23:56:11)
- /home/robclark/src/freedreno/envytools/rnndb/dsi/dsi.xml             (  11712 bytes, from 2013-08-17 17:13:43)
- /home/robclark/src/freedreno/envytools/rnndb/dsi/sfpb.xml            (    344 bytes, from 2013-08-11 19:26:32)
- /home/robclark/src/freedreno/envytools/rnndb/dsi/mmss_cc.xml         (   1686 bytes, from 2014-10-31 16:48:57)
- /home/robclark/src/freedreno/envytools/rnndb/hdmi/qfprom.xml         (    600 bytes, from 2013-07-05 19:21:12)
- /home/robclark/src/freedreno/envytools/rnndb/hdmi/hdmi.xml           (  26848 bytes, from 2015-01-13 23:55:57)
- /home/robclark/src/freedreno/envytools/rnndb/edp/edp.xml             (   8253 bytes, from 2014-12-08 16:13:00)

Copyright (C) 2013-2015 by the following authors:
- Rob Clark <robdclark@gmail.com> (robclark)

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


enum hdmi_hdcp_key_state {
	HDCP_KEYS_STATE_NO_KEYS = 0,
	HDCP_KEYS_STATE_NOT_CHECKED = 1,
	HDCP_KEYS_STATE_CHECKING = 2,
	HDCP_KEYS_STATE_VALID = 3,
	HDCP_KEYS_STATE_AKSV_NOT_VALID = 4,
	HDCP_KEYS_STATE_CHKSUM_MISMATCH = 5,
	HDCP_KEYS_STATE_PROD_AKSV = 6,
	HDCP_KEYS_STATE_RESERVED = 7,
};

enum hdmi_ddc_read_write {
	DDC_WRITE = 0,
	DDC_READ = 1,
};

enum hdmi_acr_cts {
	ACR_NONE = 0,
	ACR_32 = 1,
	ACR_44 = 2,
	ACR_48 = 3,
};

#define REG_HDMI_CTRL						0x00000000
#define HDMI_CTRL_ENABLE					0x00000001
#define HDMI_CTRL_HDMI						0x00000002
#define HDMI_CTRL_ENCRYPTED					0x00000004

#define REG_HDMI_AUDIO_PKT_CTRL1				0x00000020
#define HDMI_AUDIO_PKT_CTRL1_AUDIO_SAMPLE_SEND			0x00000001

#define REG_HDMI_ACR_PKT_CTRL					0x00000024
#define HDMI_ACR_PKT_CTRL_CONT					0x00000001
#define HDMI_ACR_PKT_CTRL_SEND					0x00000002
#define HDMI_ACR_PKT_CTRL_SELECT__MASK				0x00000030
#define HDMI_ACR_PKT_CTRL_SELECT__SHIFT				4
static inline uint32_t HDMI_ACR_PKT_CTRL_SELECT(enum hdmi_acr_cts val)
{
	return ((val) << HDMI_ACR_PKT_CTRL_SELECT__SHIFT) & HDMI_ACR_PKT_CTRL_SELECT__MASK;
}
#define HDMI_ACR_PKT_CTRL_SOURCE				0x00000100
#define HDMI_ACR_PKT_CTRL_N_MULTIPLIER__MASK			0x00070000
#define HDMI_ACR_PKT_CTRL_N_MULTIPLIER__SHIFT			16
static inline uint32_t HDMI_ACR_PKT_CTRL_N_MULTIPLIER(uint32_t val)
{
	return ((val) << HDMI_ACR_PKT_CTRL_N_MULTIPLIER__SHIFT) & HDMI_ACR_PKT_CTRL_N_MULTIPLIER__MASK;
}
#define HDMI_ACR_PKT_CTRL_AUDIO_PRIORITY			0x80000000

#define REG_HDMI_VBI_PKT_CTRL					0x00000028
#define HDMI_VBI_PKT_CTRL_GC_ENABLE				0x00000010
#define HDMI_VBI_PKT_CTRL_GC_EVERY_FRAME			0x00000020
#define HDMI_VBI_PKT_CTRL_ISRC_SEND				0x00000100
#define HDMI_VBI_PKT_CTRL_ISRC_CONTINUOUS			0x00000200
#define HDMI_VBI_PKT_CTRL_ACP_SEND				0x00001000
#define HDMI_VBI_PKT_CTRL_ACP_SRC_SW				0x00002000

#define REG_HDMI_INFOFRAME_CTRL0				0x0000002c
#define HDMI_INFOFRAME_CTRL0_AVI_SEND				0x00000001
#define HDMI_INFOFRAME_CTRL0_AVI_CONT				0x00000002
#define HDMI_INFOFRAME_CTRL0_AUDIO_INFO_SEND			0x00000010
#define HDMI_INFOFRAME_CTRL0_AUDIO_INFO_CONT			0x00000020
#define HDMI_INFOFRAME_CTRL0_AUDIO_INFO_SOURCE			0x00000040
#define HDMI_INFOFRAME_CTRL0_AUDIO_INFO_UPDATE			0x00000080

#define REG_HDMI_GEN_PKT_CTRL					0x00000034
#define HDMI_GEN_PKT_CTRL_GENERIC0_SEND				0x00000001
#define HDMI_GEN_PKT_CTRL_GENERIC0_CONT				0x00000002
#define HDMI_GEN_PKT_CTRL_GENERIC0_UPDATE__MASK			0x0000000c
#define HDMI_GEN_PKT_CTRL_GENERIC0_UPDATE__SHIFT		2
static inline uint32_t HDMI_GEN_PKT_CTRL_GENERIC0_UPDATE(uint32_t val)
{
	return ((val) << HDMI_GEN_PKT_CTRL_GENERIC0_UPDATE__SHIFT) & HDMI_GEN_PKT_CTRL_GENERIC0_UPDATE__MASK;
}
#define HDMI_GEN_PKT_CTRL_GENERIC1_SEND				0x00000010
#define HDMI_GEN_PKT_CTRL_GENERIC1_CONT				0x00000020
#define HDMI_GEN_PKT_CTRL_GENERIC0_LINE__MASK			0x003f0000
#define HDMI_GEN_PKT_CTRL_GENERIC0_LINE__SHIFT			16
static inline uint32_t HDMI_GEN_PKT_CTRL_GENERIC0_LINE(uint32_t val)
{
	return ((val) << HDMI_GEN_PKT_CTRL_GENERIC0_LINE__SHIFT) & HDMI_GEN_PKT_CTRL_GENERIC0_LINE__MASK;
}
#define HDMI_GEN_PKT_CTRL_GENERIC1_LINE__MASK			0x3f000000
#define HDMI_GEN_PKT_CTRL_GENERIC1_LINE__SHIFT			24
static inline uint32_t HDMI_GEN_PKT_CTRL_GENERIC1_LINE(uint32_t val)
{
	return ((val) << HDMI_GEN_PKT_CTRL_GENERIC1_LINE__SHIFT) & HDMI_GEN_PKT_CTRL_GENERIC1_LINE__MASK;
}

#define REG_HDMI_GC						0x00000040
#define HDMI_GC_MUTE						0x00000001

#define REG_HDMI_AUDIO_PKT_CTRL2				0x00000044
#define HDMI_AUDIO_PKT_CTRL2_OVERRIDE				0x00000001
#define HDMI_AUDIO_PKT_CTRL2_LAYOUT				0x00000002

static inline uint32_t REG_HDMI_AVI_INFO(uint32_t i0) { return 0x0000006c + 0x4*i0; }

#define REG_HDMI_GENERIC0_HDR					0x00000084

static inline uint32_t REG_HDMI_GENERIC0(uint32_t i0) { return 0x00000088 + 0x4*i0; }

#define REG_HDMI_GENERIC1_HDR					0x000000a4

static inline uint32_t REG_HDMI_GENERIC1(uint32_t i0) { return 0x000000a8 + 0x4*i0; }

static inline uint32_t REG_HDMI_ACR(enum hdmi_acr_cts i0) { return 0x000000c4 + 0x8*i0; }

static inline uint32_t REG_HDMI_ACR_0(enum hdmi_acr_cts i0) { return 0x000000c4 + 0x8*i0; }
#define HDMI_ACR_0_CTS__MASK					0xfffff000
#define HDMI_ACR_0_CTS__SHIFT					12
static inline uint32_t HDMI_ACR_0_CTS(uint32_t val)
{
	return ((val) << HDMI_ACR_0_CTS__SHIFT) & HDMI_ACR_0_CTS__MASK;
}

static inline uint32_t REG_HDMI_ACR_1(enum hdmi_acr_cts i0) { return 0x000000c8 + 0x8*i0; }
#define HDMI_ACR_1_N__MASK					0xffffffff
#define HDMI_ACR_1_N__SHIFT					0
static inline uint32_t HDMI_ACR_1_N(uint32_t val)
{
	return ((val) << HDMI_ACR_1_N__SHIFT) & HDMI_ACR_1_N__MASK;
}

#define REG_HDMI_AUDIO_INFO0					0x000000e4
#define HDMI_AUDIO_INFO0_CHECKSUM__MASK				0x000000ff
#define HDMI_AUDIO_INFO0_CHECKSUM__SHIFT			0
static inline uint32_t HDMI_AUDIO_INFO0_CHECKSUM(uint32_t val)
{
	return ((val) << HDMI_AUDIO_INFO0_CHECKSUM__SHIFT) & HDMI_AUDIO_INFO0_CHECKSUM__MASK;
}
#define HDMI_AUDIO_INFO0_CC__MASK				0x00000700
#define HDMI_AUDIO_INFO0_CC__SHIFT				8
static inline uint32_t HDMI_AUDIO_INFO0_CC(uint32_t val)
{
	return ((val) << HDMI_AUDIO_INFO0_CC__SHIFT) & HDMI_AUDIO_INFO0_CC__MASK;
}

#define REG_HDMI_AUDIO_INFO1					0x000000e8
#define HDMI_AUDIO_INFO1_CA__MASK				0x000000ff
#define HDMI_AUDIO_INFO1_CA__SHIFT				0
static inline uint32_t HDMI_AUDIO_INFO1_CA(uint32_t val)
{
	return ((val) << HDMI_AUDIO_INFO1_CA__SHIFT) & HDMI_AUDIO_INFO1_CA__MASK;
}
#define HDMI_AUDIO_INFO1_LSV__MASK				0x00007800
#define HDMI_AUDIO_INFO1_LSV__SHIFT				11
static inline uint32_t HDMI_AUDIO_INFO1_LSV(uint32_t val)
{
	return ((val) << HDMI_AUDIO_INFO1_LSV__SHIFT) & HDMI_AUDIO_INFO1_LSV__MASK;
}
#define HDMI_AUDIO_INFO1_DM_INH					0x00008000

#define REG_HDMI_HDCP_CTRL					0x00000110
#define HDMI_HDCP_CTRL_ENABLE					0x00000001
#define HDMI_HDCP_CTRL_ENCRYPTION_ENABLE			0x00000100

#define REG_HDMI_HDCP_DEBUG_CTRL				0x00000114
#define HDMI_HDCP_DEBUG_CTRL_RNG_CIPHER				0x00000004

#define REG_HDMI_HDCP_INT_CTRL					0x00000118
#define HDMI_HDCP_INT_CTRL_AUTH_SUCCESS_INT			0x00000001
#define HDMI_HDCP_INT_CTRL_AUTH_SUCCESS_ACK			0x00000002
#define HDMI_HDCP_INT_CTRL_AUTH_SUCCESS_MASK			0x00000004
#define HDMI_HDCP_INT_CTRL_AUTH_FAIL_INT			0x00000010
#define HDMI_HDCP_INT_CTRL_AUTH_FAIL_ACK			0x00000020
#define HDMI_HDCP_INT_CTRL_AUTH_FAIL_MASK			0x00000040
#define HDMI_HDCP_INT_CTRL_AUTH_FAIL_INFO_ACK			0x00000080
#define HDMI_HDCP_INT_CTRL_AUTH_XFER_REQ_INT			0x00000100
#define HDMI_HDCP_INT_CTRL_AUTH_XFER_REQ_ACK			0x00000200
#define HDMI_HDCP_INT_CTRL_AUTH_XFER_REQ_MASK			0x00000400
#define HDMI_HDCP_INT_CTRL_AUTH_XFER_DONE_INT			0x00001000
#define HDMI_HDCP_INT_CTRL_AUTH_XFER_DONE_ACK			0x00002000
#define HDMI_HDCP_INT_CTRL_AUTH_XFER_DONE_MASK			0x00004000

#define REG_HDMI_HDCP_LINK0_STATUS				0x0000011c
#define HDMI_HDCP_LINK0_STATUS_AN_0_READY			0x00000100
#define HDMI_HDCP_LINK0_STATUS_AN_1_READY			0x00000200
#define HDMI_HDCP_LINK0_STATUS_RI_MATCHES			0x00001000
#define HDMI_HDCP_LINK0_STATUS_V_MATCHES			0x00100000
#define HDMI_HDCP_LINK0_STATUS_KEY_STATE__MASK			0x70000000
#define HDMI_HDCP_LINK0_STATUS_KEY_STATE__SHIFT			28
static inline uint32_t HDMI_HDCP_LINK0_STATUS_KEY_STATE(enum hdmi_hdcp_key_state val)
{
	return ((val) << HDMI_HDCP_LINK0_STATUS_KEY_STATE__SHIFT) & HDMI_HDCP_LINK0_STATUS_KEY_STATE__MASK;
}

#define REG_HDMI_HDCP_DDC_CTRL_0				0x00000120
#define HDMI_HDCP_DDC_CTRL_0_DISABLE				0x00000001

#define REG_HDMI_HDCP_DDC_CTRL_1				0x00000124
#define HDMI_HDCP_DDC_CTRL_1_FAILED_ACK				0x00000001

#define REG_HDMI_HDCP_DDC_STATUS				0x00000128
#define HDMI_HDCP_DDC_STATUS_XFER_REQ				0x00000010
#define HDMI_HDCP_DDC_STATUS_XFER_DONE				0x00000400
#define HDMI_HDCP_DDC_STATUS_ABORTED				0x00001000
#define HDMI_HDCP_DDC_STATUS_TIMEOUT				0x00002000
#define HDMI_HDCP_DDC_STATUS_NACK0				0x00004000
#define HDMI_HDCP_DDC_STATUS_NACK1				0x00008000
#define HDMI_HDCP_DDC_STATUS_FAILED				0x00010000

#define REG_HDMI_HDCP_ENTROPY_CTRL0				0x0000012c

#define REG_HDMI_HDCP_ENTROPY_CTRL1				0x0000025c

#define REG_HDMI_HDCP_RESET					0x00000130
#define HDMI_HDCP_RESET_LINK0_DEAUTHENTICATE			0x00000001

#define REG_HDMI_HDCP_RCVPORT_DATA0				0x00000134

#define REG_HDMI_HDCP_RCVPORT_DATA1				0x00000138

#define REG_HDMI_HDCP_RCVPORT_DATA2_0				0x0000013c

#define REG_HDMI_HDCP_RCVPORT_DATA2_1				0x00000140

#define REG_HDMI_HDCP_RCVPORT_DATA3				0x00000144

#define REG_HDMI_HDCP_RCVPORT_DATA4				0x00000148

#define REG_HDMI_HDCP_RCVPORT_DATA5				0x0000014c

#define REG_HDMI_HDCP_RCVPORT_DATA6				0x00000150

#define REG_HDMI_HDCP_RCVPORT_DATA7				0x00000154

#define REG_HDMI_HDCP_RCVPORT_DATA8				0x00000158

#define REG_HDMI_HDCP_RCVPORT_DATA9				0x0000015c

#define REG_HDMI_HDCP_RCVPORT_DATA10				0x00000160

#define REG_HDMI_HDCP_RCVPORT_DATA11				0x00000164

#define REG_HDMI_HDCP_RCVPORT_DATA12				0x00000168

#define REG_HDMI_VENSPEC_INFO0					0x0000016c

#define REG_HDMI_VENSPEC_INFO1					0x00000170

#define REG_HDMI_VENSPEC_INFO2					0x00000174

#define REG_HDMI_VENSPEC_INFO3					0x00000178

#define REG_HDMI_VENSPEC_INFO4					0x0000017c

#define REG_HDMI_VENSPEC_INFO5					0x00000180

#define REG_HDMI_VENSPEC_INFO6					0x00000184

#define REG_HDMI_AUDIO_CFG					0x000001d0
#define HDMI_AUDIO_CFG_ENGINE_ENABLE				0x00000001
#define HDMI_AUDIO_CFG_FIFO_WATERMARK__MASK			0x000000f0
#define HDMI_AUDIO_CFG_FIFO_WATERMARK__SHIFT			4
static inline uint32_t HDMI_AUDIO_CFG_FIFO_WATERMARK(uint32_t val)
{
	return ((val) << HDMI_AUDIO_CFG_FIFO_WATERMARK__SHIFT) & HDMI_AUDIO_CFG_FIFO_WATERMARK__MASK;
}

#define REG_HDMI_USEC_REFTIMER					0x00000208

#define REG_HDMI_DDC_CTRL					0x0000020c
#define HDMI_DDC_CTRL_GO					0x00000001
#define HDMI_DDC_CTRL_SOFT_RESET				0x00000002
#define HDMI_DDC_CTRL_SEND_RESET				0x00000004
#define HDMI_DDC_CTRL_SW_STATUS_RESET				0x00000008
#define HDMI_DDC_CTRL_TRANSACTION_CNT__MASK			0x00300000
#define HDMI_DDC_CTRL_TRANSACTION_CNT__SHIFT			20
static inline uint32_t HDMI_DDC_CTRL_TRANSACTION_CNT(uint32_t val)
{
	return ((val) << HDMI_DDC_CTRL_TRANSACTION_CNT__SHIFT) & HDMI_DDC_CTRL_TRANSACTION_CNT__MASK;
}

#define REG_HDMI_DDC_ARBITRATION				0x00000210
#define HDMI_DDC_ARBITRATION_HW_ARBITRATION			0x00000010

#define REG_HDMI_DDC_INT_CTRL					0x00000214
#define HDMI_DDC_INT_CTRL_SW_DONE_INT				0x00000001
#define HDMI_DDC_INT_CTRL_SW_DONE_ACK				0x00000002
#define HDMI_DDC_INT_CTRL_SW_DONE_MASK				0x00000004

#define REG_HDMI_DDC_SW_STATUS					0x00000218
#define HDMI_DDC_SW_STATUS_NACK0				0x00001000
#define HDMI_DDC_SW_STATUS_NACK1				0x00002000
#define HDMI_DDC_SW_STATUS_NACK2				0x00004000
#define HDMI_DDC_SW_STATUS_NACK3				0x00008000

#define REG_HDMI_DDC_HW_STATUS					0x0000021c
#define HDMI_DDC_HW_STATUS_DONE					0x00000008

#define REG_HDMI_DDC_SPEED					0x00000220
#define HDMI_DDC_SPEED_THRESHOLD__MASK				0x00000003
#define HDMI_DDC_SPEED_THRESHOLD__SHIFT				0
static inline uint32_t HDMI_DDC_SPEED_THRESHOLD(uint32_t val)
{
	return ((val) << HDMI_DDC_SPEED_THRESHOLD__SHIFT) & HDMI_DDC_SPEED_THRESHOLD__MASK;
}
#define HDMI_DDC_SPEED_PRESCALE__MASK				0xffff0000
#define HDMI_DDC_SPEED_PRESCALE__SHIFT				16
static inline uint32_t HDMI_DDC_SPEED_PRESCALE(uint32_t val)
{
	return ((val) << HDMI_DDC_SPEED_PRESCALE__SHIFT) & HDMI_DDC_SPEED_PRESCALE__MASK;
}

#define REG_HDMI_DDC_SETUP					0x00000224
#define HDMI_DDC_SETUP_TIMEOUT__MASK				0xff000000
#define HDMI_DDC_SETUP_TIMEOUT__SHIFT				24
static inline uint32_t HDMI_DDC_SETUP_TIMEOUT(uint32_t val)
{
	return ((val) << HDMI_DDC_SETUP_TIMEOUT__SHIFT) & HDMI_DDC_SETUP_TIMEOUT__MASK;
}

static inline uint32_t REG_HDMI_I2C_TRANSACTION(uint32_t i0) { return 0x00000228 + 0x4*i0; }

static inline uint32_t REG_HDMI_I2C_TRANSACTION_REG(uint32_t i0) { return 0x00000228 + 0x4*i0; }
#define HDMI_I2C_TRANSACTION_REG_RW__MASK			0x00000001
#define HDMI_I2C_TRANSACTION_REG_RW__SHIFT			0
static inline uint32_t HDMI_I2C_TRANSACTION_REG_RW(enum hdmi_ddc_read_write val)
{
	return ((val) << HDMI_I2C_TRANSACTION_REG_RW__SHIFT) & HDMI_I2C_TRANSACTION_REG_RW__MASK;
}
#define HDMI_I2C_TRANSACTION_REG_STOP_ON_NACK			0x00000100
#define HDMI_I2C_TRANSACTION_REG_START				0x00001000
#define HDMI_I2C_TRANSACTION_REG_STOP				0x00002000
#define HDMI_I2C_TRANSACTION_REG_CNT__MASK			0x00ff0000
#define HDMI_I2C_TRANSACTION_REG_CNT__SHIFT			16
static inline uint32_t HDMI_I2C_TRANSACTION_REG_CNT(uint32_t val)
{
	return ((val) << HDMI_I2C_TRANSACTION_REG_CNT__SHIFT) & HDMI_I2C_TRANSACTION_REG_CNT__MASK;
}

#define REG_HDMI_DDC_DATA					0x00000238
#define HDMI_DDC_DATA_DATA_RW__MASK				0x00000001
#define HDMI_DDC_DATA_DATA_RW__SHIFT				0
static inline uint32_t HDMI_DDC_DATA_DATA_RW(enum hdmi_ddc_read_write val)
{
	return ((val) << HDMI_DDC_DATA_DATA_RW__SHIFT) & HDMI_DDC_DATA_DATA_RW__MASK;
}
#define HDMI_DDC_DATA_DATA__MASK				0x0000ff00
#define HDMI_DDC_DATA_DATA__SHIFT				8
static inline uint32_t HDMI_DDC_DATA_DATA(uint32_t val)
{
	return ((val) << HDMI_DDC_DATA_DATA__SHIFT) & HDMI_DDC_DATA_DATA__MASK;
}
#define HDMI_DDC_DATA_INDEX__MASK				0x00ff0000
#define HDMI_DDC_DATA_INDEX__SHIFT				16
static inline uint32_t HDMI_DDC_DATA_INDEX(uint32_t val)
{
	return ((val) << HDMI_DDC_DATA_INDEX__SHIFT) & HDMI_DDC_DATA_INDEX__MASK;
}
#define HDMI_DDC_DATA_INDEX_WRITE				0x80000000

#define REG_HDMI_HDCP_SHA_CTRL					0x0000023c

#define REG_HDMI_HDCP_SHA_STATUS				0x00000240
#define HDMI_HDCP_SHA_STATUS_BLOCK_DONE				0x00000001
#define HDMI_HDCP_SHA_STATUS_COMP_DONE				0x00000010

#define REG_HDMI_HDCP_SHA_DATA					0x00000244
#define HDMI_HDCP_SHA_DATA_DONE					0x00000001

#define REG_HDMI_HPD_INT_STATUS					0x00000250
#define HDMI_HPD_INT_STATUS_INT					0x00000001
#define HDMI_HPD_INT_STATUS_CABLE_DETECTED			0x00000002

#define REG_HDMI_HPD_INT_CTRL					0x00000254
#define HDMI_HPD_INT_CTRL_INT_ACK				0x00000001
#define HDMI_HPD_INT_CTRL_INT_CONNECT				0x00000002
#define HDMI_HPD_INT_CTRL_INT_EN				0x00000004
#define HDMI_HPD_INT_CTRL_RX_INT_ACK				0x00000010
#define HDMI_HPD_INT_CTRL_RX_INT_EN				0x00000020
#define HDMI_HPD_INT_CTRL_RCV_PLUGIN_DET_MASK			0x00000200

#define REG_HDMI_HPD_CTRL					0x00000258
#define HDMI_HPD_CTRL_TIMEOUT__MASK				0x00001fff
#define HDMI_HPD_CTRL_TIMEOUT__SHIFT				0
static inline uint32_t HDMI_HPD_CTRL_TIMEOUT(uint32_t val)
{
	return ((val) << HDMI_HPD_CTRL_TIMEOUT__SHIFT) & HDMI_HPD_CTRL_TIMEOUT__MASK;
}
#define HDMI_HPD_CTRL_ENABLE					0x10000000

#define REG_HDMI_DDC_REF					0x0000027c
#define HDMI_DDC_REF_REFTIMER_ENABLE				0x00010000
#define HDMI_DDC_REF_REFTIMER__MASK				0x0000ffff
#define HDMI_DDC_REF_REFTIMER__SHIFT				0
static inline uint32_t HDMI_DDC_REF_REFTIMER(uint32_t val)
{
	return ((val) << HDMI_DDC_REF_REFTIMER__SHIFT) & HDMI_DDC_REF_REFTIMER__MASK;
}

#define REG_HDMI_HDCP_SW_UPPER_AKSV				0x00000284

#define REG_HDMI_HDCP_SW_LOWER_AKSV				0x00000288

#define REG_HDMI_CEC_STATUS					0x00000298

#define REG_HDMI_CEC_INT					0x0000029c

#define REG_HDMI_CEC_ADDR					0x000002a0

#define REG_HDMI_CEC_TIME					0x000002a4

#define REG_HDMI_CEC_REFTIMER					0x000002a8

#define REG_HDMI_CEC_RD_DATA					0x000002ac

#define REG_HDMI_CEC_RD_FILTER					0x000002b0

#define REG_HDMI_ACTIVE_HSYNC					0x000002b4
#define HDMI_ACTIVE_HSYNC_START__MASK				0x00000fff
#define HDMI_ACTIVE_HSYNC_START__SHIFT				0
static inline uint32_t HDMI_ACTIVE_HSYNC_START(uint32_t val)
{
	return ((val) << HDMI_ACTIVE_HSYNC_START__SHIFT) & HDMI_ACTIVE_HSYNC_START__MASK;
}
#define HDMI_ACTIVE_HSYNC_END__MASK				0x0fff0000
#define HDMI_ACTIVE_HSYNC_END__SHIFT				16
static inline uint32_t HDMI_ACTIVE_HSYNC_END(uint32_t val)
{
	return ((val) << HDMI_ACTIVE_HSYNC_END__SHIFT) & HDMI_ACTIVE_HSYNC_END__MASK;
}

#define REG_HDMI_ACTIVE_VSYNC					0x000002b8
#define HDMI_ACTIVE_VSYNC_START__MASK				0x00000fff
#define HDMI_ACTIVE_VSYNC_START__SHIFT				0
static inline uint32_t HDMI_ACTIVE_VSYNC_START(uint32_t val)
{
	return ((val) << HDMI_ACTIVE_VSYNC_START__SHIFT) & HDMI_ACTIVE_VSYNC_START__MASK;
}
#define HDMI_ACTIVE_VSYNC_END__MASK				0x0fff0000
#define HDMI_ACTIVE_VSYNC_END__SHIFT				16
static inline uint32_t HDMI_ACTIVE_VSYNC_END(uint32_t val)
{
	return ((val) << HDMI_ACTIVE_VSYNC_END__SHIFT) & HDMI_ACTIVE_VSYNC_END__MASK;
}

#define REG_HDMI_VSYNC_ACTIVE_F2				0x000002bc
#define HDMI_VSYNC_ACTIVE_F2_START__MASK			0x00000fff
#define HDMI_VSYNC_ACTIVE_F2_START__SHIFT			0
static inline uint32_t HDMI_VSYNC_ACTIVE_F2_START(uint32_t val)
{
	return ((val) << HDMI_VSYNC_ACTIVE_F2_START__SHIFT) & HDMI_VSYNC_ACTIVE_F2_START__MASK;
}
#define HDMI_VSYNC_ACTIVE_F2_END__MASK				0x0fff0000
#define HDMI_VSYNC_ACTIVE_F2_END__SHIFT				16
static inline uint32_t HDMI_VSYNC_ACTIVE_F2_END(uint32_t val)
{
	return ((val) << HDMI_VSYNC_ACTIVE_F2_END__SHIFT) & HDMI_VSYNC_ACTIVE_F2_END__MASK;
}

#define REG_HDMI_TOTAL						0x000002c0
#define HDMI_TOTAL_H_TOTAL__MASK				0x00000fff
#define HDMI_TOTAL_H_TOTAL__SHIFT				0
static inline uint32_t HDMI_TOTAL_H_TOTAL(uint32_t val)
{
	return ((val) << HDMI_TOTAL_H_TOTAL__SHIFT) & HDMI_TOTAL_H_TOTAL__MASK;
}
#define HDMI_TOTAL_V_TOTAL__MASK				0x0fff0000
#define HDMI_TOTAL_V_TOTAL__SHIFT				16
static inline uint32_t HDMI_TOTAL_V_TOTAL(uint32_t val)
{
	return ((val) << HDMI_TOTAL_V_TOTAL__SHIFT) & HDMI_TOTAL_V_TOTAL__MASK;
}

#define REG_HDMI_VSYNC_TOTAL_F2					0x000002c4
#define HDMI_VSYNC_TOTAL_F2_V_TOTAL__MASK			0x00000fff
#define HDMI_VSYNC_TOTAL_F2_V_TOTAL__SHIFT			0
static inline uint32_t HDMI_VSYNC_TOTAL_F2_V_TOTAL(uint32_t val)
{
	return ((val) << HDMI_VSYNC_TOTAL_F2_V_TOTAL__SHIFT) & HDMI_VSYNC_TOTAL_F2_V_TOTAL__MASK;
}

#define REG_HDMI_FRAME_CTRL					0x000002c8
#define HDMI_FRAME_CTRL_RGB_MUX_SEL_BGR				0x00001000
#define HDMI_FRAME_CTRL_VSYNC_LOW				0x10000000
#define HDMI_FRAME_CTRL_HSYNC_LOW				0x20000000
#define HDMI_FRAME_CTRL_INTERLACED_EN				0x80000000

#define REG_HDMI_AUD_INT					0x000002cc
#define HDMI_AUD_INT_AUD_FIFO_URUN_INT				0x00000001
#define HDMI_AUD_INT_AUD_FIFO_URAN_MASK				0x00000002
#define HDMI_AUD_INT_AUD_SAM_DROP_INT				0x00000004
#define HDMI_AUD_INT_AUD_SAM_DROP_MASK				0x00000008

#define REG_HDMI_PHY_CTRL					0x000002d4
#define HDMI_PHY_CTRL_SW_RESET_PLL				0x00000001
#define HDMI_PHY_CTRL_SW_RESET_PLL_LOW				0x00000002
#define HDMI_PHY_CTRL_SW_RESET					0x00000004
#define HDMI_PHY_CTRL_SW_RESET_LOW				0x00000008

#define REG_HDMI_CEC_WR_RANGE					0x000002dc

#define REG_HDMI_CEC_RD_RANGE					0x000002e0

#define REG_HDMI_VERSION					0x000002e4

#define REG_HDMI_CEC_COMPL_CTL					0x00000360

#define REG_HDMI_CEC_RD_START_RANGE				0x00000364

#define REG_HDMI_CEC_RD_TOTAL_RANGE				0x00000368

#define REG_HDMI_CEC_RD_ERR_RESP_LO				0x0000036c

#define REG_HDMI_CEC_WR_CHECK_CONFIG				0x00000370

#define REG_HDMI_8x60_PHY_REG0					0x00000300
#define HDMI_8x60_PHY_REG0_DESER_DEL_CTRL__MASK			0x0000001c
#define HDMI_8x60_PHY_REG0_DESER_DEL_CTRL__SHIFT		2
static inline uint32_t HDMI_8x60_PHY_REG0_DESER_DEL_CTRL(uint32_t val)
{
	return ((val) << HDMI_8x60_PHY_REG0_DESER_DEL_CTRL__SHIFT) & HDMI_8x60_PHY_REG0_DESER_DEL_CTRL__MASK;
}

#define REG_HDMI_8x60_PHY_REG1					0x00000304
#define HDMI_8x60_PHY_REG1_DTEST_MUX_SEL__MASK			0x000000f0
#define HDMI_8x60_PHY_REG1_DTEST_MUX_SEL__SHIFT			4
static inline uint32_t HDMI_8x60_PHY_REG1_DTEST_MUX_SEL(uint32_t val)
{
	return ((val) << HDMI_8x60_PHY_REG1_DTEST_MUX_SEL__SHIFT) & HDMI_8x60_PHY_REG1_DTEST_MUX_SEL__MASK;
}
#define HDMI_8x60_PHY_REG1_OUTVOL_SWING_CTRL__MASK		0x0000000f
#define HDMI_8x60_PHY_REG1_OUTVOL_SWING_CTRL__SHIFT		0
static inline uint32_t HDMI_8x60_PHY_REG1_OUTVOL_SWING_CTRL(uint32_t val)
{
	return ((val) << HDMI_8x60_PHY_REG1_OUTVOL_SWING_CTRL__SHIFT) & HDMI_8x60_PHY_REG1_OUTVOL_SWING_CTRL__MASK;
}

#define REG_HDMI_8x60_PHY_REG2					0x00000308
#define HDMI_8x60_PHY_REG2_PD_DESER				0x00000001
#define HDMI_8x60_PHY_REG2_PD_DRIVE_1				0x00000002
#define HDMI_8x60_PHY_REG2_PD_DRIVE_2				0x00000004
#define HDMI_8x60_PHY_REG2_PD_DRIVE_3				0x00000008
#define HDMI_8x60_PHY_REG2_PD_DRIVE_4				0x00000010
#define HDMI_8x60_PHY_REG2_PD_PLL				0x00000020
#define HDMI_8x60_PHY_REG2_PD_PWRGEN				0x00000040
#define HDMI_8x60_PHY_REG2_RCV_SENSE_EN				0x00000080

#define REG_HDMI_8x60_PHY_REG3					0x0000030c
#define HDMI_8x60_PHY_REG3_PLL_ENABLE				0x00000001

#define REG_HDMI_8x60_PHY_REG4					0x00000310

#define REG_HDMI_8x60_PHY_REG5					0x00000314

#define REG_HDMI_8x60_PHY_REG6					0x00000318

#define REG_HDMI_8x60_PHY_REG7					0x0000031c

#define REG_HDMI_8x60_PHY_REG8					0x00000320

#define REG_HDMI_8x60_PHY_REG9					0x00000324

#define REG_HDMI_8x60_PHY_REG10					0x00000328

#define REG_HDMI_8x60_PHY_REG11					0x0000032c

#define REG_HDMI_8x60_PHY_REG12					0x00000330
#define HDMI_8x60_PHY_REG12_RETIMING_EN				0x00000001
#define HDMI_8x60_PHY_REG12_PLL_LOCK_DETECT_EN			0x00000002
#define HDMI_8x60_PHY_REG12_FORCE_LOCK				0x00000010

#define REG_HDMI_8960_PHY_REG0					0x00000400

#define REG_HDMI_8960_PHY_REG1					0x00000404

#define REG_HDMI_8960_PHY_REG2					0x00000408

#define REG_HDMI_8960_PHY_REG3					0x0000040c

#define REG_HDMI_8960_PHY_REG4					0x00000410

#define REG_HDMI_8960_PHY_REG5					0x00000414

#define REG_HDMI_8960_PHY_REG6					0x00000418

#define REG_HDMI_8960_PHY_REG7					0x0000041c

#define REG_HDMI_8960_PHY_REG8					0x00000420

#define REG_HDMI_8960_PHY_REG9					0x00000424

#define REG_HDMI_8960_PHY_REG10					0x00000428

#define REG_HDMI_8960_PHY_REG11					0x0000042c

#define REG_HDMI_8960_PHY_REG12					0x00000430
#define HDMI_8960_PHY_REG12_SW_RESET				0x00000020
#define HDMI_8960_PHY_REG12_PWRDN_B				0x00000080

#define REG_HDMI_8960_PHY_REG_BIST_CFG				0x00000434

#define REG_HDMI_8960_PHY_DEBUG_BUS_SEL				0x00000438

#define REG_HDMI_8960_PHY_REG_MISC0				0x0000043c

#define REG_HDMI_8960_PHY_REG13					0x00000440

#define REG_HDMI_8960_PHY_REG14					0x00000444

#define REG_HDMI_8960_PHY_REG15					0x00000448

#define REG_HDMI_8960_PHY_PLL_REFCLK_CFG			0x00000500

#define REG_HDMI_8960_PHY_PLL_CHRG_PUMP_CFG			0x00000504

#define REG_HDMI_8960_PHY_PLL_LOOP_FLT_CFG0			0x00000508

#define REG_HDMI_8960_PHY_PLL_LOOP_FLT_CFG1			0x0000050c

#define REG_HDMI_8960_PHY_PLL_IDAC_ADJ_CFG			0x00000510

#define REG_HDMI_8960_PHY_PLL_I_VI_KVCO_CFG			0x00000514

#define REG_HDMI_8960_PHY_PLL_PWRDN_B				0x00000518
#define HDMI_8960_PHY_PLL_PWRDN_B_PD_PLL			0x00000002
#define HDMI_8960_PHY_PLL_PWRDN_B_PLL_PWRDN_B			0x00000008

#define REG_HDMI_8960_PHY_PLL_SDM_CFG0				0x0000051c

#define REG_HDMI_8960_PHY_PLL_SDM_CFG1				0x00000520

#define REG_HDMI_8960_PHY_PLL_SDM_CFG2				0x00000524

#define REG_HDMI_8960_PHY_PLL_SDM_CFG3				0x00000528

#define REG_HDMI_8960_PHY_PLL_SDM_CFG4				0x0000052c

#define REG_HDMI_8960_PHY_PLL_SSC_CFG0				0x00000530

#define REG_HDMI_8960_PHY_PLL_SSC_CFG1				0x00000534

#define REG_HDMI_8960_PHY_PLL_SSC_CFG2				0x00000538

#define REG_HDMI_8960_PHY_PLL_SSC_CFG3				0x0000053c

#define REG_HDMI_8960_PHY_PLL_LOCKDET_CFG0			0x00000540

#define REG_HDMI_8960_PHY_PLL_LOCKDET_CFG1			0x00000544

#define REG_HDMI_8960_PHY_PLL_LOCKDET_CFG2			0x00000548

#define REG_HDMI_8960_PHY_PLL_VCOCAL_CFG0			0x0000054c

#define REG_HDMI_8960_PHY_PLL_VCOCAL_CFG1			0x00000550

#define REG_HDMI_8960_PHY_PLL_VCOCAL_CFG2			0x00000554

#define REG_HDMI_8960_PHY_PLL_VCOCAL_CFG3			0x00000558

#define REG_HDMI_8960_PHY_PLL_VCOCAL_CFG4			0x0000055c

#define REG_HDMI_8960_PHY_PLL_VCOCAL_CFG5			0x00000560

#define REG_HDMI_8960_PHY_PLL_VCOCAL_CFG6			0x00000564

#define REG_HDMI_8960_PHY_PLL_VCOCAL_CFG7			0x00000568

#define REG_HDMI_8960_PHY_PLL_DEBUG_SEL				0x0000056c

#define REG_HDMI_8960_PHY_PLL_MISC0				0x00000570

#define REG_HDMI_8960_PHY_PLL_MISC1				0x00000574

#define REG_HDMI_8960_PHY_PLL_MISC2				0x00000578

#define REG_HDMI_8960_PHY_PLL_MISC3				0x0000057c

#define REG_HDMI_8960_PHY_PLL_MISC4				0x00000580

#define REG_HDMI_8960_PHY_PLL_MISC5				0x00000584

#define REG_HDMI_8960_PHY_PLL_MISC6				0x00000588

#define REG_HDMI_8960_PHY_PLL_DEBUG_BUS0			0x0000058c

#define REG_HDMI_8960_PHY_PLL_DEBUG_BUS1			0x00000590

#define REG_HDMI_8960_PHY_PLL_DEBUG_BUS2			0x00000594

#define REG_HDMI_8960_PHY_PLL_STATUS0				0x00000598
#define HDMI_8960_PHY_PLL_STATUS0_PLL_LOCK			0x00000001

#define REG_HDMI_8960_PHY_PLL_STATUS1				0x0000059c

#define REG_HDMI_8x74_ANA_CFG0					0x00000000

#define REG_HDMI_8x74_ANA_CFG1					0x00000004

#define REG_HDMI_8x74_PD_CTRL0					0x00000010

#define REG_HDMI_8x74_PD_CTRL1					0x00000014

#define REG_HDMI_8x74_BIST_CFG0					0x00000034

#define REG_HDMI_8x74_BIST_PATN0				0x0000003c

#define REG_HDMI_8x74_BIST_PATN1				0x00000040

#define REG_HDMI_8x74_BIST_PATN2				0x00000044

#define REG_HDMI_8x74_BIST_PATN3				0x00000048


#endif /* HDMI_XML */

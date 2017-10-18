#ifndef __DT_BINDINGS_DMA_JZ4780_DMA_H__
#define __DT_BINDINGS_DMA_JZ4780_DMA_H__

/*
 * Request type numbers for the JZ4780 DMA controller (written to the DRTn
 * register for the channel).
 */
#define JZ4780_DMA_I2S1_TX	0x4
#define JZ4780_DMA_I2S1_RX	0x5
#define JZ4780_DMA_I2S0_TX	0x6
#define JZ4780_DMA_I2S0_RX	0x7
#define JZ4780_DMA_AUTO		0x8
#define JZ4780_DMA_SADC_RX	0x9
#define JZ4780_DMA_UART4_TX	0xc
#define JZ4780_DMA_UART4_RX	0xd
#define JZ4780_DMA_UART3_TX	0xe
#define JZ4780_DMA_UART3_RX	0xf
#define JZ4780_DMA_UART2_TX	0x10
#define JZ4780_DMA_UART2_RX	0x11
#define JZ4780_DMA_UART1_TX	0x12
#define JZ4780_DMA_UART1_RX	0x13
#define JZ4780_DMA_UART0_TX	0x14
#define JZ4780_DMA_UART0_RX	0x15
#define JZ4780_DMA_SSI0_TX	0x16
#define JZ4780_DMA_SSI0_RX	0x17
#define JZ4780_DMA_SSI1_TX	0x18
#define JZ4780_DMA_SSI1_RX	0x19
#define JZ4780_DMA_MSC0_TX	0x1a
#define JZ4780_DMA_MSC0_RX	0x1b
#define JZ4780_DMA_MSC1_TX	0x1c
#define JZ4780_DMA_MSC1_RX	0x1d
#define JZ4780_DMA_MSC2_TX	0x1e
#define JZ4780_DMA_MSC2_RX	0x1f
#define JZ4780_DMA_PCM0_TX	0x20
#define JZ4780_DMA_PCM0_RX	0x21
#define JZ4780_DMA_SMB0_TX	0x24
#define JZ4780_DMA_SMB0_RX	0x25
#define JZ4780_DMA_SMB1_TX	0x26
#define JZ4780_DMA_SMB1_RX	0x27
#define JZ4780_DMA_SMB2_TX	0x28
#define JZ4780_DMA_SMB2_RX	0x29
#define JZ4780_DMA_SMB3_TX	0x2a
#define JZ4780_DMA_SMB3_RX	0x2b
#define JZ4780_DMA_SMB4_TX	0x2c
#define JZ4780_DMA_SMB4_RX	0x2d
#define JZ4780_DMA_DES_TX	0x2e
#define JZ4780_DMA_DES_RX	0x2f

#endif /* __DT_BINDINGS_DMA_JZ4780_DMA_H__ */

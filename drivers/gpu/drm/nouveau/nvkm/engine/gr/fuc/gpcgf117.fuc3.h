uint32_t gf117_grgpc_data[] = {
/* 0x0000: gpc_mmio_list_head */
	0x0000006c,
/* 0x0004: gpc_mmio_list_tail */
/* 0x0004: tpc_mmio_list_head */
	0x0000006c,
/* 0x0008: tpc_mmio_list_tail */
/* 0x0008: unk_mmio_list_head */
	0x0000006c,
/* 0x000c: unk_mmio_list_tail */
	0x0000006c,
/* 0x0010: gpc_id */
	0x00000000,
/* 0x0014: tpc_count */
	0x00000000,
/* 0x0018: tpc_mask */
	0x00000000,
/* 0x001c: unk_count */
	0x00000000,
/* 0x0020: unk_mask */
	0x00000000,
/* 0x0024: cmd_queue */
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
};

uint32_t gf117_grgpc_code[] = {
	0x03a10ef5,
/* 0x0004: queue_put */
	0x9800d898,
	0x86f001d9,
	0x0489b808,
	0xf00c1bf4,
	0x21f502f7,
	0x00f8037e,
/* 0x001c: queue_put_next */
	0xb60798c4,
	0x8dbb0384,
	0x0880b600,
	0x80008e80,
	0x90b6018f,
	0x0f94f001,
	0xf801d980,
/* 0x0039: queue_get */
	0x0131f400,
	0x9800d898,
	0x89b801d9,
	0x210bf404,
	0xb60789c4,
	0x9dbb0394,
	0x0890b600,
	0x98009e98,
	0x80b6019f,
	0x0f84f001,
	0xf400d880,
/* 0x0066: queue_get_done */
	0x00f80132,
/* 0x0068: nv_rd32 */
	0xf002ecb9,
	0x07f11fc9,
	0x03f0ca00,
	0x000cd001,
/* 0x007a: nv_rd32_wait */
	0xc7f104bd,
	0xc3f0ca00,
	0x00cccf01,
	0xf41fccc8,
	0xa7f0f31b,
	0x1021f506,
	0x00f7f101,
	0x01f3f0cb,
	0xf800ffcf,
/* 0x009d: nv_wr32 */
	0x0007f100,
	0x0103f0cc,
	0xbd000fd0,
	0x02ecb904,
	0xf01fc9f0,
	0x07f11ec9,
	0x03f0ca00,
	0x000cd001,
/* 0x00be: nv_wr32_wait */
	0xc7f104bd,
	0xc3f0ca00,
	0x00cccf01,
	0xf41fccc8,
	0x00f8f31b,
/* 0x00d0: wait_donez */
	0x99f094bd,
	0x0007f100,
	0x0203f00f,
	0xbd0009d0,
	0x0007f104,
	0x0203f006,
	0xbd000ad0,
/* 0x00ed: wait_donez_ne */
	0x0087f104,
	0x0183f000,
	0xff0088cf,
	0x1bf4888a,
	0xf094bdf3,
	0x07f10099,
	0x03f01700,
	0x0009d002,
	0x00f804bd,
/* 0x0110: wait_doneo */
	0x99f094bd,
	0x0007f100,
	0x0203f00f,
	0xbd0009d0,
	0x0007f104,
	0x0203f006,
	0xbd000ad0,
/* 0x012d: wait_doneo_e */
	0x0087f104,
	0x0183f000,
	0xff0088cf,
	0x0bf4888a,
	0xf094bdf3,
	0x07f10099,
	0x03f01700,
	0x0009d002,
	0x00f804bd,
/* 0x0150: mmctx_size */
/* 0x0152: nv_mmctx_size_loop */
	0xe89894bd,
	0x1a85b600,
	0xb60180b6,
	0x98bb0284,
	0x04e0b600,
	0xf404efb8,
	0x9fb9eb1b,
/* 0x016f: mmctx_xfer */
	0xbd00f802,
	0x0199f094,
	0x0f0007f1,
	0xd00203f0,
	0x04bd0009,
	0xbbfd94bd,
	0x120bf405,
	0xc40007f1,
	0xd00103f0,
	0x04bd000b,
/* 0x0197: mmctx_base_disabled */
	0xfd0099f0,
	0x0bf405ee,
	0x0007f11e,
	0x0103f0c6,
	0xbd000ed0,
	0x0007f104,
	0x0103f0c7,
	0xbd000fd0,
	0x0199f004,
/* 0x01b8: mmctx_multi_disabled */
	0xb600abc8,
	0xb9f010b4,
	0x01aec80c,
	0xfd11e4b6,
	0x07f105be,
	0x03f0c500,
	0x000bd001,
/* 0x01d6: mmctx_exec_loop */
/* 0x01d6: mmctx_wait_free */
	0xe7f104bd,
	0xe3f0c500,
	0x00eecf01,
	0xf41fe4f0,
	0xce98f30b,
	0x05e9fd00,
	0xc80007f1,
	0xd00103f0,
	0x04bd000e,
	0xb804c0b6,
	0x1bf404cd,
	0x02abc8d8,
/* 0x0207: mmctx_fini_wait */
	0xf11f1bf4,
	0xf0c500b7,
	0xbbcf01b3,
	0x1fb4f000,
	0xf410b4b0,
	0xa7f0f01b,
	0xd021f405,
/* 0x0223: mmctx_stop */
	0xc82b0ef4,
	0xb4b600ab,
	0x0cb9f010,
	0xf112b9f0,
	0xf0c50007,
	0x0bd00103,
/* 0x023b: mmctx_stop_wait */
	0xf104bd00,
	0xf0c500b7,
	0xbbcf01b3,
	0x12bbc800,
/* 0x024b: mmctx_done */
	0xbdf31bf4,
	0x0199f094,
	0x170007f1,
	0xd00203f0,
	0x04bd0009,
/* 0x025e: strand_wait */
	0xa0f900f8,
	0xf402a7f0,
	0xa0fcd021,
/* 0x026a: strand_pre */
	0x97f000f8,
	0xfc07f10c,
	0x0203f04a,
	0xbd0009d0,
	0x5e21f504,
/* 0x027f: strand_post */
	0xf000f802,
	0x07f10d97,
	0x03f04afc,
	0x0009d002,
	0x21f504bd,
	0x00f8025e,
/* 0x0294: strand_set */
	0xf10fc7f0,
	0xf04ffc07,
	0x0cd00203,
	0xf004bd00,
	0x07f10bc7,
	0x03f04afc,
	0x000cd002,
	0x07f104bd,
	0x03f04ffc,
	0x000ed002,
	0xc7f004bd,
	0xfc07f10a,
	0x0203f04a,
	0xbd000cd0,
	0x5e21f504,
/* 0x02d3: strand_ctx_init */
	0xbd00f802,
	0x0399f094,
	0x0f0007f1,
	0xd00203f0,
	0x04bd0009,
	0x026a21f5,
	0xf503e7f0,
	0xbd029421,
	0xfc07f1c4,
	0x0203f047,
	0xbd000cd0,
	0x01c7f004,
	0x4afc07f1,
	0xd00203f0,
	0x04bd000c,
	0x025e21f5,
	0xf1010c92,
	0xf046fc07,
	0x0cd00203,
	0xf004bd00,
	0x07f102c7,
	0x03f04afc,
	0x000cd002,
	0x21f504bd,
	0x21f5025e,
	0x87f1027f,
	0x83f04200,
	0x0097f102,
	0x0293f020,
	0x950099cf,
/* 0x034a: ctx_init_strand_loop */
	0x8ed008fe,
	0x408ed000,
	0xb6808acf,
	0xa0b606a5,
	0x00eabb01,
	0xb60480b6,
	0x1bf40192,
	0x08e4b6e8,
	0xbdf2efbc,
	0x0399f094,
	0x170007f1,
	0xd00203f0,
	0x04bd0009,
/* 0x037e: error */
	0xe0f900f8,
	0xf102ffb9,
	0xf09814e7,
	0x21f440e3,
	0x01f7f09d,
	0xf102ffb9,
	0xf09c1ce7,
	0x21f440e3,
	0xf8e0fc9d,
/* 0x03a1: init */
	0xf104bd00,
	0xf0420017,
	0x11cf0013,
	0x0911e700,
	0x0814b601,
	0xf00014fe,
	0x07f10227,
	0x03f01200,
	0x0002d000,
	0x17f104bd,
	0x10fe0542,
	0x0007f100,
	0x0003f007,
	0xbd0000d0,
	0x0427f004,
	0x040007f1,
	0xd00003f0,
	0x04bd0002,
	0xf11031f4,
	0xf0820027,
	0x22cf0123,
	0x0137f000,
	0xbb1f24f0,
	0x32b60432,
	0x05028001,
	0xf1060380,
	0xf0860027,
	0x22cf0123,
	0x04028000,
	0xf10f24b6,
	0xf0c90007,
	0x02d00103,
	0xf104bd00,
	0xf00c30e7,
	0x24bd50e3,
	0x44bd34bd,
/* 0x0430: init_unk_loop */
	0xb06821f4,
	0x0bf400f6,
	0x01f7f00f,
	0xfd04f2bb,
	0x30b6054f,
/* 0x0445: init_unk_next */
	0x0120b601,
	0xb004e0b6,
	0x1bf40126,
/* 0x0451: init_unk_done */
	0x070380e2,
	0xf1080480,
	0xf0010027,
	0x22cf0223,
	0x9534bd00,
	0x07f10825,
	0x03f0c000,
	0x0005d001,
	0x07f104bd,
	0x03f0c100,
	0x0005d001,
	0x0e9804bd,
	0x010f9800,
	0x015021f5,
	0xbb002fbb,
	0x0e98003f,
	0x020f9801,
	0x015021f5,
	0xfd050e98,
	0x2ebb00ef,
	0x003ebb00,
	0x98020e98,
	0x21f5030f,
	0x0e980150,
	0x00effd07,
	0xbb002ebb,
	0x35b6003e,
	0x0007f102,
	0x0103f0d3,
	0xbd0003d0,
	0x0825b604,
	0xb60635b6,
	0x30b60120,
	0x0824b601,
	0xb90834b6,
	0x21f5022f,
	0x2fbb02d3,
	0x003fbb00,
	0x010007f1,
	0xd00203f0,
	0x04bd0003,
	0x29f024bd,
	0x0007f11f,
	0x0203f008,
	0xbd0002d0,
/* 0x0505: main */
	0x0031f404,
	0xf00028f4,
	0x21f424d7,
	0xf401f439,
	0xf404e4b0,
	0x81fe1e18,
	0x0627f001,
	0x12fd20bd,
	0x01e4b604,
	0xfe051efd,
	0x21f50018,
	0x0ef405fa,
/* 0x0535: main_not_ctx_xfer */
	0x10ef94d3,
	0xf501f5f0,
	0xf4037e21,
/* 0x0542: ih */
	0x80f9c60e,
	0xf90188fe,
	0xf990f980,
	0xf9b0f9a0,
	0xf9e0f9d0,
	0xf104bdf0,
	0xf00200a7,
	0xaacf00a3,
	0x04abc400,
	0xf02c0bf4,
	0xe7f124d7,
	0xe3f01a00,
	0x00eecf00,
	0x1900f7f1,
	0xcf00f3f0,
	0x21f400ff,
	0x01e7f004,
	0x1d0007f1,
	0xd00003f0,
	0x04bd000e,
/* 0x0590: ih_no_fifo */
	0x010007f1,
	0xd00003f0,
	0x04bd000a,
	0xe0fcf0fc,
	0xb0fcd0fc,
	0x90fca0fc,
	0x88fe80fc,
	0xf480fc00,
	0x01f80032,
/* 0x05b4: hub_barrier_done */
	0x9801f7f0,
	0xfebb040e,
	0x02ffb904,
	0x9418e7f1,
	0xf440e3f0,
	0x00f89d21,
/* 0x05cc: ctx_redswitch */
	0xf120f7f0,
	0xf0850007,
	0x0fd00103,
	0xf004bd00,
/* 0x05de: ctx_redswitch_delay */
	0xe2b608e7,
	0xfd1bf401,
	0x0800f5f1,
	0x0200f5f1,
	0x850007f1,
	0xd00103f0,
	0x04bd000f,
/* 0x05fa: ctx_xfer */
	0x07f100f8,
	0x03f08100,
	0x000fd002,
	0x11f404bd,
	0xcc21f507,
/* 0x060d: ctx_xfer_not_load */
	0x6a21f505,
	0xf124bd02,
	0xf047fc07,
	0x02d00203,
	0xf004bd00,
	0x20b6012c,
	0xfc07f103,
	0x0203f04a,
	0xbd0002d0,
	0x01acf004,
	0xf102a5f0,
	0xf00000b7,
	0x0c9850b3,
	0x0fc4b604,
	0x9800bcbb,
	0x0d98000c,
	0x00e7f001,
	0x016f21f5,
	0xf101acf0,
	0xf04000b7,
	0x0c9850b3,
	0x0fc4b604,
	0x9800bcbb,
	0x0d98010c,
	0x060f9802,
	0x0800e7f1,
	0x016f21f5,
	0xf001acf0,
	0xb7f104a5,
	0xb3f03000,
	0x040c9850,
	0xbb0fc4b6,
	0x0c9800bc,
	0x030d9802,
	0xf1080f98,
	0xf50200e7,
	0xf5016f21,
	0xf4025e21,
	0x12f40601,
/* 0x06a9: ctx_xfer_post */
	0x7f21f507,
/* 0x06ad: ctx_xfer_done */
	0xb421f502,
	0x0000f805,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
};

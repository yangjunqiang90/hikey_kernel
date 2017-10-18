/*
 * Copyright 2012 Maarten Lankhorst
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Maarten Lankhorst
 */
#include <engine/msppp.h>
#include <engine/falcon.h>

struct gf100_msppp_priv {
	struct nvkm_falcon base;
};

/*******************************************************************************
 * MSPPP object classes
 ******************************************************************************/

static struct nvkm_oclass
gf100_msppp_sclass[] = {
	{ 0x90b3, &nvkm_object_ofuncs },
	{},
};

/*******************************************************************************
 * PMSPPP context
 ******************************************************************************/

static struct nvkm_oclass
gf100_msppp_cclass = {
	.handle = NV_ENGCTX(MSPPP, 0xc0),
	.ofuncs = &(struct nvkm_ofuncs) {
		.ctor = _nvkm_falcon_context_ctor,
		.dtor = _nvkm_falcon_context_dtor,
		.init = _nvkm_falcon_context_init,
		.fini = _nvkm_falcon_context_fini,
		.rd32 = _nvkm_falcon_context_rd32,
		.wr32 = _nvkm_falcon_context_wr32,
	},
};

/*******************************************************************************
 * PMSPPP engine/subdev functions
 ******************************************************************************/

static int
gf100_msppp_init(struct nvkm_object *object)
{
	struct gf100_msppp_priv *priv = (void *)object;
	int ret;

	ret = nvkm_falcon_init(&priv->base);
	if (ret)
		return ret;

	nv_wr32(priv, 0x086010, 0x0000fff2);
	nv_wr32(priv, 0x08601c, 0x0000fff2);
	return 0;
}

static int
gf100_msppp_ctor(struct nvkm_object *parent, struct nvkm_object *engine,
		 struct nvkm_oclass *oclass, void *data, u32 size,
		 struct nvkm_object **pobject)
{
	struct gf100_msppp_priv *priv;
	int ret;

	ret = nvkm_falcon_create(parent, engine, oclass, 0x086000, true,
				 "PMSPPP", "msppp", &priv);
	*pobject = nv_object(priv);
	if (ret)
		return ret;

	nv_subdev(priv)->unit = 0x00000002;
	nv_subdev(priv)->intr = nvkm_falcon_intr;
	nv_engine(priv)->cclass = &gf100_msppp_cclass;
	nv_engine(priv)->sclass = gf100_msppp_sclass;
	return 0;
}

struct nvkm_oclass
gf100_msppp_oclass = {
	.handle = NV_ENGINE(MSPPP, 0xc0),
	.ofuncs = &(struct nvkm_ofuncs) {
		.ctor = gf100_msppp_ctor,
		.dtor = _nvkm_falcon_dtor,
		.init = gf100_msppp_init,
		.fini = _nvkm_falcon_fini,
		.rd32 = _nvkm_falcon_rd32,
		.wr32 = _nvkm_falcon_wr32,
	},
};

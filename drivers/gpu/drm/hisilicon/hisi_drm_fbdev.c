/*
 * Hisilicon Terminal SoCs drm fbdev driver
 *
 * Copyright (c) 2014-2015 Hisilicon Limited.
 * Author: z.liuxinliang@huawei.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */


#include <drm/drmP.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_fb_helper.h>
#include <drm/drm_gem_cma_helper.h>
#include "hisi_drm_fb.h"
#include "hisi_drm_fbdev.h"
#include "hisi_drm_drv.h"


#define PREFERRED_BPP		32


static inline
struct hisi_drm_fbdev *to_hisi_drm_fbdev(struct drm_fb_helper *helper)
{
	return container_of(helper, struct hisi_drm_fbdev, fb_helper);
}

static struct fb_ops hisi_drm_fbdev_ops = {
	.owner		= THIS_MODULE,
	.fb_fillrect	= sys_fillrect,
	.fb_copyarea	= sys_copyarea,
	.fb_imageblit	= sys_imageblit,
	.fb_check_var	= drm_fb_helper_check_var,
	.fb_set_par	= drm_fb_helper_set_par,
	.fb_blank	= drm_fb_helper_blank,
	.fb_pan_display	= drm_fb_helper_pan_display,
	.fb_setcmap	= drm_fb_helper_setcmap,
};

static int hisi_drm_fbdev_probe(struct drm_fb_helper *helper,
	struct drm_fb_helper_surface_size *sizes)
{
	struct hisi_drm_fbdev *fbdev = to_hisi_drm_fbdev(helper);
	struct drm_mode_fb_cmd2 mode_cmd = { 0 };
	struct drm_device *dev = helper->dev;
	struct drm_gem_cma_object *obj;
	struct drm_framebuffer *fb;
	unsigned int bytes_per_pixel;
	unsigned long offset;
	struct fb_info *fbi;
	size_t size;
	int ret;


	/* TODO: Need to use ion heaps to create frame buffer?? */

	DRM_DEBUG_DRIVER("surface width(%d), height(%d) and bpp(%d)\n",
			sizes->surface_width, sizes->surface_height,
			sizes->surface_bpp);

	bytes_per_pixel = DIV_ROUND_UP(sizes->surface_bpp, 8);
	sizes->surface_depth = PREFERRED_BPP;

	mode_cmd.width = sizes->surface_width;
	mode_cmd.height = sizes->surface_height * HISI_NUM_FRAMEBUFFERS;
	mode_cmd.pitches[0] = sizes->surface_width * bytes_per_pixel;
	mode_cmd.pixel_format = drm_mode_legacy_fb_format(sizes->surface_bpp,
		sizes->surface_depth);

	size = roundup(mode_cmd.pitches[0] * mode_cmd.height, PAGE_SIZE);
	obj = drm_gem_cma_create(dev, size);
	if (IS_ERR(obj))
		return -ENOMEM;

	fbi = framebuffer_alloc(0, dev->dev);
	if (!fbi) {
		dev_err(dev->dev, "Failed to allocate framebuffer info.\n");
		ret = -ENOMEM;
		goto err_drm_gem_cma_free_object;
	}

	fbdev->fb = hisi_drm_fb_alloc(dev, &mode_cmd, &obj, 1, true);
	if (IS_ERR(fbdev->fb)) {
		dev_err(dev->dev, "Failed to allocate DRM framebuffer.\n");
		ret = PTR_ERR(fbdev->fb);
		goto err_framebuffer_release;
	}

	fb = &fbdev->fb->fb;
	helper->fb = fb;
	helper->fbdev = fbi;

	fbi->par = helper;
	fbi->flags = FBINFO_FLAG_DEFAULT;
	fbi->fbops = &hisi_drm_fbdev_ops;

	ret = fb_alloc_cmap(&fbi->cmap, 256, 0);
	if (ret) {
		dev_err(dev->dev, "Failed to allocate color map.\n");
		goto err_hisi_drm_fb_destroy;
	}

	drm_fb_helper_fill_fix(fbi, fb->pitches[0], fb->depth);
	drm_fb_helper_fill_var(fbi, helper, fb->width,
			fb->height/HISI_NUM_FRAMEBUFFERS);

	offset = fbi->var.xoffset * bytes_per_pixel;
	offset += fbi->var.yoffset * fb->pitches[0];

	dev->mode_config.fb_base = (resource_size_t)obj->paddr;
	fbi->screen_base = obj->vaddr + offset;
	fbi->fix.smem_start = (unsigned long)(obj->paddr + offset);
	fbi->screen_size = size;
	fbi->fix.smem_len = size;

	DRM_DEBUG_DRIVER("exit successfully.\n");
	return 0;

err_hisi_drm_fb_destroy:
	drm_framebuffer_unregister_private(fb);
	hisi_drm_fb_destroy(fb);
err_framebuffer_release:
	framebuffer_release(fbi);
err_drm_gem_cma_free_object:
	drm_gem_cma_free_object(&obj->base);
	return ret;
}

static const struct drm_fb_helper_funcs hisi_fb_helper_funcs = {
	.fb_probe = hisi_drm_fbdev_probe,
};

static struct hisi_drm_fbdev *hisi_drm_fbdev_create(struct drm_device *dev,
	unsigned int preferred_bpp, unsigned int num_crtc,
	unsigned int max_conn_count)
{
	struct hisi_drm_fbdev *fbdev;
	struct drm_fb_helper *helper;
	int ret;

	DRM_DEBUG_DRIVER("enter, dev=%p, preferred_bpp=%d, num_crtc=%d .\n",
			dev, preferred_bpp, num_crtc);
	fbdev = kzalloc(sizeof(*fbdev), GFP_KERNEL);
	if (!fbdev) {
		dev_err(dev->dev, "Failed to allocate drm fbdev.\n");
		return ERR_PTR(-ENOMEM);
	}

	helper = &fbdev->fb_helper;

	drm_fb_helper_prepare(dev, helper, &hisi_fb_helper_funcs);

	ret = drm_fb_helper_init(dev, helper, num_crtc, max_conn_count);
	if (ret < 0) {
		dev_err(dev->dev, "Failed to initialize drm fb helper.\n");
		goto err_free;
	}

	ret = drm_fb_helper_single_add_all_connectors(helper);
	if (ret < 0) {
		dev_err(dev->dev, "Failed to add connectors.\n");
		goto err_drm_fb_helper_fini;

	}

	/* disable all the possible outputs/crtcs before entering KMS mode */
	drm_helper_disable_unused_functions(dev);

	ret = drm_fb_helper_initial_config(helper, preferred_bpp);
	if (ret < 0) {
		dev_err(dev->dev, "Failed to set initial hw configuration.\n");
		goto err_drm_fb_helper_fini;
	}

	DRM_DEBUG_DRIVER("exit successfully.\n");
	return fbdev;

err_drm_fb_helper_fini:
	drm_fb_helper_fini(helper);
err_free:
	kfree(fbdev);

	return ERR_PTR(ret);
}

static void hisi_drm_fbdev_fini(struct hisi_drm_fbdev *fbdev)
{
	if (fbdev->fb_helper.fbdev) {
		struct fb_info *info;
		int ret;

		info = fbdev->fb_helper.fbdev;
		ret = unregister_framebuffer(info);
		if (ret < 0)
			DRM_DEBUG_KMS("failed unregister_framebuffer()\n");

		if (info->cmap.len)
			fb_dealloc_cmap(&info->cmap);

		framebuffer_release(info);
	}

	if (fbdev->fb) {
		drm_framebuffer_unregister_private(&fbdev->fb->fb);
		hisi_drm_fb_destroy(&fbdev->fb->fb);
	}

	drm_fb_helper_fini(&fbdev->fb_helper);
	kfree(fbdev);
}

/**
 * hisi_drm_fbdev_init - create and init hisi_drm_fbdev
 * @dev: The drm_device struct
 */
int hisi_drm_fbdev_init(struct drm_device *dev)
{
	struct hisi_drm_private *private = dev->dev_private;


	DRM_DEBUG_DRIVER("enter.\n");

	private->fbdev = hisi_drm_fbdev_create(dev, PREFERRED_BPP,
			dev->mode_config.num_crtc,
			dev->mode_config.num_connector);
	if (IS_ERR(private->fbdev))
		return PTR_ERR(private->fbdev);

	DRM_DEBUG_DRIVER("exit successfully.\n");
	return 0;
}

/**
 * hisi_drm_fbdev_exit - Free hisi_drm_fbdev struct
 * @dev: The drm_device struct
 */
void hisi_drm_fbdev_exit(struct drm_device *dev)
{
	struct hisi_drm_private *private = dev->dev_private;

	if (private->fbdev) {
		hisi_drm_fbdev_fini(private->fbdev);
		private->fbdev = NULL;
	}
}

/*
 * IOMMU sysfs class support
 *
 * Copyright (C) 2014 Red Hat, Inc.  All rights reserved.
 *     Author: Alex Williamson <alex.williamson@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/device.h>
#include <linux/iommu.h>
#include <linux/module.h>
#include <linux/slab.h>

/*
 * We provide a common class "devices" group which initially has no attributes.
 * As devices are added to the IOMMU, we'll add links to the group.
 */
static struct attribute *devices_attr[] = {
	NULL,
};

static const struct attribute_group iommu_devices_attr_group = {
	.name = "devices",
	.attrs = devices_attr,
};

static const struct attribute_group *iommu_dev_groups[] = {
	&iommu_devices_attr_group,
	NULL,
};

static void iommu_release_device(struct device *dev)
{
	kfree(dev);
}

static struct class iommu_class = {
	.name = "iommu",
	.dev_release = iommu_release_device,
	.dev_groups = iommu_dev_groups,
};

static int __init iommu_dev_init(void)
{
	return class_register(&iommu_class);
}
postcore_initcall(iommu_dev_init);

/*
 * Create an IOMMU device and return a pointer to it.  IOMMU specific
 * attributes can be provided as an attribute group, allowing a unique
 * namespace per IOMMU type.
 */

//创建一个iommu 设备，并且返回这个设备的指针，
 //iommu指定属性在属性数组中提供，每一个iommu类型
 //允许有一个唯一的namespace

//drvdata就是iommu
//最后一个参数就是iommu的名字
//创建iommu的device，并且通过device_add添加到系统中
//device中的drvdata就是这个iommu，device中kobject的名字就是
//iommu的名字
struct device *iommu_device_create(struct device *parent, void *drvdata,
				   const struct attribute_group **groups,
				   const char *fmt, ...)
{
	struct device *dev;
	va_list vargs;
	int ret;

	//申请一个device结构体
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return ERR_PTR(-ENOMEM);

	device_initialize(dev);

	dev->class = &iommu_class;
	dev->parent = parent;
	dev->groups = groups;

	//dev->driver_data = data;
	//这个data就是iommu
	dev_set_drvdata(dev, drvdata);

	va_start(vargs, fmt);
	//设置kobject的名字
	ret = kobject_set_name_vargs(&dev->kobj, fmt, vargs);
	va_end(vargs);
	if (ret)
		goto error;

	ret = device_add(dev);
	if (ret)
		goto error;

	return dev;

error:
	put_device(dev);
	return ERR_PTR(ret);
}

void iommu_device_destroy(struct device *dev)
{
	if (!dev || IS_ERR(dev))
		return;

	device_unregister(dev);
}

/*
 * IOMMU drivers can indicate a device is managed by a given IOMMU using
 * this interface.  A link to the device will be created in the "devices"
 * directory of the IOMMU device in sysfs and an "iommu" link will be
 * created under the linked device, pointing back at the IOMMU device.
 */
int iommu_device_link(struct device *dev, struct device *link)
{
	int ret;

	if (!dev || IS_ERR(dev))
		return -ENODEV;

	ret = sysfs_add_link_to_group(&dev->kobj, "devices",
				      &link->kobj, dev_name(link));
	if (ret)
		return ret;

	ret = sysfs_create_link_nowarn(&link->kobj, &dev->kobj, "iommu");
	if (ret)
		sysfs_remove_link_from_group(&dev->kobj, "devices",
					     dev_name(link));

	return ret;
}

void iommu_device_unlink(struct device *dev, struct device *link)
{
	if (!dev || IS_ERR(dev))
		return;

	sysfs_remove_link(&link->kobj, "iommu");
	sysfs_remove_link_from_group(&dev->kobj, "devices", dev_name(link));
}

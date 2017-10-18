/* Texas Instruments Ethernet Switch Driver
 *
 * Copyright (C) 2013 Texas Instruments
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/phy.h>
#include <linux/of.h>
#include <linux/of_device.h>

#include "cpsw.h"

/* AM33xx SoC specific definitions for the CONTROL port */
#define AM33XX_GMII_SEL_MODE_MII	0
#define AM33XX_GMII_SEL_MODE_RMII	1
#define AM33XX_GMII_SEL_MODE_RGMII	2

#define AM33XX_GMII_SEL_RMII2_IO_CLK_EN	BIT(7)
#define AM33XX_GMII_SEL_RMII1_IO_CLK_EN	BIT(6)

#define GMII_SEL_MODE_MASK		0x3

struct cpsw_phy_sel_priv {
	struct device	*dev;
	u32 __iomem	*gmii_sel;
	bool		rmii_clock_external;
	void (*cpsw_phy_sel)(struct cpsw_phy_sel_priv *priv,
			     phy_interface_t phy_mode, int slave);
};


static void cpsw_gmii_sel_am3352(struct cpsw_phy_sel_priv *priv,
				 phy_interface_t phy_mode, int slave)
{
	u32 reg;
	u32 mask;
	u32 mode = 0;

	reg = readl(priv->gmii_sel);

	switch (phy_mode) {
	case PHY_INTERFACE_MODE_RMII:
		mode = AM33XX_GMII_SEL_MODE_RMII;
		break;

	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		mode = AM33XX_GMII_SEL_MODE_RGMII;
		break;

	case PHY_INTERFACE_MODE_MII:
	default:
		mode = AM33XX_GMII_SEL_MODE_MII;
		break;
	};

	mask = GMII_SEL_MODE_MASK << (slave * 2) | BIT(slave + 6);
	mode <<= slave * 2;

	if (priv->rmii_clock_external) {
		if (slave == 0)
			mode |= AM33XX_GMII_SEL_RMII1_IO_CLK_EN;
		else
			mode |= AM33XX_GMII_SEL_RMII2_IO_CLK_EN;
	}

	reg &= ~mask;
	reg |= mode;

	writel(reg, priv->gmii_sel);
}

static void cpsw_gmii_sel_dra7xx(struct cpsw_phy_sel_priv *priv,
				 phy_interface_t phy_mode, int slave)
{
	u32 reg;
	u32 mask;
	u32 mode = 0;

	reg = readl(priv->gmii_sel);

	switch (phy_mode) {
	case PHY_INTERFACE_MODE_RMII:
		mode = AM33XX_GMII_SEL_MODE_RMII;
		break;

	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		mode = AM33XX_GMII_SEL_MODE_RGMII;
		break;

	case PHY_INTERFACE_MODE_MII:
	default:
		mode = AM33XX_GMII_SEL_MODE_MII;
		break;
	};

	switch (slave) {
	case 0:
		mask = GMII_SEL_MODE_MASK;
		break;
	case 1:
		mask = GMII_SEL_MODE_MASK << 4;
		mode <<= 4;
		break;
	default:
		dev_err(priv->dev, "invalid slave number...\n");
		return;
	}

	if (priv->rmii_clock_external)
		dev_err(priv->dev, "RMII External clock is not supported\n");

	reg &= ~mask;
	reg |= mode;

	writel(reg, priv->gmii_sel);
}

static struct platform_driver cpsw_phy_sel_driver;
static int match(struct device *dev, void *data)
{
	struct device_node *node = (struct device_node *)data;
	return dev->of_node == node &&
		dev->driver == &cpsw_phy_sel_driver.driver;
}

void cpsw_phy_sel(struct device *dev, phy_interface_t phy_mode, int slave)
{
	struct device_node *node;
	struct cpsw_phy_sel_priv *priv;

	node = of_get_child_by_name(dev->of_node, "cpsw-phy-sel");
	if (!node) {
		dev_err(dev, "Phy mode driver DT not found\n");
		return;
	}

	dev = bus_find_device(&platform_bus_type, NULL, node, match);
	priv = dev_get_drvdata(dev);

	priv->cpsw_phy_sel(priv, phy_mode, slave);
}
EXPORT_SYMBOL_GPL(cpsw_phy_sel);

static const struct of_device_id cpsw_phy_sel_id_table[] = {
	{
		.compatible	= "ti,am3352-cpsw-phy-sel",
		.data		= &cpsw_gmii_sel_am3352,
	},
	{
		.compatible	= "ti,dra7xx-cpsw-phy-sel",
		.data		= &cpsw_gmii_sel_dra7xx,
	},
	{
		.compatible	= "ti,am43xx-cpsw-phy-sel",
		.data		= &cpsw_gmii_sel_am3352,
	},
	{}
};
MODULE_DEVICE_TABLE(of, cpsw_phy_sel_id_table);

static int cpsw_phy_sel_probe(struct platform_device *pdev)
{
	struct resource	*res;
	const struct of_device_id *of_id;
	struct cpsw_phy_sel_priv *priv;

	of_id = of_match_node(cpsw_phy_sel_id_table, pdev->dev.of_node);
	if (!of_id)
		return -EINVAL;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		dev_err(&pdev->dev, "unable to alloc memory for cpsw phy sel\n");
		return -ENOMEM;
	}

	priv->dev = &pdev->dev;
	priv->cpsw_phy_sel = of_id->data;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "gmii-sel");
	priv->gmii_sel = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(priv->gmii_sel))
		return PTR_ERR(priv->gmii_sel);

	if (of_find_property(pdev->dev.of_node, "rmii-clock-ext", NULL))
		priv->rmii_clock_external = true;

	dev_set_drvdata(&pdev->dev, priv);

	return 0;
}

static struct platform_driver cpsw_phy_sel_driver = {
	.probe		= cpsw_phy_sel_probe,
	.driver		= {
		.name	= "cpsw-phy-sel",
		.of_match_table = cpsw_phy_sel_id_table,
	},
};

module_platform_driver(cpsw_phy_sel_driver);
MODULE_AUTHOR("Mugunthan V N <mugunthanvnm@ti.com>");
MODULE_LICENSE("GPL v2");

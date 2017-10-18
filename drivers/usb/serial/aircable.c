/*
 * AIRcable USB Bluetooth Dongle Driver.
 *
 * Copyright (C) 2010 Johan Hovold <jhovold@gmail.com>
 * Copyright (C) 2006 Manuel Francisco Naranjo (naranjo.manuel@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * The device works as an standard CDC device, it has 2 interfaces, the first
 * one is for firmware access and the second is the serial one.
 * The protocol is very simply, there are two possibilities reading or writing.
 * When writing the first urb must have a Header that starts with 0x20 0x29 the
 * next two bytes must say how much data will be sent.
 * When reading the process is almost equal except that the header starts with
 * 0x00 0x20.
 *
 * The device simply need some stuff to understand data coming from the usb
 * buffer: The First and Second byte is used for a Header, the Third and Fourth
 * tells the  device the amount of information the package holds.
 * Packages are 60 bytes long Header Stuff.
 * When writing to the device the first two bytes of the header are 0x20 0x29
 * When reading the bytes are 0x00 0x20, or 0x00 0x10, there is an strange
 * situation, when too much data arrives to the device because it sends the data
 * but with out the header. I will use a simply hack to override this situation,
 * if there is data coming that does not contain any header, then that is data
 * that must go directly to the tty, as there is no documentation about if there
 * is any other control code, I will simply check for the first
 * one.
 *
 * The driver registers himself with the USB-serial core and the USB Core. I had
 * to implement a probe function against USB-serial, because other way, the
 * driver was attaching himself to both interfaces. I have tried with different
 * configurations of usb_serial_driver with out exit, only the probe function
 * could handle this correctly.
 *
 * I have taken some info from a Greg Kroah-Hartman article:
 * http://www.linuxjournal.com/article/6573
 * And from Linux Device Driver Kit CD, which is a great work, the authors taken
 * the work to recompile lots of information an knowledge in drivers development
 * and made it all available inside a cd.
 * URL: http://kernel.org/pub/linux/kernel/people/gregkh/ddk/
 *
 */

#include <asm/unaligned.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/tty_flip.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>

/* Vendor and Product ID */
#define AIRCABLE_VID		0x16CA
#define AIRCABLE_USB_PID	0x1502

/* Protocol Stuff */
#define HCI_HEADER_LENGTH	0x4
#define TX_HEADER_0		0x20
#define TX_HEADER_1		0x29
#define RX_HEADER_0		0x00
#define RX_HEADER_1		0x20
#define HCI_COMPLETE_FRAME	64

/* rx_flags */
#define THROTTLED		0x01
#define ACTUALLY_THROTTLED	0x02

#define DRIVER_AUTHOR "Naranjo, Manuel Francisco <naranjo.manuel@gmail.com>, Johan Hovold <jhovold@gmail.com>"
#define DRIVER_DESC "AIRcable USB Driver"

/* ID table that will be registered with USB core */
static const struct usb_device_id id_table[] = {
	{ USB_DEVICE(AIRCABLE_VID, AIRCABLE_USB_PID) },
	{ },
};
MODULE_DEVICE_TABLE(usb, id_table);

static int aircable_prepare_write_buffer(struct usb_serial_port *port,
						void *dest, size_t size)
{
	int count;
	unsigned char *buf = dest;

	count = kfifo_out_locked(&port->write_fifo, buf + HCI_HEADER_LENGTH,
					size - HCI_HEADER_LENGTH, &port->lock);
	buf[0] = TX_HEADER_0;
	buf[1] = TX_HEADER_1;
	put_unaligned_le16(count, &buf[2]);

	return count + HCI_HEADER_LENGTH;
}

static int aircable_probe(struct usb_serial *serial,
			  const struct usb_device_id *id)
{
	struct usb_host_interface *iface_desc = serial->interface->
								cur_altsetting;
	struct usb_endpoint_descriptor *endpoint;
	int num_bulk_out = 0;
	int i;

	for (i = 0; i < iface_desc->desc.bNumEndpoints; i++) {
		endpoint = &iface_desc->endpoint[i].desc;
		if (usb_endpoint_is_bulk_out(endpoint)) {
			dev_dbg(&serial->dev->dev,
				"found bulk out on endpoint %d\n", i);
			++num_bulk_out;
		}
	}

	if (num_bulk_out == 0) {
		dev_dbg(&serial->dev->dev, "Invalid interface, discarding\n");
		return -ENODEV;
	}

	return 0;
}

static int aircable_process_packet(struct usb_serial_port *port,
		int has_headers, char *packet, int len)
{
	if (has_headers) {
		len -= HCI_HEADER_LENGTH;
		packet += HCI_HEADER_LENGTH;
	}
	if (len <= 0) {
		dev_dbg(&port->dev, "%s - malformed packet\n", __func__);
		return 0;
	}

	tty_insert_flip_string(&port->port, packet, len);

	return len;
}

static void aircable_process_read_urb(struct urb *urb)
{
	struct usb_serial_port *port = urb->context;
	char *data = (char *)urb->transfer_buffer;
	int has_headers;
	int count;
	int len;
	int i;

	has_headers = (urb->actual_length > 2 && data[0] == RX_HEADER_0);

	count = 0;
	for (i = 0; i < urb->actual_length; i += HCI_COMPLETE_FRAME) {
		len = min_t(int, urb->actual_length - i, HCI_COMPLETE_FRAME);
		count += aircable_process_packet(port, has_headers,
								&data[i], len);
	}

	if (count)
		tty_flip_buffer_push(&port->port);
}

static struct usb_serial_driver aircable_device = {
	.driver = {
		.owner =	THIS_MODULE,
		.name =		"aircable",
	},
	.id_table = 		id_table,
	.num_ports =		1,
	.bulk_out_size =	HCI_COMPLETE_FRAME,
	.probe =		aircable_probe,
	.process_read_urb =	aircable_process_read_urb,
	.prepare_write_buffer =	aircable_prepare_write_buffer,
	.throttle =		usb_serial_generic_throttle,
	.unthrottle =		usb_serial_generic_unthrottle,
};

static struct usb_serial_driver * const serial_drivers[] = {
	&aircable_device, NULL
};

module_usb_serial_driver(serial_drivers, id_table);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

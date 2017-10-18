/*
 * oxfw_hwdep.c - a part of driver for OXFW970/971 based devices
 *
 * Copyright (c) 2014 Takashi Sakamoto
 *
 * Licensed under the terms of the GNU General Public License, version 2.
 */

/*
 * This codes give three functionality.
 *
 * 1.get firewire node information
 * 2.get notification about starting/stopping stream
 * 3.lock/unlock stream
 */

#include "oxfw.h"

static long hwdep_read(struct snd_hwdep *hwdep, char __user *buf,  long count,
		       loff_t *offset)
{
	struct snd_oxfw *oxfw = hwdep->private_data;
	DEFINE_WAIT(wait);
	union snd_firewire_event event;

	spin_lock_irq(&oxfw->lock);

	while (!oxfw->dev_lock_changed) {
		prepare_to_wait(&oxfw->hwdep_wait, &wait, TASK_INTERRUPTIBLE);
		spin_unlock_irq(&oxfw->lock);
		schedule();
		finish_wait(&oxfw->hwdep_wait, &wait);
		if (signal_pending(current))
			return -ERESTARTSYS;
		spin_lock_irq(&oxfw->lock);
	}

	memset(&event, 0, sizeof(event));
	if (oxfw->dev_lock_changed) {
		event.lock_status.type = SNDRV_FIREWIRE_EVENT_LOCK_STATUS;
		event.lock_status.status = (oxfw->dev_lock_count > 0);
		oxfw->dev_lock_changed = false;

		count = min_t(long, count, sizeof(event.lock_status));
	}

	spin_unlock_irq(&oxfw->lock);

	if (copy_to_user(buf, &event, count))
		return -EFAULT;

	return count;
}

static unsigned int hwdep_poll(struct snd_hwdep *hwdep, struct file *file,
			       poll_table *wait)
{
	struct snd_oxfw *oxfw = hwdep->private_data;
	unsigned int events;

	poll_wait(file, &oxfw->hwdep_wait, wait);

	spin_lock_irq(&oxfw->lock);
	if (oxfw->dev_lock_changed)
		events = POLLIN | POLLRDNORM;
	else
		events = 0;
	spin_unlock_irq(&oxfw->lock);

	return events;
}

static int hwdep_get_info(struct snd_oxfw *oxfw, void __user *arg)
{
	struct fw_device *dev = fw_parent_device(oxfw->unit);
	struct snd_firewire_get_info info;

	memset(&info, 0, sizeof(info));
	info.type = SNDRV_FIREWIRE_TYPE_OXFW;
	info.card = dev->card->index;
	*(__be32 *)&info.guid[0] = cpu_to_be32(dev->config_rom[3]);
	*(__be32 *)&info.guid[4] = cpu_to_be32(dev->config_rom[4]);
	strlcpy(info.device_name, dev_name(&dev->device),
		sizeof(info.device_name));

	if (copy_to_user(arg, &info, sizeof(info)))
		return -EFAULT;

	return 0;
}

static int hwdep_lock(struct snd_oxfw *oxfw)
{
	int err;

	spin_lock_irq(&oxfw->lock);

	if (oxfw->dev_lock_count == 0) {
		oxfw->dev_lock_count = -1;
		err = 0;
	} else {
		err = -EBUSY;
	}

	spin_unlock_irq(&oxfw->lock);

	return err;
}

static int hwdep_unlock(struct snd_oxfw *oxfw)
{
	int err;

	spin_lock_irq(&oxfw->lock);

	if (oxfw->dev_lock_count == -1) {
		oxfw->dev_lock_count = 0;
		err = 0;
	} else {
		err = -EBADFD;
	}

	spin_unlock_irq(&oxfw->lock);

	return err;
}

static int hwdep_release(struct snd_hwdep *hwdep, struct file *file)
{
	struct snd_oxfw *oxfw = hwdep->private_data;

	spin_lock_irq(&oxfw->lock);
	if (oxfw->dev_lock_count == -1)
		oxfw->dev_lock_count = 0;
	spin_unlock_irq(&oxfw->lock);

	return 0;
}

static int hwdep_ioctl(struct snd_hwdep *hwdep, struct file *file,
		       unsigned int cmd, unsigned long arg)
{
	struct snd_oxfw *oxfw = hwdep->private_data;

	switch (cmd) {
	case SNDRV_FIREWIRE_IOCTL_GET_INFO:
		return hwdep_get_info(oxfw, (void __user *)arg);
	case SNDRV_FIREWIRE_IOCTL_LOCK:
		return hwdep_lock(oxfw);
	case SNDRV_FIREWIRE_IOCTL_UNLOCK:
		return hwdep_unlock(oxfw);
	default:
		return -ENOIOCTLCMD;
	}
}

#ifdef CONFIG_COMPAT
static int hwdep_compat_ioctl(struct snd_hwdep *hwdep, struct file *file,
			      unsigned int cmd, unsigned long arg)
{
	return hwdep_ioctl(hwdep, file, cmd,
			   (unsigned long)compat_ptr(arg));
}
#else
#define hwdep_compat_ioctl NULL
#endif

int snd_oxfw_create_hwdep(struct snd_oxfw *oxfw)
{
	static const struct snd_hwdep_ops hwdep_ops = {
		.read		= hwdep_read,
		.release	= hwdep_release,
		.poll		= hwdep_poll,
		.ioctl		= hwdep_ioctl,
		.ioctl_compat	= hwdep_compat_ioctl,
	};
	struct snd_hwdep *hwdep;
	int err;

	err = snd_hwdep_new(oxfw->card, oxfw->card->driver, 0, &hwdep);
	if (err < 0)
		goto end;
	strcpy(hwdep->name, oxfw->card->driver);
	hwdep->iface = SNDRV_HWDEP_IFACE_FW_OXFW;
	hwdep->ops = hwdep_ops;
	hwdep->private_data = oxfw;
	hwdep->exclusive = true;
end:
	return err;
}

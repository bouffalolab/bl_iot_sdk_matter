#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <utils_list.h>
#include <FreeRTOS.h>
#include <task.h>
#include "tp_io.h"

tp_handle_t tp_io_open(void)
{
	tp_handle_t hd = pvPortMalloc((sizeof(struct tp_io)));
	return hd;
}

int tp_io_drv_register(tp_handle_t hd, const struct tp_io_drv *drv, void *srv)
{
	hd->drv = drv;
	hd->srv = srv;
	return 0;
}

int tp_write(tp_handle_t hd, const void *buf, size_t nbytes)
{
	if (!hd->drv->tp_write) {
		return -1;
	}
	return hd->drv->tp_write(hd->srv, buf, nbytes);
}

int tp_read(tp_handle_t hd, void *buf, size_t nbytes)
{
	if (!hd->drv->tp_read) {
		return -1;
	}
	return hd->drv->tp_read(hd->srv, buf, nbytes);
}

int tp_ioctl(tp_handle_t hd, int ctl, void *arg)
{
	if (!hd->drv->tp_ioctl) {
		return -1;
	}
	return hd->drv->tp_ioctl(hd->srv, ctl, arg);
}

int tp_io_close(tp_handle_t hd)
{
	vPortFree(hd);
	return 0;
}

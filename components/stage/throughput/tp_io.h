#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

#ifndef __TP_IO_H__
#define __TP_IO_H__

struct tp_io {
    const struct tp_io_drv *drv;
    void *srv;
};

struct tp_io_drv {
    int (*tp_write)(void *srv, const void *buf, size_t nbytes);
    int (*tp_read)(void *srv, void *buf, size_t nbytes);
    int (*tp_ioctl)(void *srv, int ctl, void *arg);
};

typedef struct tp_io *tp_handle_t;

tp_handle_t tp_io_open(void);

int tp_io_drv_register(tp_handle_t hd, const struct tp_io_drv *drv, void *srv);

int tp_write(tp_handle_t hd, const void *buf, size_t nbytes);

int tp_read(tp_handle_t hd, void *buf, size_t nbytes);

int tp_ioctl(tp_handle_t hd, int ctl, void *arg);

int tp_io_close(tp_handle_t hd);

#endif

#ifndef __OS_HAL_H__
#define __OS_HAL_H__

//#define USING_RTTHREAD

#ifdef USING_RTTHREAD
extern void rt_kprintf(const char *fmt, ...);
extern void *rt_malloc(int size);
#define os_printf rt_kprintf
#define os_malloc rt_malloc
#else
#define os_printf printf
#endif

#endif

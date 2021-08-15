#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

#ifndef __TP_CHECK_H__
#define __TP_CHECK_H__

static inline int tp_check(const uint8_t *buf, size_t length)
{
	int check_sum = 0;
	uint8_t *ptr = (uint8_t *)buf;

	for (int i = 0; i < length; i++, ptr++) {
		check_sum += *ptr;
	}
	return check_sum;
}

#endif

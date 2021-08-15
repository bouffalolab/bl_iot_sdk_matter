#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "stdint.h"

/* ring buffer */
struct ringbuf
{
    uint8_t *buffer_ptr;
    /* use the msb of the {read,write}_index as mirror bit. You can see this as
     * if the buffer adds a virtual mirror and the pointers point either to the
     * normal or to the mirrored buffer. If the write_index has the same value
     * with the read_index, but in a different mirror, the buffer is full.
     * While if the write_index and the read_index are the same and within the
     * same mirror, the buffer is empty. The ASCII art of the ringbuffer is:
     *
     *          mirror = 0                    mirror = 1
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     * | 0 | 1 | 2 | 3 | 4 | 5 | 6 ||| 0 | 1 | 2 | 3 | 4 | 5 | 6 | Full
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     *  read_idx-^                   write_idx-^
     *
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     * | 0 | 1 | 2 | 3 | 4 | 5 | 6 ||| 0 | 1 | 2 | 3 | 4 | 5 | 6 | Empty
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     * read_idx-^ ^-write_idx
     *
     * The tradeoff is we could only use 32KiB of buffer for 16 bit of index.
     * But it should be enough for most of the cases.
     *
     * Ref: http://en.wikipedia.org/wiki/Circular_buffer#Mirroring */
    uint16_t read_mirror : 1;
    uint16_t read_index : 15;
    uint16_t write_mirror : 1;
    uint16_t write_index : 15;
    /* as we use msb of index as mirror bit, the size should be signed and
     * could only be positive. */
    int16_t buffer_size;
};

enum ringbuf_state
{
    RINGBUF_EMPTY,
    RINGBUF_FULL,
    /* half full is neither full nor empty */
    RINGBUF_HALFFULL,
};

/**
 * RingBuffer for DeviceDriver
 *
 * Please note that the ring buffer implementation of RT-Thread
 * has no thread wait or resume feature.
 */
void ringbuf_init(struct ringbuf *rb, uint8_t *pool, int16_t size);
void ringbuf_reset(struct ringbuf *rb);
size_t ringbuf_put(struct ringbuf *rb, const uint8_t *ptr, uint16_t length);
size_t ringbuf_put_force(struct ringbuf *rb, const uint8_t *ptr, uint16_t length);
size_t ringbuf_putchar(struct ringbuf *rb, const uint8_t ch);
size_t ringbuf_putchar_force(struct ringbuf *rb, const uint8_t ch);
size_t ringbuf_get(struct ringbuf *rb, uint8_t *ptr, uint16_t length);
size_t ringbuf_peak(struct ringbuf *rb, uint8_t **ptr);
size_t ringbuf_getchar(struct ringbuf *rb, uint8_t *ch);
size_t ringbuf_data_len(struct ringbuf *rb);

static inline uint16_t ringbuf_get_size(struct ringbuf *rb)
{
    return rb->buffer_size;
}

/** return the size of empty space in rb */
#define ringbuf_space_len(rb) ((rb)->buffer_size - ringbuf_data_len(rb))


#ifdef __cplusplus
}
#endif

#endif

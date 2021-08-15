#include <string.h>
#include "ringbuf.h"

#define _ALIGN_DOWN(size, align)      ((size) & ~((align) - 1))

static inline enum ringbuf_state ringbuf_status(struct ringbuf *rb)
{
    if (rb->read_index == rb->write_index)
    {
        if (rb->read_mirror == rb->write_mirror)
            return RINGBUF_EMPTY;
        else
            return RINGBUF_FULL;
    }
    return RINGBUF_HALFFULL;
}

void ringbuf_init(struct ringbuf *rb,
                  uint8_t           *pool,
                  int16_t            size)
{
    /* initialize read and write index */
    rb->read_mirror = rb->read_index = 0;
    rb->write_mirror = rb->write_index = 0;

    /* set buffer pool and size */
    rb->buffer_ptr = pool;
    rb->buffer_size = _ALIGN_DOWN(size, 4);
}

/**
 * put a block of data into ring buffer
 */
size_t ringbuf_put(struct ringbuf *rb,
                   const uint8_t     *ptr,
                   uint16_t           length)
{
    uint16_t size;

    /* whether has enough space */
    size = ringbuf_space_len(rb);

    /* no space */
    if (size == 0)
        return 0;

    /* drop some data */
    if (size < length)
        length = size;

    if (rb->buffer_size - rb->write_index > length)
    {
        /* read_index - write_index = empty space */
        memcpy(&rb->buffer_ptr[rb->write_index], ptr, length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        rb->write_index += length;
        return length;
    }

    memcpy(&rb->buffer_ptr[rb->write_index],
           &ptr[0],
           rb->buffer_size - rb->write_index);
    memcpy(&rb->buffer_ptr[0],
           &ptr[rb->buffer_size - rb->write_index],
           length - (rb->buffer_size - rb->write_index));

    /* we are going into the other side of the mirror */
    rb->write_mirror = ~rb->write_mirror;
    rb->write_index = length - (rb->buffer_size - rb->write_index);

    return length;
}

/**
 * put a block of data into ring buffer
 *
 * When the buffer is full, it will discard the old data.
 */
size_t ringbuf_put_force(struct ringbuf *rb,
                            const uint8_t     *ptr,
                            uint16_t           length)
{
    uint16_t space_length;

    space_length = ringbuf_space_len(rb);

    if (length > rb->buffer_size)
    {
        ptr = &ptr[length - rb->buffer_size];
        length = rb->buffer_size;
    }

    if (rb->buffer_size - rb->write_index > length)
    {
        /* read_index - write_index = empty space */
        memcpy(&rb->buffer_ptr[rb->write_index], ptr, length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        rb->write_index += length;

        if (length > space_length)
            rb->read_index = rb->write_index;

        return length;
    }

    memcpy(&rb->buffer_ptr[rb->write_index],
           &ptr[0],
           rb->buffer_size - rb->write_index);
    memcpy(&rb->buffer_ptr[0],
           &ptr[rb->buffer_size - rb->write_index],
           length - (rb->buffer_size - rb->write_index));

    /* we are going into the other side of the mirror */
    rb->write_mirror = ~rb->write_mirror;
    rb->write_index = length - (rb->buffer_size - rb->write_index);

    if (length > space_length)
    {
        rb->read_mirror = ~rb->read_mirror;
        rb->read_index = rb->write_index;
    }

    return length;
}

/**
 *  get data from ring buffer
 */
size_t ringbuf_get(struct ringbuf *rb,
                            uint8_t           *ptr,
                            uint16_t           length)
{
    size_t size;

    /* whether has enough data  */
    size = ringbuf_data_len(rb);

    /* no data */
    if (size == 0)
        return 0;

    /* less data */
    if (size < length)
        length = size;

    if (rb->buffer_size - rb->read_index > length)
    {
        /* copy all of data */
        memcpy(ptr, &rb->buffer_ptr[rb->read_index], length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        rb->read_index += length;
        return length;
    }

    memcpy(&ptr[0],
           &rb->buffer_ptr[rb->read_index],
           rb->buffer_size - rb->read_index);
    memcpy(&ptr[rb->buffer_size - rb->read_index],
           &rb->buffer_ptr[0],
           length - (rb->buffer_size - rb->read_index));

    /* we are going into the other side of the mirror */
    rb->read_mirror = ~rb->read_mirror;
    rb->read_index = length - (rb->buffer_size - rb->read_index);

    return length;
}

/**
 *  peak data from ring buffer
 */
size_t ringbuf_peak(struct ringbuf *rb, uint8_t **ptr)
{
    *ptr = NULL;

    /* whether has enough data  */
    size_t size = ringbuf_data_len(rb);

    /* no data */
    if (size == 0)
        return 0;

    *ptr = &rb->buffer_ptr[rb->read_index];

    if(rb->buffer_size - rb->read_index > size)
    {
        rb->read_index += size;
        return size;
    }

    size = rb->buffer_size - rb->read_index;

    /* we are going into the other side of the mirror */
    rb->read_mirror = ~rb->read_mirror;
    rb->read_index = 0;

    return size;
}

/**
 * put a character into ring buffer
 */
size_t ringbuf_putchar(struct ringbuf *rb, const uint8_t ch)
{
    /* whether has enough space */
    if (!ringbuf_space_len(rb))
        return 0;

    rb->buffer_ptr[rb->write_index] = ch;

    /* flip mirror */
    if (rb->write_index == rb->buffer_size-1)
    {
        rb->write_mirror = ~rb->write_mirror;
        rb->write_index = 0;
    }
    else
    {
        rb->write_index++;
    }

    return 1;
}

/**
 * put a character into ring buffer
 *
 * When the buffer is full, it will discard one old data.
 */
size_t ringbuf_putchar_force(struct ringbuf *rb, const uint8_t ch)
{
    enum ringbuf_state old_state;

    old_state = ringbuf_status(rb);

    rb->buffer_ptr[rb->write_index] = ch;

    /* flip mirror */
    if (rb->write_index == rb->buffer_size-1)
    {
        rb->write_mirror = ~rb->write_mirror;
        rb->write_index = 0;
        if (old_state == RINGBUF_FULL)
        {
            rb->read_mirror = ~rb->read_mirror;
            rb->read_index = rb->write_index;
        }
    }
    else
    {
        rb->write_index++;
        if (old_state == RINGBUF_FULL)
            rb->read_index = rb->write_index;
    }

    return 1;
}

/**
 * get a character from a ringbuffer
 */
size_t ringbuf_getchar(struct ringbuf *rb, uint8_t *ch)
{
    /* ringbuffer is empty */
    if (!ringbuf_data_len(rb))
        return 0;

    /* put character */
    *ch = rb->buffer_ptr[rb->read_index];

    if (rb->read_index == rb->buffer_size-1)
    {
        rb->read_mirror = ~rb->read_mirror;
        rb->read_index = 0;
    }
    else
    {
        rb->read_index++;
    }

    return 1;
}

/**
 * get the size of data in rb
 */
size_t ringbuf_data_len(struct ringbuf *rb)
{
    switch (ringbuf_status(rb))
    {
    case RINGBUF_EMPTY:
        return 0;
    case RINGBUF_FULL:
        return rb->buffer_size;
    case RINGBUF_HALFFULL:
    default:
        if (rb->write_index > rb->read_index)
            return rb->write_index - rb->read_index;
        else
            return rb->buffer_size - (rb->read_index - rb->write_index);
    };
}

/**
 * empty the rb
 */
void ringbuf_reset(struct ringbuf *rb)
{
    rb->read_mirror = 0;
    rb->read_index = 0;
    rb->write_mirror = 0;
    rb->write_index = 0;
}

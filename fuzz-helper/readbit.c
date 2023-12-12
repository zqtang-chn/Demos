
#include <stdbool.h>

typedef struct bs_s
{
    uint8_t *p_start;
    uint8_t *p;
    uint8_t *p_end;

    size_t  i_left;    /* i_count number of available bits */
} bs_t;

static void bs_init( bs_t *s, const void *p_data, size_t i_data )
{
    s->p_start = (void *)p_data;
    s->p       = s->p_start;
    s->p_end   = s->p_start + i_data;
    s->i_left  = 8;
}

static inline uint32_t bs_read( bs_t *s, int i_count, bool *stat)
{
     static const uint32_t i_mask[33] =
     {  0x00,
        0x01,      0x03,      0x07,      0x0f,
        0x1f,      0x3f,      0x7f,      0xff,
        0x1ff,     0x3ff,     0x7ff,     0xfff,
        0x1fff,    0x3fff,    0x7fff,    0xffff,
        0x1ffff,   0x3ffff,   0x7ffff,   0xfffff,
        0x1fffff,  0x3fffff,  0x7fffff,  0xffffff,
        0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff,
        0x1fffffff,0x3fffffff,0x7fffffff,0xffffffff};
    int      i_shr;
    uint32_t i_result = 0;

    while( i_count > 0 )
    {
        if( s->p >= s->p_end )
        {
            *stat = false;
            break;
        }

        if( ( i_shr = s->i_left - i_count ) >= 0 )
        {
            /* more in the buffer than requested */
            i_result |= ( *s->p >> i_shr )&i_mask[i_count];
            s->i_left -= i_count;
            if( s->i_left == 0 )
            {
                s->p++;
                s->i_left = 8;
            }
            *stat = true;
            return( i_result );
        }
        else
        {
            /* less in the buffer than requested */
           i_result |= (*s->p&i_mask[s->i_left]) << -i_shr;
           i_count  -= s->i_left;
           s->p++;
           s->i_left = 8;
        }
    }
    *stat = true;
    return( i_result );
}


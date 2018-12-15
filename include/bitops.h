#ifndef __BITOPS_H__
#define __BITOPS_H__

#define set_bit(bit, data) \
    do\
    {\
        (*(data)) |= (1 << (bit));\
    } while (0)

#define clear_bit(bit, data)\
    do\
    {\
        (*(data)) &= ~(1 << (bit));\
    } while (0)

#define test_bit(bit, data) (data) & (1 << (bit))

#endif


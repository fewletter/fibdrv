#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>

#ifndef __BN_H_
#define __BN_H_

/* number[size - 1] = msb, number[0] = slsb */
typedef struct _bn {
    unsigned int *number;
    unsigned int size;
} bn;

int fibn_per_32bit(int n)
{
    return n < 2 ? 1 : ((-1160964 + n * 694242) / (10 * 6) + 1);
}

void bn_init(struct _bn *bign, int _size)
{
    bign->size = _size;
    bign->number = kmalloc(_size * sizeof(unsigned int), GFP_KERNEL);

    if (bign->number == NULL)
        return -ENOMEM;
}

void bn_set32(struct _bn *bign, unsigned int _num)
{
    for (int i = 0; i < bign->size; i++)
        bign->number[i] = 0;

    bign->number[0] = _num;
}

bn *bn_alloc(size_t size)
{
    bn *new = (bn *) kmalloc(sizeof(bn), GFP_KERNEL);
    new->number = kmalloc(sizeof(int) * size, GFP_KERNEL);
    memset(new->number, 0, sizeof(int) * size);
    new->size = size;
    return new;
}

int bn_free(bn *src)
{
    if (src == NULL)
        return -1;
    kfree(src->number);
    kfree(src);
    return 0;
}

char *bn_to_string(bn src)
{
    size_t len = (8 * sizeof(int) * src.size) / 3 + 2;
    char *s = kmalloc(len, GFP_KERNEL);
    char *p = s;

    memset(s, '0', len - 1);
    s[len - 1] = '\0';

    for (int i = src.size - 1; i >= 0; i--) {
        for (unsigned int d = 1U << 31; d; d >>= 1) {
            /* binary -> decimal string */
            int carry = !!(d & src.number[i]);
            for (int j = len - 2; j >= 0; j--) {
                s[j] += s[j] - '0' + carry;  // double it
                carry = (s[j] > '9');
                if (carry)
                    s[j] -= 10;
            }
        }
    }
    // skip leading zero
    while (p[0] == '0' && p[1] != '\0') {
        p++;
    }
    memmove(s, p, strlen(p) + 1);
    return s;
}

void bn_swap(bn *a, bn *b)
{
    bn tmp = *a;
    *a = *b;
    *b = tmp;
}

void bn_add(bn *a, bn *b, bn *c)
{
    unsigned long long int carry = 0;
    for (int i = 0; i < c->size; i++) {
        unsigned int tmp1 = (i < a->size) ? a->number[i] : 0;
        unsigned int tmp2 = (i < b->size) ? b->number[i] : 0;
        carry += (unsigned long long int) tmp1 + tmp2;
        c->number[i] = carry;
        carry >>= 32;
    }
}

static int bn_resize(bn *src, size_t size)
{
    if (!src)
        return -1;
    if (size == src->size)
        return 0;
    if (size == 0)  // prevent krealloc(0) = kfree, which will cause problem
        return bn_free(src);
    src->number = krealloc(src->number, sizeof(int) * size, GFP_KERNEL);
    if (!src->number) {  // realloc fails
        return -1;
    }
    if (size > src->size)
        memset(src->number + src->size, 0, sizeof(int) * (size - src->size));
    src->size = size;
    return 0;
}

#endif

#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>
#define MAX_32BIT 4294967295
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define DIV_ROUNDUP(x, len) (((x) + (len) -1) / (len))

#ifndef __BN_H_
#define __BN_H_

/* number[size - 1] = msb, number[0] = slsb */
typedef struct _bn {
    unsigned int *number;
    unsigned int size;
} bn;

/* count leading zeros of src*/
static int bn_clz(const bn *src)
{
    int cnt = 0;
    for (int i = src->size - 1; i >= 0; i--) {
        if (src->number[i]) {
            // prevent undefined behavior when src = 0
            cnt += __builtin_clz(src->number[i]);
            return cnt;
        } else {
            cnt += 32;
        }
    }
    return cnt;
}

/* count the digits of most significant bit */
static int bn_msb(const bn *src)
{
    return src->size * 32 - bn_clz(src);
}

void bn_init(struct _bn *bign, int _size)
{
    bign = kmalloc(sizeof(bn), GFP_KERNEL);
    bign->size = _size;
    bign->number = kmalloc(_size * sizeof(unsigned int), GFP_KERNEL);
    memset(bign->number, 0, sizeof(int) * _size);
}

void bn_set32(struct _bn *bign, unsigned int _num)
{
    for (int i = 0; i < bign->size; i++)
        bign->number[i] = 0;

    bign->number[0] = _num;
}

bn *bn_alloc(size_t size)
{
    bn *new = kmalloc(sizeof(bn), GFP_KERNEL);
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

static void bn_sub(const bn *a, const bn *b, bn *c)
{
    // max digits = max(sizeof(a) + sizeof(b))
    int d = MAX(a->size, b->size);
    bn_resize(c, d);

    long long int carry = 0;
    for (int i = 0; i < c->size; i++) {
        unsigned int tmp1 = (i < a->size) ? a->number[i] : 0;
        unsigned int tmp2 = (i < b->size) ? b->number[i] : 0;
        carry = (long long int) tmp1 - tmp2 - carry;
        if (carry < 0) {
            c->number[i] = carry + (1LL << 32);
            carry = 1;
        } else {
            c->number[i] = carry;
            carry = 0;
        }
    }

    d = bn_clz(c) / 32;
    if (d == c->size)
        --d;
    bn_resize(c, c->size - d);
}

static void bn_mult_add(bn *c, int offset, unsigned long long int x)
{
    unsigned long long int carry = 0;
    for (int i = offset; i < c->size; i++) {
        carry += c->number[i] + (x & 0xFFFFFFFF);
        c->number[i] = carry;
        carry >>= 32;
        x >>= 32;
        if (!x && !carry)  // done
            return;
    }
}

int bn_cpy(bn *dest, bn *src)
{
    if (bn_resize(dest, src->size) < 0)
        return -1;
    memcpy(dest->number, src->number, src->size * sizeof(int));
    return 0;
}

void bn_mult(const bn *a, const bn *b, bn *c)
{
    // max digits = sizeof(a) + sizeof(b))
    int d = bn_msb(a) + bn_msb(b);
    d = DIV_ROUNDUP(d, 32) + !d;  // round up, min size = 1
    bn *tmp;
    /* make it work properly when c == a or c == b */
    if (c == a || c == b) {
        tmp = c;  // save c
        c = bn_alloc(d);
    } else {
        tmp = NULL;
        bn_resize(c, d);
    }

    for (int i = 0; i < a->size; i++) {
        for (int j = 0; j < b->size; j++) {
            unsigned long long int carry = 0;
            carry = (unsigned long long int) a->number[i] * b->number[j];
            bn_mult_add(c, i + j, carry);
        }
    }

    if (tmp) {
        bn_cpy(tmp, c);  // restore c
        bn_free(c);
    }
}

void bn_add(bn *a, bn *b, bn *c)
{
    int d = MAX(a->size, b->size);
    bn_resize(c, d);

    unsigned long long int carry = 0;
    for (int i = 0; i < c->size; i++) {
        unsigned int tmp1 = (i < a->size) ? a->number[i] : 0;
        unsigned int tmp2 = (i < b->size) ? b->number[i] : 0;
        carry += (unsigned long long int) tmp1 + tmp2;
        c->number[i] = carry;
        carry >>= 32;
    }
}

static void bn_multSSA(const bn *a, const bn *b, bn *c)
{
    /* a*b = c */
    int d = bn_msb(a) + bn_msb(b);
    d = DIV_ROUNDUP(d, 32) + !d;  // round up, min size = 1
    bn_resize(c, d);
    __uint128_t carry = 0;
    for (int i = 0; i < c->size; i++) {
        int j = a->size;
        while (j >= 0) {
            unsigned int tmp1 = ((j <= i) && j < a->size) ? a->number[j] : 0;
            unsigned int tmp2 = ((i - j) < b->size) ? b->number[i - j] : 0;
            carry += (__uint128_t) tmp1 * tmp2;
            j--;
        }
        c->number[i] = carry;
        carry >>= 32;
    }
}

void bn_reset(bn *bn)
{
    for (int i = 0; i < bn->size; i++) {
        bn->number[i] = 0;
    }
}

#endif

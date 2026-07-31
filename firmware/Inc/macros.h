#ifndef MACROS_H
#define MACROS_H

#define FLAG_SET(x, f)  ((x) |=  (f))
#define FLAG_CLR(x, f)  ((x) &= ~(f))
#define FLAG_TOG(x, f)  ((x) ^=  (f))
#define FLAG_HAS(x, f)  (((x) & (f)) != 0)

#define ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))


#define POW2_SPREAD_1(x)    ((x) | ((x) >> 1))
#define POW2_SPREAD_2(x)    (POW2_SPREAD_1(x) | \
                            (POW2_SPREAD_1(x) >> 2))
#define POW2_SPREAD_4(x)    (POW2_SPREAD_2(x) | \
                            (POW2_SPREAD_2(x) >> 4))
#define POW2_SPREAD_8(x)    (POW2_SPREAD_4(x) | \
                            (POW2_SPREAD_4(x) >> 8))
#define POW2_SPREAD_16(x)   (POW2_SPREAD_8(x) | \
                            (POW2_SPREAD_8(x) >> 16))
#define POW2_SPREAD_32(x)   (POW2_SPREAD_16((x) - 1U) + 1U)

#define ROUND_UP_POW2(x)    ((x) <= 1U ? 1U : (POW2_SPREAD_32(x)))

#endif

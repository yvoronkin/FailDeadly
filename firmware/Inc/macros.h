#ifndef MACROS_H
#define MACROS_H

#define FLAG_SET(x, f)  ((x) |=  (f))
#define FLAG_CLR(x, f)  ((x) &= ~(f))
#define FLAG_TOG(x, f)  ((x) ^=  (f))
#define FLAG_HAS(x, f)  (((x) & (f)) != 0)

#define ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))

#endif

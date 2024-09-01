


#ifndef __SL_COMMON_H_
#define __SL_COMMON_H_

#include <stdint.h>

// C99 or later
#ifndef __PTR64__
#if UINTPTR_MAX == 0xFFFF
  #define __PTR16__
#elif UINTPTR_MAX == 0xFFFFFFFF
  #define __PTR32__
#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFFu
  #define __PTR64__
#else
  #error TBD pointer size
#endif
#endif

#ifndef __member_size
#define __member_size(type, member) (sizeof( ((type *)0)->member ))
#endif


#endif /* __SL_COMMON_H_ */
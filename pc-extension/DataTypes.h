#if !defined(_DATATYPES_H)
#define _DATATYPES_H

//#pragma ident	"@(#)DataTypes.h 1.1	04/03/01 SMI"


//#ifdef sun

#include <linux/types.h>
typedef int8_t int8_T;
typedef int16_t int16_T;
typedef int32_t int32_T;
typedef int64_t int64_T;
typedef uint8_t uint8_T;
typedef uint16_t uint16_T;
typedef uint32_t uint32_T;
typedef uint64_t uint64_T;
typedef u_int uint_T;
typedef int int_T;
typedef char boolean_T;
typedef double real_T;

//#define __inline

//#else


//#include "tmwtypes.h"
/*
#if !defined(INT64_T)
#ifdef _MSC_VER
typedef __int64 int64_T;
#else
typedef long long int64_T;
#endif
#endif

#if !defined(UINT64_T)
#ifdef _MSC_VER
typedef unsigned __int64 uint64_T;
#else
typedef unsigned long long uint64_T;
#endif
#endif
*/
//#endif /* !sun */

#if !defined(TRUE)
#define TRUE    (1)
#endif

#if !defined(FALSE)
#define FALSE   (0)
#endif

#if !defined(EXTERN)
#if defined(__cplusplus)
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif
#endif

#if !defined(INLINE)
#define INLINE __inline //static 
#endif

#endif /* _DATATYPES_H */


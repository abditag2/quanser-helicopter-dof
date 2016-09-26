#if !defined(_Q8DI_H)
#define _Q8DI_H

#include "q8dig.h"

/*********************************************************************

  Name:         Q8_diInput

  Purpose:      Read the digital inputs.

---------------------------------------------------------------------*/
INLINE uint32_T Q8_diInput(PQ8 psQ8)
{
    return ReadDWordFromHwMem(&psQ8->m_pRegisters->digitalIO);
}

#endif


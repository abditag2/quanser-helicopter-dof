#if !defined(_Q8DO_H)
#define _Q8DO_H

#include "q8dig.h"

#define Q8_doIsInitialOutputParamOK(val)  (((val) == 0) || ((val) == 1))
#define Q8_doIsFinalOutputParamOK(val)    (((val) == 0) || ((val) == 1))

/*********************************************************************

  Name:         Q8_doOutput

  Purpose:      Write the digital outputs.

---------------------------------------------------------------------*/
INLINE void Q8_doOutput(PQ8 psQ8, uint32_T nValues)
{
    WriteDWordToHwMem(&psQ8->m_pRegisters->digitalIO, nValues);
}

#endif


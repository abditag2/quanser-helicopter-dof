#if !defined(_Q8DIG_H)
#define _Q8DIG_H

#include "q8.h"

#define Q8_NUMBER_OF_DIG_CHANNELS  (32)

/*********************************************************************

  Name:         Q8_digIsNumChannelsParamOK

  Purpose:      Checks that the number of channels is within range
                of the number of channels on the card.

---------------------------------------------------------------------*/
INLINE boolean_T Q8_digIsNumChannelsParamOK(int_T numChannels)
{
    return (numChannels > 0 && numChannels <= Q8_NUMBER_OF_DIG_CHANNELS);
}

/*********************************************************************

  Name:         Q8_digConfigure

  Purpose:      Configure the direction of the digital I/O pins.

---------------------------------------------------------------------*/
INLINE void Q8_digConfigure(PQ8 psQ8, uint32_T nDirections)
{
    WriteDWordToHwMem(&psQ8->m_pRegisters->digitalDirection, nDirections);
    psQ8->m_nDigitalDirections = nDirections;
}

#endif


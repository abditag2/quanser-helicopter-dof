#if !defined(_Q8DA_H)
#define _Q8DA_H

#include "DataTypes.h"
#include "Hardware.h"

#if !defined(_Q8_H)
#include "q8.h"
#endif

#define Q8_NUMBER_OF_DAC_CHANNELS   (8)
#define Q8_DAC_MIN_VOLTAGE          (-10.0)
#define Q8_DAC_MAX_VOLTAGE          (+10.0)
/*#define Q8_DAC_MAX_VOLTAGE          (+9.9951171875) */

#define Q8_DAC_RESOLUTION  (12)
#define Q8_DAC_ZERO        (0x0800)

/********* Macros for testing parameter validity **********/

#define Q8_dacIsNumChannelsParamOK(num)    \
    ((num) > 0 && (num) <= Q8_NUMBER_OF_DAC_CHANNELS)
#define Q8_dacIsInitialOutputParamOK(out, isVoltage)  \
    Q8_dacIsInRange(out, isVoltage)
#define Q8_dacIsFinalOutputParamOK(out, isVoltage)  \
    Q8_dacIsInRange(out, isVoltage)

/********* Common functions **********/

#ifndef linux
INLINE boolean_T Q8_dacIsInRange(real_T out, boolean_T isVoltage)
{
    if (isVoltage)
        return (out >= Q8_DAC_MIN_VOLTAGE && out <= Q8_DAC_MAX_VOLTAGE);
    else
        return (out >= 0 && out <= ((1 << Q8_DAC_RESOLUTION) - 1));
}
#endif
INLINE uint16_T Q8_dacVoltageToOutput( real_T nVoltage, boolean_T bBipolar, real_T nRange )
{
    if (bBipolar)
    {
        const real_T nOneLSB = 2 * nRange / (1 << Q8_DAC_RESOLUTION); // volts per LSB
        if (nVoltage < -nRange + nOneLSB/2)
            return 0;
        else if (nVoltage >= nRange - 1.5*nOneLSB)
            return 0x0fffL;
        else
            return ((uint16_T)(nVoltage / nOneLSB) + Q8_DAC_ZERO);
    }
    else
    {
        const real_T nOneLSB = nRange / (1 << Q8_DAC_RESOLUTION); // volts per LSB
        if (nVoltage < nOneLSB/2)
            return 0;
        else if (nVoltage >= nRange - 1.5*nOneLSB)
            return 0x0fff;
        else
            return (uint16_T)(nVoltage / nOneLSB);
    }
}

#ifndef linux
INLINE void Q8_dacPreconfigure(PQ8 psQ8, uint8_T nChannel, boolean_T bBipolar, real_T nRange)
#else
INLINE void Q8_dacPreconfigure(PQ8 psQ8, uint8_T nChannel, boolean_T bBipolar, int16_T nRange)
#endif
{
    if (nChannel < 4)
    {
        if (bBipolar)
        {
            psQ8->m_nDacModes |= (0x0010 << (3 - nChannel));
            if (nRange == 10)
                psQ8->m_nDacModes |= (0x0100 << (3 - nChannel));
            else
                psQ8->m_nDacModes &= ~(0x0100 << (3 - nChannel));
        }
        else
            psQ8->m_nDacModes &= ~(0x0110 << (3 - nChannel));
    }
    else
    {
        if (bBipolar)
        {
            psQ8->m_nDacModes |= (0x00100000 << (7 - nChannel));
            if (nRange == 10)
                psQ8->m_nDacModes |= (0x01000000 << (7 - nChannel));
            else
                psQ8->m_nDacModes &= ~(0x01000000 << (7 - nChannel));
        }
        else
            psQ8->m_nDacModes &= ~(0x01100000 << (7 - nChannel));
    }
}

#ifndef linux
INLINE void Q8_dacConfigure(PQ8 psQ8, uint8_T nChannel, boolean_T bBipolar, real_T nRange)
#else
INLINE void Q8_dacConfigure(PQ8 psQ8, uint8_T nChannel, boolean_T bBipolar, int16_T nRange)
#endif
{
    Q8_dacPreconfigure(psQ8, nChannel, bBipolar, nRange);

    WriteDWordToHwMem(&psQ8->m_pRegisters->analogMode.all, psQ8->m_nDacModes);
    WriteDWordToHwMem(&psQ8->m_pRegisters->analogModeUpdate.all, 0);
}

INLINE void Q8_dacPreload(PQ8 psQ8, uint8_T nChannels, uint16_T *pnValues)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;
    volatile uint32_T *pnAnalogOutputs = &pRegisters->analogOutput.pairs[0];
    int16_T * pnValues47      = pnValues;
    int_T     idx;

    /* Determine the number of channels in the range 0-3 */
    for (idx=3; idx >= 0; --idx)
        pnValues47 += (nChannels >> idx) & 1;

    while (nChannels != 0)
    {
        if ((nChannels & 0x11) == 0x11) /* pair of channels can be written at the same time */
            WriteDWordToHwMem(pnAnalogOutputs, *(pnValues++) | (*(pnValues47++) << 16));
        else if (nChannels & 0x01)
            WriteWordToHwMem(pnAnalogOutputs, *(pnValues++));
        else if (nChannels & 0x10)
            WriteWordToHwMem((uint16_T *)pnAnalogOutputs + 1, *(pnValues47++));

        nChannels = (nChannels & ~0x11) >> 1;
        pnAnalogOutputs++;
    }
}

INLINE void Q8_dacPreloadSingle(PQ8 psQ8, uint8_T nChannel, uint16_T nValue)
{
    static const int_T dacMap[8] = { 0, 2, 4, 6, 1, 3, 5, 7 };

    WriteWordToHwMem(&psQ8->m_pRegisters->analogOutput.dacs[dacMap[nChannel]], nValue);
}

INLINE void Q8_dacUpdate(PQ8 psQ8)
{
    /* Update all DAC outputs simultaneously */
    WriteDWordToHwMem(&psQ8->m_pRegisters->analogUpdate.all, 0);
}

INLINE void Q8_dacOutput(PQ8 psQ8, uint8_T nChannels, uint16_T *pnValues)
{
    Q8_dacPreload(psQ8, nChannels, pnValues);
    Q8_dacUpdate(psQ8);
}

INLINE void Q8_dacRead(PQ8 psQ8, uint8_T nChannels, uint16_T *pnValues)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;
    volatile uint32_T *pnAnalogOutputs = &pRegisters->analogOutput.pairs[0];
    int16_T * pnValues47 = pnValues;
    int_T     idx;

    /* Determine the number of channels in the range 0-3 */
    for (idx=3; idx >= 0; --idx)
        pnValues47 += (nChannels >> idx) & 1;

    while (nChannels != 0)
    {
        if ((nChannels & 0x11) == 0x11) /* pair of channels can be written at the same time */
		{
            uint32_T values = ReadDWordFromHwMem(pnAnalogOutputs);
			*(pnValues++)   = (uint16_T)(values & 0xffff);
			*(pnValues47++) = (uint16_T)((values >> 16) & 0xffff);
		}
        else if (nChannels & 0x01)
            *(pnValues++) = ReadWordFromHwMem(pnAnalogOutputs);
        else if (nChannels & 0x10)
            *(pnValues47++) = ReadWordFromHwMem((uint16_T *)pnAnalogOutputs + 1);

        nChannels = (nChannels & ~0x11) >> 1;
        pnAnalogOutputs++;
    }
}

INLINE real_T Q8_dacOutputToVoltage( uint16_T nOutput, real_T nRange )
{
    if (nRange < 0)
    {
        const real_T nOneLSB = -2 * nRange / (1 << Q8_DAC_RESOLUTION); // volts per LSB
		return (nOutput - Q8_DAC_ZERO) * nOneLSB;
    }
    else
    {
        const real_T nOneLSB = nRange / (1 << Q8_DAC_RESOLUTION); // volts per LSB
		return nOutput * nOneLSB;
    }
}

#ifndef linux
INLINE real_T Q8_dacRange(PQ8 psQ8, uint8_T nChannel)
#else
INLINE int16_T Q8_dacRange(PQ8 psQ8, uint8_T nChannel)
#endif
{
    if (nChannel < 4)
    {
		if (psQ8->m_nDacModes & (0x0010 << (3 - nChannel))) // bipolar
		{
			return (psQ8->m_nDacModes & (0x0100 << (3 - nChannel))) ? -10 : -5;
		}
		else // unipolar
			return 10; // only 0-10V range supported
    }
    else
    {
		if (psQ8->m_nDacModes & (0x00100000 << (7 - nChannel))) // bipolar
		{
			return (psQ8->m_nDacModes & (0x01000000 << (7 - nChannel))) ? -10 : -5;
		}
		else // unipolar
			return 10; // only 0-10V range supported
    }
}



#endif

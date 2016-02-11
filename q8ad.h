#if !defined(_Q8AD_H)
#define _Q8AD_H

#include "DataTypes.h"
#include "Hardware.h"

#if !defined(_Q8_H)
#include "q8.h"
#endif

#if !defined(_Q8TB_H)
#include "q8tb.h"
#endif

#define Q8_NUMBER_OF_ADC_CHANNELS   (8)
#define Q8_ADC_MAX_VOLTAGE          (+10.0)
#define Q8_ADC_MIN_VOLTAGE          (-10.0)
#define Q8_ADC_RESOLUTION           (14)

/********* Macros **********/

#define Q8_adcIsNumChannelsParamOK(num)    \
    ((num) > 0  && (num) <= Q8_NUMBER_OF_ADC_CHANNELS)
#define Q8_adcIsRangeParamOK(num)          \
    ((num) == 5 || (num) == 10)

#define Q8_ADC_ONE_VOLT     ((Q8_ADC_MAX_VOLTAGE - Q8_ADC_MIN_VOLTAGE) / ((1 << Q8_ADC_RESOLUTION) - 1))

/********* Common functions **********/

/*********************************************************************

  Name:         Q8_adcInputToVoltage

  Purpose:      Convert from a raw A/D value to a voltage.

---------------------------------------------------------------------*/
INLINE real_T Q8_adcInputToVoltage(int_T value)
{
    return value * Q8_ADC_ONE_VOLT;
}
/*********************************************************************

  Name:         Q8_adcInput

  Purpose:      Read the selected A/D channels. If the simultaneous flag
                is specified then the inputs are sampled simultaneously.
                Note that when only one A/D chip is used (channels 0-3 only
                or channels 4-7 only), the inputs are always sampled
                simultaneously. The conversion results are read after all
                the conversions are complete.

---------------------------------------------------------------------*/
INLINE boolean_T Q8_adcInput(PQ8 psQ8, uint8_T nChannels, int16_T *nValues, boolean_T bSimultaneous)
{
    register PQ8Registers pRegisters = psQ8->m_pRegisters;

    if ((nChannels & 0xf0) == 0) /* only channels in the range 0-3 */
    {
        /* Select the internal A/D clock, and channel selection via the Control Register. Also set the selected channels */
        uint32_T nControl = (psQ8->m_nControl & ~CTRL_ADC03MASK) | ((nChannels & 0x0f) << 8);
        WriteDWordToHwMem(&pRegisters->control, nControl);

        /* Start conversions on all channels */
        WriteDWordToHwMem(&pRegisters->control, nControl | CTRL_ADC03CV);

        /* Wait until the conversions are complete */
        while ((ReadDWordFromHwMem(&pRegisters->status) & STAT_ADC03RDY) == 0);

        /* Read the conversion results */
        while (nChannels != 0)
        {
            if (nChannels & 1)
                *(nValues++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc03);
            nChannels >>= 1;
        }
    }
    else if ((nChannels & 0x0f) == 0) /* only channels in the range 4-7 */
    {
        /* Select the internal A/D clock, and channel selection via the Control Register. Also set the selected channels */
        uint32_T nControl = (psQ8->m_nControl & ~CTRL_ADC47MASK) | ((nChannels & 0xf0) << 12);
        WriteDWordToHwMem(&pRegisters->control, nControl);

        /* Start conversions on all channels */
        WriteDWordToHwMem(&pRegisters->control, nControl | CTRL_ADC47CV);

        /* Wait until the conversions are complete */
        while ((ReadDWordFromHwMem(&pRegisters->status) & STAT_ADC47RDY) == 0);

        /* Read the conversion results */
        nChannels >>= 4;
        while (nChannels != 0)
        {
            if (nChannels & 1)
                *(nValues++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc47);
            nChannels >>= 1;
        }
    }
    else /* channels in the range 0-7 */
    {
        const uint32_T nMask = STAT_ADC47RDY | STAT_ADC03RDY;
        int_T    nAdc03Reads;  /* number of reads to perform on ADC03 */
        int_T    nAdc47Reads;  /* number of reads to perform on ADC47 */
        int16_T *pnValues03 = nValues;
        int16_T *pnValues47;
        uint32_T nControl;
        int_T    i;
        
        /* Compute the offset for values in the range 4-7 */
        nAdc03Reads = 0;
        for (i=3; i >= 0; --i)
            nAdc03Reads += (nChannels >> i) & 1;

        nAdc47Reads = 0;
        for (i=7; i >= 4; --i)
            nAdc47Reads += (nChannels >> i) & 1;

        pnValues47 = nValues + nAdc03Reads;

        if (bSimultaneous)
        {
            /* Select the common A/D clock, and channel selection via the A/D Register */
            nControl = (psQ8->m_nControl & ~CTRL_ADCMASK) | CTRL_ADC47HS | CTRL_ADC03HS | CTRL_ADC47SCK | CTRL_ADC03SCK;
            WriteDWordToHwMem(&pRegisters->control, nControl);

            /* Write to the A/D Register to select the channels to convert */
            WriteDWordToHwMem(&pRegisters->analogInput.select, (((uint16_T)nChannels & 0xf0) << 12) | (nChannels & 0x0f));
        }
        else
        {
            /* Select the internal A/D clocks, and channel selection via the Control Register. Select the channels. */
            nControl = (psQ8->m_nControl & ~CTRL_ADCMASK) | ((nChannels & 0x0f) << 8) | ((nChannels & 0xf0) << 12);
            WriteDWordToHwMem(&pRegisters->control, nControl);
        }

        /* Start conversions on all channels */
        WriteDWordToHwMem(&pRegisters->control, nControl | CTRL_ADC47CV | CTRL_ADC03CV);

        /* Wait for conversions to complete */
        while ((ReadDWordFromHwMem(&pRegisters->status) & nMask) != nMask);

        /* Read the results from the FIFO */
        while (nAdc03Reads > 0 && nAdc47Reads > 0) /* channels left to read on both chips */
        {
            uint32_T nResults = ReadDWordFromHwMem(&pRegisters->analogInput.two); /* Read two channels simultaneously */
            *(pnValues03++) = (int16_T)(nResults);
            *(pnValues47++) = (int16_T)(nResults >> 16);
            --nAdc03Reads;
            --nAdc47Reads;
        }

        while (--nAdc03Reads >= 0) /* channels left on ADC03 */
            *(pnValues03++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc03);

        while (--nAdc47Reads >= 0) /* channels left on ADC47 */
            *(pnValues47++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc47);
    }

    return TRUE;
}


/*********************************************************************

  Name:         Q8_adcInputUsingEOC

  Purpose:      Read the selected A/D channels. If the simultaneous flag
                is specified then the inputs are sampled simultaneously.
                Note that when only one A/D chip is used (channels 0-3 only
                or channels 4-7 only), the inputs are always sampled
                simultaneously. The conversion results are read as they
                become available. Note that this technique is faster
                but may inject some noise into the measurements.

---------------------------------------------------------------------*/
INLINE boolean_T Q8_adcInputUsingEOC(PQ8 psQ8, uint8_T nChannels, int16_T *nValues, boolean_T bSimultaneous)
{
    register PQ8Registers pRegisters = psQ8->m_pRegisters;

    if ((nChannels & 0xf0) == 0) /* only channels in the range 0-3 */
    {
        /* Select the internal A/D clock, and channel selection via the Control Register. Also set the selected channels */
        uint32_T nControl = (psQ8->m_nControl & ~CTRL_ADC03MASK) | ((nChannels & 0x0f) << 8);
        WriteDWordToHwMem(&pRegisters->control, nControl);

        /* Clear the Interrupt Status Register */
        WriteDWordToHwMem(&pRegisters->interruptStatus, INT_ADC03EOC | INT_ADC03RDY);

        /* Start conversions on all channels */
        WriteDWordToHwMem(&pRegisters->control, nControl | CTRL_ADC03CV);

        /* Read each channel as soon as it is converted */
        while (nChannels != 0)
        {
            if (nChannels & 1)
            {
                /* Wait for channel to be converted */
                while ((ReadDWordFromHwMem(&pRegisters->interruptStatus) & (INT_ADC03EOC | INT_ADC03RDY)) == 0);

                /* Read channel value */
                *(nValues++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc03);

                /* Clear the EOC bit in the Interrupt Status Register. Do not clear the RDY bit so that if we miss an EOC we will
                 * not wait for one once all the channels are converted. */
                WriteDWordToHwMem(&pRegisters->interruptStatus, INT_ADC03EOC);
            }

            nChannels >>=1 ;
        }
    }
    else if ((nChannels & 0x0f) == 0) /* only channels in the range 4-7 */
    {
        /* Select the internal A/D clock, and channel selection via the Control Register. Also set the selected channels */
        uint32_T nControl = (psQ8->m_nControl & ~CTRL_ADC47MASK) | ((nChannels & 0xf0) << 12);
        WriteDWordToHwMem(&pRegisters->control, nControl);

        /* Clear the Interrupt Status Register */
        WriteDWordToHwMem(&pRegisters->interruptStatus, -1);

        /* Start conversions on all channels */
        WriteDWordToHwMem(&pRegisters->control, nControl | CTRL_ADC47CV);

        /* Read each channel as soon as it is converted */
        nChannels >>= 4;
        while (nChannels != 0)
        {
            if (nChannels & 1)
            {
                /* Wait for channel to be converted */
                while ((ReadDWordFromHwMem(&pRegisters->interruptStatus) & (INT_ADC47EOC | INT_ADC47RDY)) == 0);

                /* Read channel value */
                *(nValues++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc47);

                /* Clear the EOC bit in the Interrupt Status Register. Do not clear the RDY bit so that if we miss an EOC we will
                 * not wait for one once all the channels are converted. */
                WriteDWordToHwMem(&pRegisters->interruptStatus, INT_ADC47EOC);
            }

            nChannels >>=1 ;
        }
    }
    else /* channels in the range 0-7 */
    {
        int_T    nAdc03Reads;  /* number of reads to perform on ADC03 */
        int_T    nAdc47Reads;  /* number of reads to perform on ADC47 */
        int16_T *pnValues03 = nValues;
        int16_T *pnValues47;
        uint32_T nControl;
        int_T    i;

        /* Compute the offset for values in the range 4-7 */
        nAdc03Reads = 0;
        for (i=3; i >= 0; --i)
            nAdc03Reads += (nChannels >> i) & 1;

        nAdc47Reads = 0;
        for (i=7; i >= 4; --i)
            nAdc47Reads += (nChannels >> i) & 1;

        pnValues47 = nValues + nAdc03Reads;

        if (bSimultaneous)
        {
            /* Select the common A/D clock, and channel selection via the A/D Register */
            nControl = (psQ8->m_nControl & ~CTRL_ADCMASK) | CTRL_ADC47HS | CTRL_ADC03HS | CTRL_ADC47SCK | CTRL_ADC03SCK;
            WriteDWordToHwMem(&pRegisters->control, nControl);

            /* Write to the A/D Register to select the channels to convert */
            WriteDWordToHwMem(&pRegisters->analogInput.select, (((uint16_T)nChannels & 0xf0) << 12) | (nChannels & 0x0f));
        }
        else
        {
            /* Select the internal A/D clocks, and channel selection via the Control Register. Select the channels. */
            nControl = (psQ8->m_nControl & ~CTRL_ADCMASK) | ((nChannels & 0x0f) << 8) | ((nChannels & 0xf0) << 12);
            WriteDWordToHwMem(&pRegisters->control, nControl);
        }

        /* Clear the Interrupt Status Register */
        WriteDWordToHwMem(&pRegisters->interruptStatus, INT_ADC03EOC | INT_ADC03RDY | INT_ADC47EOC | INT_ADC47RDY);

        /* Start conversions on all channels */
        WriteDWordToHwMem(&pRegisters->control, nControl | CTRL_ADC47CV | CTRL_ADC03CV);

        /* Read the results as they become available */
        while (nAdc03Reads > 0 && nAdc47Reads > 0) /* channels left to read on both chips */
        {
            uint32_T nResults;

            /* Wait for at least one channel on each A/D chip to be converted */
            while ((ReadDWordFromHwMem(&pRegisters->interruptStatus) & (INT_ADC03EOC | INT_ADC03RDY)) == 0);
            while ((ReadDWordFromHwMem(&pRegisters->interruptStatus) & (INT_ADC47EOC | INT_ADC47RDY)) == 0);

            nResults = ReadDWordFromHwMem(&pRegisters->analogInput.two); /* Read two channels simultaneously */

            /* Clear the EOC bit in the Interrupt Status Register. Do not clear the RDY bit so that if we miss an EOC we will
             * not wait for one once all the channels are converted. */
            WriteDWordToHwMem(&pRegisters->interruptStatus, INT_ADC03EOC | INT_ADC47EOC);

            *(pnValues03++) = (int16_T)(nResults);
            *(pnValues47++) = (int16_T)(nResults >> 16);
            --nAdc03Reads;
            --nAdc47Reads;
        }

        while (--nAdc03Reads >= 0) /* channels left on ADC03 */
        {
            while ((ReadDWordFromHwMem(&pRegisters->interruptStatus) & (INT_ADC03EOC | INT_ADC03RDY)) == 0);
            *(pnValues03++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc03);
            WriteDWordToHwMem(&pRegisters->interruptStatus, INT_ADC03EOC);
        }

        while (--nAdc47Reads >= 0) /* channels left on ADC47 */
        {
            while ((ReadDWordFromHwMem(&pRegisters->interruptStatus) & (INT_ADC47EOC | INT_ADC47RDY)) == 0);
            *(pnValues47++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc47);
            WriteDWordToHwMem(&pRegisters->interruptStatus, INT_ADC47EOC);
        }
    }

    return TRUE;
}

/*********************************************************************

  Name:         Q8_adcPreprogram

  Purpose:      Preprogram the A/D chips for the selected channels.
                This function is used when there is only one
                Analog Input block in the diagram.

---------------------------------------------------------------------*/
INLINE void Q8_adcPreprogram(PQ8 psQ8, uint8_T nChannels, boolean_T bSimultaneous)
{
    register PQ8Registers pRegisters = psQ8->m_pRegisters;
    register int_T i;

    /* Write to the A/D Register to select the channels to convert */
    WriteDWordToHwMem(&pRegisters->analogInput.select, (((uint16_T)nChannels & 0xf0) << 12) | (nChannels & 0x0f));

    /* Write to the control register to configure the A/D conversions */
    if ((nChannels & 0xf0) == 0) /* only channels in the range 0-3 */
    {
        /* use internal clock (always simultaneous sampling), A/D Register selects channels */
        psQ8->m_nControl = (psQ8->m_nControl & ~CTRL_ADC03MASK) | CTRL_ADC03HS;
    }
    else if ((nChannels & 0x0f) == 0) /* only channels in the range 4-7 */
    {
        /* use internal clock (always simultaneous sampling), A/D Register selects channels */
        psQ8->m_nControl = (psQ8->m_nControl & ~CTRL_ADC47MASK) | CTRL_ADC47HS;
    }
    else if (bSimultaneous) /* channels in the range 0-7 */
    {
        /* common clock for guaranteed simultaneous sampling across both A/D chips, A/D Register selects channels */
        psQ8->m_nControl = (psQ8->m_nControl & ~CTRL_ADCMASK) | CTRL_ADC03HS | CTRL_ADC47HS | CTRL_ADC03SCK | CTRL_ADC47SCK;
    }
    else /* channels in the range 0-7 */
    {
        /* internal clocks, A/D Register selects channels */
        psQ8->m_nControl = (psQ8->m_nControl & ~CTRL_ADCMASK) | CTRL_ADC03HS | CTRL_ADC47HS;
    }
    WriteDWordToHwMem(&pRegisters->control, psQ8->m_nControl);

    /* Precalculate the number of channels being converted on each A/D chip */
    psQ8->m_nAdc03Reads = 0;
    for (i=3; i >= 0; --i)
        psQ8->m_nAdc03Reads += (nChannels >> i) & 1;

    psQ8->m_nAdc47Reads = 0;
    for (i=7; i >= 4; --i)
        psQ8->m_nAdc47Reads += (nChannels >> i) & 1;
}

/*********************************************************************

  Name:         Q8_adcPreprogrammedInput

  Purpose:      Read the selected A/D channels. If the simultaneous flag
                is specified then the inputs are sampled simultaneously.
                Note that when only one A/D chip is used (channels 0-3 only
                or channels 4-7 only), the inputs are always sampled
                simultaneously. The conversion results are read after all
                the conversions are complete.

                This function assumes that the A/D chips have been
                preprogrammed for the selected channels using
                Q8_adcPreprogram.

---------------------------------------------------------------------*/
INLINE boolean_T Q8_adcPreprogrammedInput(PQ8 psQ8, uint8_T nChannels, int16_T *nValues)
{
    register PQ8Registers pRegisters = psQ8->m_pRegisters;

    if ((nChannels & 0xf0) == 0) /* only channels in the range 0-3 */
    {
        int_T nAdc03Reads = psQ8->m_nAdc03Reads;

        /* Start conversions on all channels */
        WriteDWordToHwMem(&pRegisters->control, psQ8->m_nControl | CTRL_ADC03CV);

        /* Wait until the conversions are complete */
        while ((ReadDWordFromHwMem(&pRegisters->status) & STAT_ADC03RDY) == 0);

        /* Read the conversion results */
        while (--nAdc03Reads >= 0)
            *(nValues++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc03);
    }
    else if ((nChannels & 0x0f) == 0) /* only channels in the range 4-7 */
    {
        int_T nAdc47Reads = psQ8->m_nAdc47Reads;

        /* Start conversions on all channels */
        WriteDWordToHwMem(&pRegisters->control, psQ8->m_nControl | CTRL_ADC47CV);

        /* Wait until the conversions are complete */
        while ((ReadDWordFromHwMem(&pRegisters->status) & STAT_ADC47RDY) == 0);

        /* Read the conversion results */
        while (--nAdc47Reads >= 0)
            *(nValues++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc47);
    }
    else /* channels in the range 0-7 */
    {
        const uint32_T nMask = STAT_ADC47RDY | STAT_ADC03RDY;
        int_T    nAdc03Reads = psQ8->m_nAdc03Reads;  /* number of reads to perform on ADC03 */
        int_T    nAdc47Reads = psQ8->m_nAdc47Reads;  /* number of reads to perform on ADC47 */
        int16_T *pnValues03  = nValues;
        int16_T *pnValues47  = nValues + psQ8->m_nAdc03Reads;
        
        /* Start conversions on all channels */
        WriteDWordToHwMem(&pRegisters->control, psQ8->m_nControl | CTRL_ADC47CV | CTRL_ADC03CV);

        /* Wait for all the conversions to complete */
        while ((ReadDWordFromHwMem(&pRegisters->status) & nMask) != nMask);

        /* Read the results from the FIFO */
        while (nAdc03Reads > 0 && nAdc47Reads > 0) /* channels left to read on both chips */
        {
            uint32_T nResults = ReadDWordFromHwMem(&pRegisters->analogInput.two); /* Read two channels simultaneously */
            *(pnValues03++) = (int16_T)(nResults);
            *(pnValues47++) = (int16_T)(nResults >> 16);
            --nAdc03Reads;
            --nAdc47Reads;
        }

        while (--nAdc03Reads >= 0) /* channels left on ADC03 */
            *(pnValues03++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc03);

        while (--nAdc47Reads >= 0) /* channels left on ADC47 */
            *(pnValues47++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc47);
    }

    return TRUE;
}


/*********************************************************************

  Name:         Q8_adcPreprogrammedInputUsingEOC

  Purpose:      Read the selected A/D channels. If the simultaneous flag
                is specified then the inputs are sampled simultaneously.
                Note that when only one A/D chip is used (channels 0-3 only
                or channels 4-7 only), the inputs are always sampled
                simultaneously. The conversion results are read as they
                become available. Note that this technique is faster
                but may inject some noise into the measurements.

                This function assumes that the A/D chips have been
                preprogrammed for the selected channels using
                Q8_adcPreprogram.

---------------------------------------------------------------------*/
INLINE boolean_T Q8_adcPreprogrammedInputUsingEOC(PQ8 psQ8, uint8_T nChannels, int16_T *nValues)
{
    register PQ8Registers pRegisters = psQ8->m_pRegisters;

    if ((nChannels & 0xf0) == 0) /* only channels in the range 0-3 */
    {
        int_T nAdc03Reads = psQ8->m_nAdc03Reads;

        /* Clear the Interrupt Status Register */
        WriteDWordToHwMem(&pRegisters->interruptStatus, INT_ADC03EOC | INT_ADC03RDY);

        /* Start conversions on all channels */
        WriteDWordToHwMem(&pRegisters->control, psQ8->m_nControl | CTRL_ADC03CV);

        /* Read each channel as soon as it is converted */
        while (--nAdc03Reads >= 0)
        {
            /* Wait for channel to be converted */
            while ((ReadDWordFromHwMem(&pRegisters->interruptStatus) & (INT_ADC03EOC | INT_ADC03RDY)) == 0);

            /* Read channel value */
            *(nValues++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc03);

            /* Clear the EOC bit in the Interrupt Status Register. Do not clear the RDY bit so that if we miss an EOC we will
             * not wait for one once all the channels are converted. */
            WriteDWordToHwMem(&pRegisters->interruptStatus, INT_ADC03EOC);
        }
    }
    else if ((nChannels & 0x0f) == 0) /* only channels in the range 4-7 */
    {
        int_T nAdc47Reads = psQ8->m_nAdc47Reads;

        /* Clear the Interrupt Status Register */
        WriteDWordToHwMem(&pRegisters->interruptStatus, INT_ADC47EOC | INT_ADC47RDY);

        /* Start conversions on all channels */
        WriteDWordToHwMem(&pRegisters->control, psQ8->m_nControl | CTRL_ADC47CV);

        /* Read each channel as soon as it is converted */
        while (--nAdc47Reads >= 0)
        {
            /* Wait for channel to be converted */
            while ((ReadDWordFromHwMem(&pRegisters->interruptStatus) & (INT_ADC47EOC | INT_ADC47RDY)) == 0);

            /* Read channel value */
            *(nValues++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc47);

            /* Clear the EOC bit in the Interrupt Status Register. Do not clear the RDY bit so that if we miss an EOC we will
             * not wait for one once all the channels are converted. */
            WriteDWordToHwMem(&pRegisters->interruptStatus, INT_ADC47EOC);
        }
    }
    else /* channels in the range 0-7 */
    {
        int_T    nAdc03Reads = psQ8->m_nAdc03Reads;  /* number of reads to perform on ADC03 */
        int_T    nAdc47Reads = psQ8->m_nAdc47Reads;  /* number of reads to perform on ADC47 */
        int16_T *pnValues03  = nValues;
        int16_T *pnValues47  = nValues + psQ8->m_nAdc03Reads;

        /* Clear the Interrupt Status Register */
        WriteDWordToHwMem(&pRegisters->interruptStatus, INT_ADC03EOC | INT_ADC03RDY | INT_ADC47EOC | INT_ADC47RDY);

        /* Start conversions on all channels */
        WriteDWordToHwMem(&pRegisters->control, psQ8->m_nControl | CTRL_ADC47CV | CTRL_ADC03CV);

        /* Read the results as they become available */
        while (nAdc03Reads > 0 && nAdc47Reads > 0) /* channels left to read on both chips */
        {
            uint32_T nResults;

            /* Wait for at least one channel on each A/D chip to be converted */
            while ((ReadDWordFromHwMem(&pRegisters->interruptStatus) & (INT_ADC03EOC | INT_ADC03RDY)) == 0);
            while ((ReadDWordFromHwMem(&pRegisters->interruptStatus) & (INT_ADC47EOC | INT_ADC47RDY)) == 0);

            nResults = ReadDWordFromHwMem(&pRegisters->analogInput.two); /* Read two channels simultaneously */

            /* Clear the EOC bit in the Interrupt Status Register. Do not clear the RDY bit so that if we miss an EOC we will
             * not wait for one once all the channels are converted. */
            WriteDWordToHwMem(&pRegisters->interruptStatus, INT_ADC03EOC | INT_ADC47EOC);

            *(pnValues03++) = (int16_T)(nResults);
            *(pnValues47++) = (int16_T)(nResults >> 16);
            --nAdc03Reads;
            --nAdc47Reads;
        }

        while (--nAdc03Reads >= 0) /* channels left on ADC03 */
        {
            while ((ReadDWordFromHwMem(&pRegisters->interruptStatus) & (INT_ADC03EOC | INT_ADC03RDY)) == 0);
            *(pnValues03++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc03);
            WriteDWordToHwMem(&pRegisters->interruptStatus, INT_ADC03EOC);
        }

        while (--nAdc47Reads >= 0) /* channels left on ADC47 */
        {
            while ((ReadDWordFromHwMem(&pRegisters->interruptStatus) & (INT_ADC47EOC | INT_ADC47RDY)) == 0);
            *(pnValues47++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc47);
            WriteDWordToHwMem(&pRegisters->interruptStatus, INT_ADC47EOC);
        }
    }

    return TRUE;
}

/*********************************************************************

  Name:         Q8_adcPreprogramUsingCounter

  Purpose:      Initialize the counter to drive the A/D converter(s).
                This function is used by the Analog Time Base block
                to perform A/D conversions periodically and interrupt
                on conversion complete. Note that simultaneous sampling
                is always used to guarantee that all conversion results
                are available as soon as the interrupt occurs (only true
				if we interrupt on the right end of conversion signal!)

---------------------------------------------------------------------*/
INLINE void Q8_adcPreprogramUsingCounter(PQ8 psQ8, uint8_T nChannels, real_T nPeriod)
{
    uint32_T period = (uint32_T)(nPeriod / Q8_COUNTER_TIMEBASE) - 1;
    register PQ8Registers pRegisters = psQ8->m_pRegisters;
    register int_T i;

    /* Precalculate the number of channels being converted on each A/D chip */
    psQ8->m_nAdc03Reads = 0;
    for (i=3; i >= 0; --i)
        psQ8->m_nAdc03Reads += (nChannels >> i) & 1;

    psQ8->m_nAdc47Reads = 0;
    for (i=7; i >= 4; --i)
        psQ8->m_nAdc47Reads += (nChannels >> i) & 1;

    /* Write to the A/D Register to select the channels to convert */
    WriteDWordToHwMem(&pRegisters->analogInput.select, (((uint16_T)nChannels & 0xf0) << 12) | (nChannels & 0x0f));

    /* Set the counter preload value (which will determine the period) */
    WriteDWordToHwMem(&pRegisters->counter.sq.preload, period);

    /* Enable the counter and force the counter to load the preload value immediately and set the output high. */
    psQ8->m_nCounterControl |= CCTRL_CNTREN;
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_CNTRLD | CCTRL_CNTRVAL);

    /* Write to the control register to configure the A/D conversions */
    if ((nChannels & 0xf0) == 0) /* only channels in the range 0-3 */
    {
        /* use internal clock (always simultaneous sampling), A/D Register selects channels */
        psQ8->m_nControl = (psQ8->m_nControl & ~CTRL_ADC03MASK) | CTRL_ADC03HS | CTRL_ADC03CT;
    }
    else if ((nChannels & 0x0f) == 0) /* only channels in the range 4-7 */
    {
        /* use internal clock (always simultaneous sampling), A/D Register selects channels */
        psQ8->m_nControl = (psQ8->m_nControl & ~CTRL_ADC47MASK) | CTRL_ADC47HS | CTRL_ADC47CT;
    }
    else /* channels in the range 0-7 */
    {
        /* common clock for guaranteed simultaneous sampling across both A/D chips, A/D Register selects channels */
        psQ8->m_nControl = (psQ8->m_nControl & ~CTRL_ADCMASK) | CTRL_ADC03HS | CTRL_ADC47HS | CTRL_ADC03SCK | CTRL_ADC47SCK
            | CTRL_ADC03CT | CTRL_ADC47CT;
    }
    WriteDWordToHwMem(&pRegisters->control, psQ8->m_nControl);
}

/*********************************************************************

  Name:         Q8_adcPreprogramUsingCounterEnable

  Purpose:      Initialize the CNTR_EN line to drive the A/D converter(s).
                This function is used by the Analog Input External Time Base 
                block to perform A/D conversions on the rising or falling
                edge of the CNTR_EN line and to interrupt on conversion complete. 
                Note that simultaneous sampling is always used to guarantee that 
                all conversion results are available as soon as the interrupt occurs.

---------------------------------------------------------------------*/
INLINE void Q8_adcPreprogramUsingCounterEnable(PQ8 psQ8, uint8_T nChannels)
{
    register PQ8Registers pRegisters = psQ8->m_pRegisters;
    register int_T i;

    /* Precalculate the number of channels being converted on each A/D chip */
    psQ8->m_nAdc03Reads = 0;
    for (i=3; i >= 0; --i)
        psQ8->m_nAdc03Reads += (nChannels >> i) & 1;

    psQ8->m_nAdc47Reads = 0;
    for (i=7; i >= 4; --i)
        psQ8->m_nAdc47Reads += (nChannels >> i) & 1;

    /* Write to the A/D Register to select the channels to convert */
    WriteDWordToHwMem(&pRegisters->analogInput.select, (((uint16_T)nChannels & 0xf0) << 12) | (nChannels & 0x0f));

    /* Write to the control register to configure the A/D conversions */
    if ((nChannels & 0xf0) == 0) /* only channels in the range 0-3 */
    {
        /* use internal clock (always simultaneous sampling), A/D Register selects channels */
        psQ8->m_nControl = (psQ8->m_nControl & ~CTRL_ADC03MASK) | CTRL_ADC03HS | CTRL_ADC03CT | CTRL_CNTRENCV;
    }
    else if ((nChannels & 0x0f) == 0) /* only channels in the range 4-7 */
    {
        /* use internal clock (always simultaneous sampling), A/D Register selects channels */
        psQ8->m_nControl = (psQ8->m_nControl & ~CTRL_ADC47MASK) | CTRL_ADC47HS | CTRL_ADC47CT | CTRL_CNTRENCV;
    }
    else /* channels in the range 0-7 */
    {
        /* common clock for guaranteed simultaneous sampling across both A/D chips, A/D Register selects channels */
        psQ8->m_nControl = (psQ8->m_nControl & ~CTRL_ADCMASK) | CTRL_ADC03HS | CTRL_ADC47HS | CTRL_ADC03SCK | CTRL_ADC47SCK
            | CTRL_ADC03CT | CTRL_ADC47CT | CTRL_CNTRENCV;
    }
    WriteDWordToHwMem(&pRegisters->control, psQ8->m_nControl);
}

/*********************************************************************

  Name:         Q8_adcPreprogrammedInputFromCounter

  Purpose:      Read the selected A/D channels. The conversions have
                already taken place and are available in the A/D FIFOs.

                This function assumes that the A/D chips have been
                preprogrammed for the selected channels using
                Q8_adcPreprogramUsingCounter(Enable).

---------------------------------------------------------------------*/
INLINE boolean_T Q8_adcPreprogrammedInputFromCounter(PQ8 psQ8, uint8_T nChannels, int16_T *nValues)
{
    register PQ8Registers pRegisters = psQ8->m_pRegisters;

    if ((nChannels & 0xf0) == 0) /* only channels in the range 0-3 */
    {
        int_T nAdc03Reads = psQ8->m_nAdc03Reads;

        /*
        if ((ReadDWordFromHwMem(&pRegisters->status) & STAT_ADC03FST) != STAT_ADC03FST)
            return FALSE;
        */

        /* Read the conversion results */
        while (--nAdc03Reads >= 0)
            *(nValues++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc03);
    }
    else if ((nChannels & 0x0f) == 0) /* only channels in the range 4-7 */
    {
        int_T nAdc47Reads = psQ8->m_nAdc47Reads;

        /*
        if ((ReadDWordFromHwMem(&pRegisters->status) & STAT_ADC47FST) != STAT_ADC47FST)
            return FALSE;
        */

        /* Read the conversion results */
        while (--nAdc47Reads >= 0)
            *(nValues++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc47);
    }
    else /* channels in the range 0-7 */
    {
        int_T    nAdc03Reads = psQ8->m_nAdc03Reads;  /* number of reads to perform on ADC03 */
        int_T    nAdc47Reads = psQ8->m_nAdc47Reads;  /* number of reads to perform on ADC47 */
        int16_T *pnValues03  = nValues;
        int16_T *pnValues47  = nValues + psQ8->m_nAdc03Reads;
        
        /*
        if ((ReadDWordFromHwMem(&pRegisters->status) & (STAT_ADC47FST | STAT_ADC03FST)) != (STAT_ADC47FST | STAT_ADC03FST))
            return FALSE;
        */

        /* Read the results from the FIFO */
        while (nAdc03Reads > 0 && nAdc47Reads > 0) /* channels left to read on both chips */
        {
            uint32_T nResults = ReadDWordFromHwMem(&pRegisters->analogInput.two); /* Read two channels simultaneously */
            *(pnValues03++) = (int16_T)(nResults);
            *(pnValues47++) = (int16_T)(nResults >> 16);
            --nAdc03Reads;
            --nAdc47Reads;
        }

        while (--nAdc03Reads >= 0) /* channels left on ADC03 */
            *(pnValues03++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc03);

        while (--nAdc47Reads >= 0) /* channels left on ADC47 */
            *(pnValues47++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc47);
    }

    return TRUE;
}

/*********************************************************************

  Name:         Q8_adcPreprogrammedInputFromExternalTrigger

  Purpose:      Read the selected A/D channels. The conversions have
                been trigger, but may not necessarily be available
				in the A/D FIFOs yet.

                This function assumes that the A/D chips have been
                preprogrammed for the selected channels using
                Q8_adcPreprogramUsingCounterEnable. This function
				is used by the Analog Input External Trigger block.

---------------------------------------------------------------------*/
INLINE boolean_T Q8_adcPreprogrammedInputFromExternalTrigger(PQ8 psQ8, uint8_T nChannels, int16_T *nValues)
{
    register PQ8Registers pRegisters = psQ8->m_pRegisters;

    if ((nChannels & 0xf0) == 0) /* only channels in the range 0-3 */
    {
        int_T nAdc03Reads = psQ8->m_nAdc03Reads;

        /* Wait until the conversions are complete */
        while ((ReadDWordFromHwMem(&pRegisters->status) & STAT_ADC03RDY) == 0);

        /* Read the conversion results */
        while (--nAdc03Reads >= 0)
            *(nValues++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc03);
    }
    else if ((nChannels & 0x0f) == 0) /* only channels in the range 4-7 */
    {
        int_T nAdc47Reads = psQ8->m_nAdc47Reads;

        /* Wait until the conversions are complete */
        while ((ReadDWordFromHwMem(&pRegisters->status) & STAT_ADC47RDY) == 0);

        /* Read the conversion results */
        while (--nAdc47Reads >= 0)
            *(nValues++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc47);
    }
    else /* channels in the range 0-7 */
    {
        const uint32_T nMask = STAT_ADC47RDY | STAT_ADC03RDY;
        int_T    nAdc03Reads = psQ8->m_nAdc03Reads;  /* number of reads to perform on ADC03 */
        int_T    nAdc47Reads = psQ8->m_nAdc47Reads;  /* number of reads to perform on ADC47 */
        int16_T *pnValues03  = nValues;
        int16_T *pnValues47  = nValues + psQ8->m_nAdc03Reads;
        
        /* Wait for all the conversions to complete */
        while ((ReadDWordFromHwMem(&pRegisters->status) & nMask) != nMask);

        /* Read the results from the FIFO */
        while (nAdc03Reads > 0 && nAdc47Reads > 0) /* channels left to read on both chips */
        {
            uint32_T nResults = ReadDWordFromHwMem(&pRegisters->analogInput.two); /* Read two channels simultaneously */
            *(pnValues03++) = (int16_T)(nResults);
            *(pnValues47++) = (int16_T)(nResults >> 16);
            --nAdc03Reads;
            --nAdc47Reads;
        }

        while (--nAdc03Reads >= 0) /* channels left on ADC03 */
            *(pnValues03++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc03);

        while (--nAdc47Reads >= 0) /* channels left on ADC47 */
            *(pnValues47++) = (int16_T)ReadWordFromHwMem(&pRegisters->analogInput.one.adc47);
    }

    return TRUE;
}

/*********************************************************************

  Name:         Q8_adcEnableInterrupt

  Purpose:      Enables interrupts from the counter.

---------------------------------------------------------------------*/
INLINE void Q8_adcEnableInterrupt(PQ8 psQ8)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;

    if (psQ8->m_nAdc47Reads > psQ8->m_nAdc03Reads)
        psQ8->m_nInterruptEnable |= INT_ADC47RDY;
    else
        psQ8->m_nInterruptEnable |= INT_ADC03RDY; /* simultaneous sampling must always be used when channels in 0-7 range read */

    /* Clear any existing interrupt condition for the A/D */
    WriteDWordToHwMem(&psQ8->m_pRegisters->interruptStatus, INT_ADC03RDY | INT_ADC47RDY);

    /* Enable interrupts from the A/D */
    WriteDWordToHwMem(&pRegisters->interruptEnable, psQ8->m_nInterruptEnable);
}

/*********************************************************************

  Name:         Q8_adcDisableInterrupt

  Purpose:      Disables interrupts from the counter.

---------------------------------------------------------------------*/
INLINE void Q8_adcDisableInterrupt(PQ8 psQ8)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;

    psQ8->m_nInterruptEnable &= ~(INT_ADC03RDY | INT_ADC47RDY);
    WriteDWordToHwMem(&pRegisters->interruptEnable, psQ8->m_nInterruptEnable);
}

/*********************************************************************

  Name:         Q8_adcClearInterrupt

  Purpose:      Clear the A/D condition. This function clears the
                ADC03RDY and ADC47RDY bits in the Interrupt Status Register.

---------------------------------------------------------------------*/
INLINE void Q8_adcClearInterrupt(Q8 *psQ8)
{
    WriteDWordToHwMem(&psQ8->m_pRegisters->interruptStatus, INT_ADC03RDY | INT_ADC47RDY);
}

/*********************************************************************

  Name:         Q8_adcCausedInterrupt

  Purpose:      Returns non-zero if the A/D caused the interrupt.

---------------------------------------------------------------------*/
INLINE boolean_T Q8_adcCausedInterrupt(Q8 *psQ8)
{
    uint32_T nStatus = ReadDWordFromHwMem(&psQ8->m_pRegisters->interruptStatus);
    if (psQ8->m_nAdc47Reads > psQ8->m_nAdc03Reads)
        return ((nStatus & INT_ADC47RDY) != 0);
    else
        return ((nStatus & INT_ADC03RDY) != 0);
}

#endif

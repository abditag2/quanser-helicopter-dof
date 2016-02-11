#if !defined(_Q8EN_H)
#define _Q8EN_H

#if !defined(_Q8_H)
#include "q8.h"
#endif

#define Q8_NUMBER_OF_ENC_CHANNELS   (8)
#define Q8_ENC_TIMEBASE             (60e-9)     /* 60ns filter clock frequency */

/********* Macros **********/

#define Q8_encIsNumChannelsParamOK(num)    \
    ((num) > 0 && (num) <= Q8_NUMBER_OF_ENC_CHANNELS)
#define Q8_encIsPrescaleParamOK(prescale) \
    ((prescale) >= 1 && (prescale) <= 256)
#define Q8_encIsModeParamOK(mode) \
    ((mode) >= 0 && (mode) <= 3)
#define Q8_encIsQuadratureParamOK(quadrature) \
    ((quadrature) >= 0 && (quadrature) <= 3)
 
/********* Common functions **********/

static const int_T nEncChannelMap[] = { 0, 4, 1, 5, 2, 6, 3, 7 };

INLINE void Q8_encEnableIndex(PQ8 psQ8, uint8_T nChannel)
{
    psQ8->m_nControl |= (1 << nChannel);
    WriteDWordToHwMem(&psQ8->m_pRegisters->control, psQ8->m_nControl);
}

INLINE void Q8_encDisableIndex(PQ8 psQ8, uint8_T nChannel)
{
    psQ8->m_nControl &= ~(1 << nChannel);
    WriteDWordToHwMem(&psQ8->m_pRegisters->control, psQ8->m_nControl);
}

INLINE void Q8_encEnableIndices(PQ8 psQ8, uint8_T nChannels)
{
    psQ8->m_nControl |= nChannels;
    WriteDWordToHwMem(&psQ8->m_pRegisters->control, psQ8->m_nControl);
}

INLINE void Q8_encDisableIndices(PQ8 psQ8, uint8_T nChannels)
{
    psQ8->m_nControl &= ~nChannels;
    WriteDWordToHwMem(&psQ8->m_pRegisters->control, psQ8->m_nControl);
}

INLINE void Q8_encInitialize(PQ8 psQ8, uint8_T nChannel, boolean_T bcd, uint8_T mode, uint8_T quadrature,
                             boolean_T enableIndex, boolean_T indexPolarity)
{
    register PQ8Registers pRegisters = psQ8->m_pRegisters;
    register int_T idx = nEncChannelMap[nChannel];
    uint8_T cmr, idr;

    /* Program the Counter Mode Register (CMR) [N.B. can corrupt encoder count. Reset count before using encoder] */
    cmr = ENC_ONE_CHANNEL | ENC_CMR_REGISTER | bcd | (mode << 1) | (quadrature << 3);
    WriteByteToHwMem(&pRegisters->encoderControl.encs[idx], cmr);

    /* Program the Index Control Register (IDR) */
    if ((quadrature > 0) && enableIndex) /* 1X, 2X or 4X quadrature */
        idr = ENC_ONE_CHANNEL | ENC_IDR_REGISTER | ENC_IDR_ENABLE_INDEX | ENC_IDR_LCNTR_INDEX;
    else
        idr = ENC_ONE_CHANNEL | ENC_IDR_REGISTER | ENC_IDR_DISABLE_INDEX | ENC_IDR_LCNTR_INDEX;

    idr |= indexPolarity ? ENC_IDR_POS_INDEX : ENC_IDR_NEG_INDEX;
    WriteByteToHwMem(&pRegisters->encoderControl.encs[idx], idr);

    if (enableIndex)
        Q8_encEnableIndex(psQ8, nChannel);
}

INLINE void Q8_encResetSingleCount(PQ8 psQ8, uint8_T nChannel)
{
    register int_T idx = nEncChannelMap[nChannel];

    /* Reset the counter to zero */
    WriteByteToHwMem(&psQ8->m_pRegisters->encoderControl.encs[idx],
        ENC_ONE_CHANNEL | ENC_RLD_REGISTER | ENC_RLD_RESET_CNTR);
}

INLINE void Q8_encResetCounts(PQ8 psQ8, uint8_T nChannels)
{
    const int rldReset = ENC_RLD_REGISTER | ENC_RLD_RESET_CNTR;
    register PQ8Registers pRegisters = psQ8->m_pRegisters;

    uint32_T nEvenRLD;
    uint32_T nOddRLD;
    int_T i;

    if ((nChannels & 0xaa) == 0) /* even-numbered channels only */
    {
        uint32_T resetOne = rldReset | ENC_ONE_CHANNEL;
        uint8_T  channels = nChannels;

        /* Form the commands used to reset the count values */
        nEvenRLD = 0;
        for (i=3; i >= 0; --i)
        {
            if (channels & 0x01) 
                nEvenRLD |= resetOne;

            resetOne  <<= 8;     /* move to next position in 32-bit encoder control word */
            channels  >>= 2;     /* move to next even channel */
        }

        /* Reset the counter values of selected channels */
        WriteDWordToHwMem(&pRegisters->encoderControl.four.enc0246, nEvenRLD);
    }
    else if ((nChannels & 0x55) == 0) /* odd-numbered channels only */
    {
        uint32_T resetOne = rldReset | ENC_ONE_CHANNEL;
        uint8_T  channels = nChannels;

        /* Form the commands used to reset the count values */
        nOddRLD  = 0;
        for (i=3; i >= 0; --i)
        {
            if (channels & 0x02)
                nOddRLD |= resetOne;

            resetOne  <<= 8;     /* move to next position in 32-bit encoder control word */
            channels  >>= 2;     /* move to next odd channels */
        }

        /* Reset the counter values of selected channels */
        WriteDWordToHwMem(&pRegisters->encoderControl.four.enc1357, nOddRLD);
    }
    else /* both even and odd numbered channels */
    {
        uint32_T resetBoth = rldReset | ENC_BOTH_CHANNELS;
        uint32_T resetOne  = rldReset | ENC_ONE_CHANNEL;

        uint8_T channels = nChannels;

        /* Form the commands used to reset the count values */
        nEvenRLD = 0;
        nOddRLD  = 0;
        for (i=3; i >= 0; --i)
        {
            if ((channels & 0x03) == 0x03) /* channels 2*i and 2*i+1 */
                nEvenRLD |= resetBoth;
            else if (channels & 0x01) 
                nEvenRLD |= resetOne;
            else if (channels & 0x02)
                nOddRLD |= resetOne;

            resetOne  <<= 8;     /* move to next position in 32-bit encoder control word */
            resetBoth <<= 8;     /* move to next position in 32-bit encoder control word */
            channels  >>= 2;     /* move to next pair of channels */
        }

        /* Reset the counter values of selected channels */
        WriteDWordToHwMem(&pRegisters->encoderControl.four.enc0246, nEvenRLD);
        if (nOddRLD)
            WriteDWordToHwMem(&pRegisters->encoderControl.four.enc1357, nOddRLD);
    }
}

INLINE void Q8_encConfigureIO(PQ8 psQ8, uint8_T nChannel, uint8_T nFlags, boolean_T bLoadCounterOnIndex)
{
    register int_T idx = nEncChannelMap[nChannel];
    WriteByteToHwMem(&psQ8->m_pRegisters->encoderControl.encs[idx],
        ENC_ONE_CHANNEL | ENC_IOR_REGISTER | ENC_IOR_ENABLE_AB | (nFlags << 3)
        | (bLoadCounterOnIndex ? ENC_IOR_LCNTR_LOAD : ENC_IOR_LCNTR_LATCH));
}

INLINE void Q8_encResetFlags(PQ8 psQ8, uint8_T nChannel)
{
    register int_T idx = nEncChannelMap[nChannel];
    WriteByteToHwMem(&psQ8->m_pRegisters->encoderControl.encs[idx],
        ENC_ONE_CHANNEL | ENC_RLD_REGISTER | ENC_RLD_RESET_FLAGS);
    WriteByteToHwMem(&psQ8->m_pRegisters->encoderControl.encs[idx],
        ENC_ONE_CHANNEL | ENC_RLD_REGISTER | ENC_RLD_RESET_E);
}

INLINE uint8_T Q8_encReadFlags(PQ8 psQ8, uint8_T nChannel)
{
    register int_T idx = nEncChannelMap[nChannel];
    return ReadByteFromHwMem(&psQ8->m_pRegisters->encoderControl.encs[idx]);
}

INLINE int32_T Q8_encReadSingleLatch(PQ8 psQ8, uint8_T nChannel)
{
    register PQ8Registers pRegisters = psQ8->m_pRegisters;
    register int_T idx = nEncChannelMap[nChannel];
    int8_T  nMSB;
    uint8_T nISB;
    uint8_T nLSB;
    int32_T nValue;

    /* Read the count value from the output latch (OL) */
    WriteByteToHwMem(&pRegisters->encoderControl.encs[idx], 
        ENC_ONE_CHANNEL | ENC_RLD_REGISTER | ENC_RLD_RESET_BP);             /* reset byte pointer (BP) */
    nLSB = ReadByteFromHwMem(&pRegisters->encoderData.encs[idx]);           /* read LSB */
    nISB = ReadByteFromHwMem(&pRegisters->encoderData.encs[idx]);           /* read ISB */
    nMSB = ReadByteFromHwMem(&pRegisters->encoderData.encs[idx]);           /* read MSB */

    nValue = ((nMSB << 16) | (nISB << 8) | nLSB);
    return -nValue;
}

INLINE int32_T Q8_encInputSingle(PQ8 psQ8, uint8_T nChannel)
{
    register PQ8Registers pRegisters = psQ8->m_pRegisters;
    register int_T idx = nEncChannelMap[nChannel];
    int8_T  nMSB;
    uint8_T nISB;
    uint8_T nLSB;
    int32_T nValue;

    /* Transfer the counter value (CNTR) to the output latch (OL) and reset the byte pointer (BP) */
    WriteByteToHwMem(&pRegisters->encoderControl.encs[idx], 
        ENC_ONE_CHANNEL | ENC_RLD_REGISTER | ENC_RLD_GET_CNTR | ENC_RLD_RESET_BP);

    /* Read the count value from the output latch (OL) */
    nLSB = ReadByteFromHwMem(&pRegisters->encoderData.encs[idx]);           /* read LSB */
    nISB = ReadByteFromHwMem(&pRegisters->encoderData.encs[idx]);           /* read ISB */
    nMSB = ReadByteFromHwMem(&pRegisters->encoderData.encs[idx]);           /* read MSB */

    nValue = ((nMSB << 16) | (nISB << 8) | nLSB);
    return -nValue;
}

INLINE void Q8_encInput(PQ8 psQ8, uint8_T nChannels, int32_T *pnValues)
{
    const int rldLatch = ENC_RLD_REGISTER | ENC_RLD_GET_CNTR | ENC_RLD_RESET_BP;

    PQ8Registers pRegisters = psQ8->m_pRegisters;
    uint32_T nEvenRLD;
    uint32_T nOddRLD;
    uint32_T nEvenLSBs;
    uint32_T nEvenISBs;
    uint32_T nEvenMSBs;
    uint32_T nOddLSBs;
    uint32_T nOddISBs;
    uint32_T nOddMSBs;
    int_T i;
    register int32_T nValue;

    /* 
        Construct command to transfer count value to output latch. Only latch selected channels since other channels
        may be latching on the index pulse or some other operation.
    */
    if ((nChannels & 0xaa) == 0) /* even-numbered channels only */
    {
        uint32_T latchOne = rldLatch | ENC_ONE_CHANNEL;
        uint8_T  channels = nChannels;

        /* Form the commands used to latch the count values */
        nEvenRLD = 0;
        for (i=3; i >= 0; --i)
        {
            if (channels & 0x01) 
                nEvenRLD |= latchOne;

            latchOne  <<= 8;     /* move to next position in 32-bit encoder control word */
            channels  >>= 2;     /* move to next even channel */
        }

        /* Latch the counter values of selected channels */
        WriteDWordToHwMem(&pRegisters->encoderControl.four.enc0246, nEvenRLD);

        /* Read the count values of the selected channels */
        nEvenLSBs = ReadDWordFromHwMem(&pRegisters->encoderData.four.enc0246);
        nEvenISBs = ReadDWordFromHwMem(&pRegisters->encoderData.four.enc0246);
        nEvenMSBs = ReadDWordFromHwMem(&pRegisters->encoderData.four.enc0246);

        /* Extract the desired count values */
        if (nChannels & 0x01) /* channel 0 */
        {
            nValue = ((int8_T)(nEvenMSBs & 0xff) << 16) | ((nEvenISBs & 0xff) << 8) | (nEvenLSBs & 0xff);
            *(pnValues++) = -nValue;
        }
        if (nChannels & 0x04) /* channel 2 */
        {
            nValue = ((int16_T)(nEvenMSBs & 0xff00) << 8) | (nEvenISBs & 0xff00) | ((nEvenLSBs & 0xff00) >> 8);
            *(pnValues++) = -nValue;
        }
        if (nChannels & 0x10) /* channel 4 */
        {
            nValue = (nEvenMSBs & 0xff0000) | ((nEvenISBs & 0xff0000) >> 8) | ((nEvenLSBs & 0xff0000) >> 16);
            if (nEvenMSBs & 0x800000)
                nValue |= 0xff000000;
            *(pnValues++) = -nValue;
        }
        if (nChannels & 0x40) /* channel 6 */
        {
            nValue = ((int32_T)(nEvenMSBs & 0xff000000) >> 8) | ((nEvenISBs & 0xff000000) >> 16) | ((nEvenLSBs & 0xff000000) >> 24);
            *(pnValues++) = -nValue;
        }
    }
    else if ((nChannels & 0x55) == 0) /* odd-numbered channels only */
    {
        uint32_T latchOne = rldLatch | ENC_ONE_CHANNEL;
        uint8_T  channels = nChannels;

        /* Form the commands used to latch the count values */
        nOddRLD  = 0;
        for (i=3; i >= 0; --i)
        {
            if (channels & 0x02)
                nOddRLD |= latchOne;

            latchOne  <<= 8;     /* move to next position in 32-bit encoder control word */
            channels  >>= 2;     /* move to next odd channels */
        }

        /* Latch the counter values of selected channels */
        WriteDWordToHwMem(&pRegisters->encoderControl.four.enc1357, nOddRLD);

        /* Read the count values of the selected channels */
        nOddLSBs = ReadDWordFromHwMem(&pRegisters->encoderData.four.enc1357);
        nOddISBs = ReadDWordFromHwMem(&pRegisters->encoderData.four.enc1357);
        nOddMSBs = ReadDWordFromHwMem(&pRegisters->encoderData.four.enc1357);

        /* Extract the desired count values */
        if (nChannels & 0x02) /* channel 1 */
        {
            nValue = ((int8_T)(nOddMSBs & 0xff) << 16)  | ((nOddISBs & 0xff) << 8)  | (nOddLSBs & 0xff);
            *(pnValues++) = -nValue;
        }
        if (nChannels & 0x08) /* channel 3 */
        {
            nValue = ((int16_T)(nOddMSBs & 0xff00) << 8)  | (nOddISBs & 0xff00)  | ((nOddLSBs & 0xff00) >> 8);
            *(pnValues++) = -nValue;
        }
        if (nChannels & 0x20) /* channel 5 */
        {
            nValue = (nOddMSBs & 0xff0000) | ((nOddISBs & 0xff0000) >> 8) | ((nOddLSBs & 0xff0000) >> 16);
            if (nOddMSBs & 0x800000)
                nValue |= 0xff000000;
            *(pnValues++) = -nValue;
        }
        if (nChannels & 0x80) /* channel 7 */
        {
            nValue = ((int32_T)(nOddMSBs & 0xff000000) >> 8) | ((nOddISBs & 0xff000000) >> 16) | ((nOddLSBs & 0xff000000) >> 24);
            *(pnValues++) = -nValue;
        }
    }
    else /* both even and odd numbered channels */
    {
        uint32_T latchBoth = rldLatch | ENC_BOTH_CHANNELS;
        uint32_T latchOne  = rldLatch | ENC_ONE_CHANNEL;

        uint8_T channels = nChannels;

        /* Form the commands used to latch the count values */
        nEvenRLD = 0;
        nOddRLD  = 0;
        for (i=3; i >= 0; --i)
        {
            if ((channels & 0x03) == 0x03) /* channels 2*i and 2*i+1 */
                nEvenRLD |= latchBoth;
            else if (channels & 0x01) 
                nEvenRLD |= latchOne;
            else if (channels & 0x02)
                nOddRLD |= latchOne;

            latchOne  <<= 8;     /* move to next position in 32-bit encoder control word */
            latchBoth <<= 8;     /* move to next position in 32-bit encoder control word */
            channels  >>= 2;     /* move to next pair of channels */
        }

        /* Latch the counter values of selected channels */
        WriteDWordToHwMem(&pRegisters->encoderControl.four.enc0246, nEvenRLD);
        if (nOddRLD)
            WriteDWordToHwMem(&pRegisters->encoderControl.four.enc1357, nOddRLD);

        /* Read the count values of the selected channels */
        nEvenLSBs = ReadDWordFromHwMem(&pRegisters->encoderData.four.enc0246);
        nOddLSBs  = ReadDWordFromHwMem(&pRegisters->encoderData.four.enc1357);

        nEvenISBs = ReadDWordFromHwMem(&pRegisters->encoderData.four.enc0246);
        nOddISBs  = ReadDWordFromHwMem(&pRegisters->encoderData.four.enc1357);
        
        nEvenMSBs = ReadDWordFromHwMem(&pRegisters->encoderData.four.enc0246);
        nOddMSBs  = ReadDWordFromHwMem(&pRegisters->encoderData.four.enc1357);

        /* Extract the desired count values */
        if (nChannels & 0x01) /* channel 0 */
        {
            nValue = ((int8_T)(nEvenMSBs & 0xff) << 16) | ((nEvenISBs & 0xff) << 8) | (nEvenLSBs & 0xff);
            *(pnValues++) = -nValue;
        }
        if (nChannels & 0x02) /* channel 1 */
        {
            nValue = ((int8_T)(nOddMSBs & 0xff) << 16)  | ((nOddISBs & 0xff) << 8)  | (nOddLSBs & 0xff);
            *(pnValues++)  = -nValue;
        }
        if (nChannels & 0x04) /* channel 2 */
        {
            nValue = ((int16_T)(nEvenMSBs & 0xff00) << 8) | (nEvenISBs & 0xff00) | ((nEvenLSBs & 0xff00) >> 8);
            *(pnValues++) = -nValue;
        }
        if (nChannels & 0x08) /* channel 3 */
        {
            nValue = ((int16_T)(nOddMSBs & 0xff00) << 8)  | (nOddISBs & 0xff00)  | ((nOddLSBs & 0xff00) >> 8);
            *(pnValues++)= -nValue;
        }
        if (nChannels & 0x10) /* channel 4 */
        {
            nValue = (nEvenMSBs & 0xff0000) | ((nEvenISBs & 0xff0000) >> 8) | ((nEvenLSBs & 0xff0000) >> 16);
            if (nEvenMSBs & 0x800000)
                nValue |= 0xff000000;
            *(pnValues++)= -nValue;
        }
        if (nChannels & 0x20) /* channel 5 */
        {
            nValue = (nOddMSBs & 0xff0000) | ((nOddISBs & 0xff0000) >> 8) | ((nOddLSBs & 0xff0000) >> 16);
            if (nOddMSBs & 0x800000)
                nValue |= 0xff000000;
            *(pnValues++)= -nValue;
        }
        if (nChannels & 0x40) /* channel 6 */
        {
            nValue = ((int32_T)(nEvenMSBs & 0xff000000) >> 8) | ((nEvenISBs & 0xff000000) >> 16) | ((nEvenLSBs & 0xff000000) >> 24);
            *(pnValues++) = -nValue;
        }
        if (nChannels & 0x80) /* channel 7 */
        {
            nValue = ((int32_T)(nOddMSBs & 0xff000000) >> 8) | ((nOddISBs & 0xff000000) >> 16) | ((nOddLSBs & 0xff000000) >> 24);
            *(pnValues++) = -nValue;
        }
    }
}

/* @todo: Make operation far more efficient by using 32-bit accesses and multiple channels */

INLINE void Q8_encSetCount(PQ8 psQ8, uint8_T nChannel, uint32_T nValue)
{
    register PQ8Registers pRegisters = psQ8->m_pRegisters;
    register int_T idx = nEncChannelMap[nChannel];

    nValue = -(int32_T)nValue;

    /* Write the count value to the encoder preload register (PR) */
    WriteByteToHwMem(&pRegisters->encoderControl.encs[idx], 
        ENC_ONE_CHANNEL | ENC_RLD_REGISTER | ENC_RLD_RESET_BP);                    /* reset byte pointer (BP) */
    WriteByteToHwMem(&pRegisters->encoderData.encs[idx], nValue & 0x0ff);          /* write LSB */
    WriteByteToHwMem(&pRegisters->encoderData.encs[idx], (nValue >> 8)  & 0x0ff);
    WriteByteToHwMem(&pRegisters->encoderData.encs[idx], (nValue >> 16) & 0x0ff);  /* write MSB */

    /* Transfer the preload register (PR) to the counter (CNTR) */
    WriteByteToHwMem(&pRegisters->encoderControl.encs[idx], 
        ENC_ONE_CHANNEL | ENC_RLD_REGISTER | ENC_RLD_SET_CNTR);
}

INLINE void Q8_encSetFilterPrescaler(PQ8 psQ8, uint8_T nChannel, uint8_T nValue)
{
    register PQ8Registers pRegisters = psQ8->m_pRegisters;
    register int_T idx = nEncChannelMap[nChannel];

    /* Write the count value to the encoder preload register (PR) */
    WriteByteToHwMem(&pRegisters->encoderControl.encs[idx], 
        ENC_ONE_CHANNEL | ENC_RLD_REGISTER | ENC_RLD_RESET_BP);            /* reset byte pointer (BP) */
    WriteByteToHwMem(&pRegisters->encoderData.encs[idx], nValue);          /* write prescaler value (0-255) */

    /* Transfer the preload register (PR) to the filter clock prescaler (PSC) */
    WriteByteToHwMem(&pRegisters->encoderControl.encs[idx], 
        ENC_ONE_CHANNEL | ENC_RLD_REGISTER | ENC_RLD_SET_PSC);
}

INLINE void Q8_encRestore(PQ8 psQ8, uint8_T nChannel)
{
    Q8_encSetCount(psQ8, nChannel, -psQ8->m_nEncoderCounts[nChannel]);
}

#endif

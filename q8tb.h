#if !defined(_Q8TB_H)
#define _Q8TB_H

#include "DataTypes.h"
#include "Hardware.h"

#if !defined(_Q8_H)
#include "q8.h"
#endif

/********* Macros **********/

#define Q8_TB_NUM_CLOCKS        (2)                               /* Two general purpose counters available as a timebase */
#define Q8_COUNTER_PWM_TIMEBASE (30e-9)                           /* 30ns base period (PWM mode) */
#define Q8_COUNTER_TIMEBASE     (2*Q8_COUNTER_PWM_TIMEBASE)
#define Q8_MIN_COUNTER_TIMEOUT  (2*Q8_COUNTER_PWM_TIMEBASE)       /* Min. period for internal use of counter is 60ns */
#define Q8_MIN_OUTPUT_TIMEOUT   (2*Q8_COUNTER_PWM_TIMEBASE)       /* Min. period for external use of CNTR_OUT is 60ns */
#define Q8_MAX_COUNTER_TIMEOUT  (0xffffffffU*Q8_COUNTER_TIMEBASE) /* Max. period for counter */

#define Q8_tbIsClockSourceParamOK(clock)  \
    ((clock) > 0 && (clock) <= Q8_TB_NUM_CLOCKS)

#define Q8_tbIsCounterPeriodParamOK(period)  \
            ((period) >= Q8_MIN_COUNTER_TIMEOUT && (period) <= Q8_MAX_COUNTER_TIMEOUT)

#define Q8_tbIsOutputPeriodParamOK(period)  \
            ((period) >= Q8_MIN_OUTPUT_TIMEOUT && (period) <= Q8_MAX_COUNTER_TIMEOUT)

#define Q8_tbIsDutyParamOK(duty) \
            ((duty) >= 0 && (duty) <= 100)

/*********************************************************************

  Name:         Q8_cntrTimedOut

  Purpose:      Returns non-zero if the timer expired.

---------------------------------------------------------------------*/
INLINE boolean_T Q8_cntrTimedOut(PQ8 psQ8)
{
	return ((ReadDWordFromHwMem(&psQ8->m_pRegisters->interruptStatus) & INT_CNTROUT) != 0);
}

/*********************************************************************

  Name:         Q8_cntrSet

  Purpose:      Set the counter value.

---------------------------------------------------------------------*/
INLINE void Q8_cntrSet(PQ8 psQ8, uint32_T nCount)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;

    /* Set the counter preload value (which will determine the period) */
    WriteDWordToHwMem(&pRegisters->counter.sq.preload, nCount);

    /* Force the counter to load the preload value immediately and set the output high. */
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_CNTRLD | CCTRL_CNTRVAL);
}
	
/*********************************************************************

  Name:         Q8_cntrPreinitialize

  Purpose:      Initialize the timer. Set the period.

---------------------------------------------------------------------*/
INLINE void Q8_cntrPreinitializeCount(PQ8 psQ8, uint32_T nCount, boolean_T bEnableOutput)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;

    /* Set the counter preload value (which will determine the period) */
    WriteDWordToHwMem(&pRegisters->counter.sq.preload, nCount);

    /* Force the counter to load the preload value immediately and set the output high. */
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_CNTRLD | CCTRL_CNTRVAL);

    /* Clear the counter output bit in the Interrupt Status Register */
    WriteDWordToHwMem(&pRegisters->interruptStatus, INT_CNTROUT);

    /* Enable the counter output if necessary */
    if (bEnableOutput)
	{
        psQ8->m_nCounterControl |= CCTRL_CNTROUTEN;
		WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
	}
}

/*********************************************************************

  Name:         Q8_cntrInitialize

  Purpose:      Initialize the timer. Set the period.

---------------------------------------------------------------------*/
INLINE void Q8_cntrInitialize(PQ8 psQ8, real_T nPeriod, boolean_T bEnableOutput)
{
    uint32_T     period     = (uint32_T)(nPeriod / Q8_COUNTER_TIMEBASE) - 1;
	PQ8Registers pRegisters = psQ8->m_pRegisters;

    /* Set the counter preload value (which will determine the period) */
    WriteDWordToHwMem(&pRegisters->counter.sq.preload, period);

    /* Force the counter to load the preload value immediately and set the output high. */
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_CNTRLD | CCTRL_CNTRVAL);

    /* Clear the counter output bit in the Interrupt Status Register */
    WriteDWordToHwMem(&pRegisters->interruptStatus, INT_CNTROUT);

    /* Enable the counter and the output if necessary */
    psQ8->m_nCounterControl |= CCTRL_CNTREN;
    if (bEnableOutput)
        psQ8->m_nCounterControl |= CCTRL_CNTROUTEN;
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
}
/*********************************************************************

  Name:         Q8_cntrInitializePWM

  Purpose:      Initialize the counter as a PWM counter.

---------------------------------------------------------------------*/
INLINE void Q8_cntrInitializePWM(PQ8 psQ8, real_T nPeriod, real_T nDuty)
{
    int32_T      period     = (int32_T)(nPeriod / Q8_COUNTER_PWM_TIMEBASE);
    int32_T      duty       = (int32_T)(nDuty * nPeriod / Q8_COUNTER_PWM_TIMEBASE); /* high time */
	PQ8Registers pRegisters = psQ8->m_pRegisters;

    /* Set the counter preload registers. Use double-buffering to avoid glitches */
    psQ8->m_nCounterControl &= ~(CCTRL_CNTRWSET | CCTRL_CNTRRSET);
    psQ8->m_nCounterControl |= CCTRL_CNTRWSET;
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);

    psQ8->m_nCounterControl |= (CCTRL_CNTROUTEN | CCTRL_CNTRMODE);   /* enable PWM mode and output */
    psQ8->m_nCounterControl ^= (CCTRL_CNTRWSET  | CCTRL_CNTRRSET);   /* switch register sets */

    if (duty <= 0) /* counter always low */
    {
        WriteDWordToHwMem(&pRegisters->counter.pwm.preloadLow,  period - 1);
        WriteDWordToHwMem(&pRegisters->counter.pwm.preloadHigh, 0);

        /* Enable the counter output but not the counter. Set the output to zero */
        WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
        WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_CNTRLD);
    }
    else if (duty >= period) /* counter always high */
    {
        WriteDWordToHwMem(&pRegisters->counter.pwm.preloadLow,  0);
        WriteDWordToHwMem(&pRegisters->counter.pwm.preloadHigh, period - 1);

        /* Enable the counter output but not the counter. Set the output to one */
        WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
        WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_CNTRLD | CCTRL_CNTRVAL);
    }
    else /* counter oscillates */
    {
        /* Program the low and high pulse times */
        WriteDWordToHwMem(&pRegisters->counter.pwm.preloadLow,  period - duty - 1);
        WriteDWordToHwMem(&pRegisters->counter.pwm.preloadHigh, duty - 1);

        /* Enable the counter output and the counter. Start with the output high */
        psQ8->m_nCounterControl |= CCTRL_CNTREN;
        WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
        WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_CNTRLD | CCTRL_CNTRVAL);
    }
}

/*********************************************************************

  Name:         Q8_cntrSetPWM

  Purpose:      Set the duty cycle for pulse-width modulation. This
                function can be called while the model is running. PWM
                mode must have been initialized using Q8_cntrInitializePWM.
                The period should not be changed.

---------------------------------------------------------------------*/
INLINE void Q8_cntrSetPWM(PQ8 psQ8, real_T nPeriod, real_T nDuty)
{
    int32_T      period     = (int32_T)(nPeriod / Q8_COUNTER_PWM_TIMEBASE);
    int32_T      duty       = (int32_T)(nDuty * nPeriod / Q8_COUNTER_PWM_TIMEBASE); /* high time */
    PQ8Registers pRegisters = psQ8->m_pRegisters;

	/* Switch register sets so that changes are double-buffered */
    psQ8->m_nCounterControl ^= (CCTRL_CNTRWSET | CCTRL_CNTRRSET);

    if (duty <= 0) /* counter always low */
    {
        /* Disable the counter. Set the output to zero */
        psQ8->m_nCounterControl &= ~CCTRL_CNTREN;
        WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
        WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_CNTRLD);
    }
    else if (duty >= period) /* counter always high */
    {
        /* Disable the counter. Set the output to one */
        psQ8->m_nCounterControl &= ~CCTRL_CNTREN;
        WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
        WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_CNTRLD | CCTRL_CNTRVAL);
    }
    else /* counter oscillates */
    {
        /* Program the low and high pulse times */
        WriteDWordToHwMem(&pRegisters->counter.pwm.preloadLow,  period - duty - 1);
        WriteDWordToHwMem(&pRegisters->counter.pwm.preloadHigh, duty - 1);

        /* Enable the counter. Allow the output to change after one period */
        if (!(psQ8->m_nCounterControl & CCTRL_CNTREN)) /* output was constant */
        {
            psQ8->m_nCounterControl |= CCTRL_CNTREN;
            WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
            WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_CNTRLD | CCTRL_CNTRVAL);
        }
        else
            WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
    }
}

/*********************************************************************

  Name:         Q8_cntrUninitialize

  Purpose:      Uninitialize the timer.

---------------------------------------------------------------------*/
INLINE void Q8_cntrUninitialize(PQ8 psQ8)
{
    /* Disable the counter */
    psQ8->m_nCounterControl &= ~(CCTRL_CNTREN | CCTRL_CNTROUTEN | CCTRL_CNTRMODE | CCTRL_CNTRRSET | CCTRL_CNTRWSET);
    WriteDWordToHwMem(&psQ8->m_pRegisters->counterControl, psQ8->m_nCounterControl);
}

/*********************************************************************

  Name:         Q8_cntrReload

  Purpose:      Reload the timer by reloading the preload
                value into the counter.

---------------------------------------------------------------------*/
INLINE void Q8_cntrReload(Q8 *psQ8)
{
    /* Force the counter to load the preload value immediately and set the output high. */
    WriteDWordToHwMem(&psQ8->m_pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_CNTRLD | CCTRL_CNTRVAL);
}

/*********************************************************************

  Name:         Q8_cntrEnableInterrupt

  Purpose:      Enables interrupts from the counter.

---------------------------------------------------------------------*/
INLINE void Q8_cntrEnableInterrupt(PQ8 psQ8)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;

    /* Clear any existing interrupt condition for the counter */
    WriteDWordToHwMem(&pRegisters->interruptStatus, INT_CNTROUT);

    /* Enable interrupts from the counter */
    psQ8->m_nInterruptEnable |= INT_CNTROUT;
    WriteDWordToHwMem(&pRegisters->interruptEnable, psQ8->m_nInterruptEnable);
}

/*********************************************************************

  Name:         Q8_cntrDisableInterrupt

  Purpose:      Disables interrupts from the counter.

---------------------------------------------------------------------*/
INLINE void Q8_cntrDisableInterrupt(PQ8 psQ8)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;

    /* Disable interrupts from the counter */
    psQ8->m_nInterruptEnable &= ~INT_CNTROUT;
    WriteDWordToHwMem(&pRegisters->interruptEnable, psQ8->m_nInterruptEnable);
}

/*********************************************************************

  Name:         Q8_cntrEnableOutput

  Purpose:      Enables the output from the counter, _CNTR_OUT.

---------------------------------------------------------------------*/
INLINE void Q8_cntrEnableOutput(PQ8 psQ8)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;

    /* Enable the counter output */
    psQ8->m_nCounterControl |= CCTRL_CNTROUTEN;
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
}

/*********************************************************************

  Name:         Q8_cntrDisableOutput

  Purpose:      Disables the output from the counter, _CNTR_OUT.

---------------------------------------------------------------------*/
INLINE void Q8_cntrDisableOutput(PQ8 psQ8)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;

    /* Disable the counter output */
    psQ8->m_nCounterControl &= ~CCTRL_CNTROUTEN;
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
}

/*********************************************************************

  Name:         Q8_cntrEnable

  Purpose:      Enables the counter.

---------------------------------------------------------------------*/
INLINE void Q8_cntrEnable(PQ8 psQ8)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;

    /* Enable the counter and the output if necessary */
    psQ8->m_nCounterControl |= CCTRL_CNTREN;
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
}

/*********************************************************************

  Name:         Q8_cntrDisable

  Purpose:      Enables the counter.

---------------------------------------------------------------------*/
INLINE void Q8_cntrDisable(PQ8 psQ8)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;

    /* Enable the counter and the output if necessary */
    psQ8->m_nCounterControl &= ~CCTRL_CNTREN;
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
}

/*********************************************************************

  Name:         Q8_cntrClearInterrupt

  Purpose:      Clear the counter expiration condition. This function clears the
                CNTR_OUT bit in the Interrupt Status Register.

---------------------------------------------------------------------*/
INLINE void Q8_cntrClearInterrupt(Q8 *psQ8)
{
    /* Clear the counter output bit in the Interrupt Status Register */
    WriteDWordToHwMem(&psQ8->m_pRegisters->interruptStatus, INT_CNTROUT);
}


/*********************************************************************

  Name:         Q8_cntrRead

  Purpose:      Read the timer.

---------------------------------------------------------------------*/
INLINE uint32_T Q8_cntrRead(PQ8 psQ8)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;
    return ReadDWordFromHwMem(&pRegisters->counter.sq.value);
}

#endif


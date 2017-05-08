#if !defined(_Q8WD_H)
#define _Q8WD_H

#include "DataTypes.h"
#include "Hardware.h"

#if !defined(_Q8_H)
#include "q8.h"
#endif

/********* Macros **********/

#define Q8_WATCHDOG_PWM_TIMEBASE    (30e-9)     /* 30ns base period (PWM mode) */
#define Q8_WATCHDOG_TIMEBASE        (2*Q8_WATCHDOG_PWM_TIMEBASE)
#define Q8_MIN_WATCHDOG_TIMEOUT     (2*Q8_WATCHDOG_PWM_TIMEBASE)
#define Q8_MAX_WATCHDOG_TIMEOUT     (0xffffffffU*Q8_WATCHDOG_PWM_TIMEBASE)

#define Q8_tbIsWatchdogPeriodParamOK(period)  \
            ((period) >= Q8_MIN_WATCHDOG_TIMEOUT && (period) <= Q8_MAX_WATCHDOG_TIMEOUT)

/*********************************************************************

  Name:         Q8_wdogTimedOut

  Purpose:      Returns non-zero if the watchdog timer expired.

---------------------------------------------------------------------*/
INLINE boolean_T Q8_wdogTimedOut(PQ8 psQ8)
{
	return ((ReadDWordFromHwMem(&psQ8->m_pRegisters->interruptStatus) & INT_WATCHDOG) != 0);
}

/*********************************************************************

  Name:         Q8_wdogSet

  Purpose:      Set the counter value.

---------------------------------------------------------------------*/
INLINE void Q8_wdogSet(PQ8 psQ8, uint32_T nCount)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;

    /* Set the counter preload value (which will determine the period) */
    WriteDWordToHwMem(&pRegisters->watchdog.sq.preload, nCount);

    /* Force the counter to load the preload value immediately and set the output high. */
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_WDOGLD | CCTRL_WDOGVAL);
}
	
/*********************************************************************

  Name:         Q8_wdogPreinitializeCount

  Purpose:      Initialize the watchdog timer. Set the period and load it.
                Do not activate the watchdog timer features.

---------------------------------------------------------------------*/
INLINE void Q8_wdogPreinitializeCount(PQ8 psQ8, uint32_T nCount, boolean_T bEnableOutput)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;

    /* Set the watchdog counter preload value (which will determine the period) */
    WriteDWordToHwMem(&pRegisters->watchdog.sq.preload, nCount);

    /* Force the watchdog counter to load the preload value immediately and set the output high. */
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_WDOGLD | CCTRL_WDOGVAL);

    /* Clear the watchdog bit in the Interrupt Status Register */
    WriteDWordToHwMem(&pRegisters->interruptStatus, INT_WATCHDOG);

    if (bEnableOutput)
	{
        psQ8->m_nCounterControl |= CCTRL_WDOGOUTEN;
		WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
	}
}

/*********************************************************************

  Name:         Q8_wdogInitialize

  Purpose:      Initialize the watchdog timer. Set the period and enable it.
                Do not activate the watchdog timer features.

---------------------------------------------------------------------*/
INLINE void Q8_wdogInitialize(PQ8 psQ8, real_T nPeriod, boolean_T bEnableOutput)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;
    uint32_T     period = (uint32_T)(nPeriod / Q8_WATCHDOG_TIMEBASE) - 1;

	/* Set the watchdog counter preload value (which will determine the period) */
    WriteDWordToHwMem(&pRegisters->watchdog.sq.preload, period);

    /* Force the watchdog counter to load the preload value immediately and set the output high. */
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_WDOGLD | CCTRL_WDOGVAL);

    /* Clear the watchdog bit in the Interrupt Status Register */
    WriteDWordToHwMem(&pRegisters->interruptStatus, INT_WATCHDOG);

    psQ8->m_nCounterControl |= CCTRL_WDOGEN;
    if (bEnableOutput)
        psQ8->m_nCounterControl |= CCTRL_WDOGOUTEN;
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
}

/*********************************************************************

  Name:         Q8_wdogInitializeActive

  Purpose:      Initialize the watchdog timer. Set the period, enable it
                and activate the watchdog timer features.

  See Also:     Q8_wdogReload

---------------------------------------------------------------------*/
INLINE void Q8_wdogInitializeActive(PQ8 psQ8, real_T nPeriod, boolean_T bEnableOutput)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;
    uint32_T     period = (uint32_T)(nPeriod / Q8_WATCHDOG_PWM_TIMEBASE) - 1;

    /* Set the watchdog counter preload value (which will determine the period) */
    WriteDWordToHwMem(&pRegisters->watchdog.sq.preload, period);

    /* Force the watchdog counter to load the preload value immediately and set the output high. */
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_WDOGLD | CCTRL_WDOGVAL);

    /* Clear the watchdog bit in the Interrupt Status Register */
    WriteDWordToHwMem(&pRegisters->interruptStatus, INT_WATCHDOG);

    /*
        Enable the watchdog output, if requested, and set the enable flag. However do not enable the
        watchdog counter yet because this function is called in MdlStart before the controller runs.
        Hence, we don't want it expiring while we are doing lengthy initializations so that we cannot
        even run the controller. Because we have set the enable flag, it will get enabled when the
        watchdog is reloaded the first time in the control loop.
    */
    if (bEnableOutput)
        psQ8->m_nCounterControl |= CCTRL_WDOGOUTEN;
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl & ~(CCTRL_WDOGEN | CCTRL_WDOGACT));
    psQ8->m_nCounterControl |= CCTRL_WDOGEN | CCTRL_WDOGACT;
}

/*********************************************************************

  Name:         Q8_wdogReload

  Purpose:      Reload the watchdog timer by reloading the preload
                value into the counter. This function should be called
                each sampling instant to prevent the watchdog timer
                from expiring.

---------------------------------------------------------------------*/
INLINE void Q8_wdogReload(Q8 *psQ8)
{
    /* Force the watchdog counter to load the preload value immediately and set the output high. */
    WriteDWordToHwMem(&psQ8->m_pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_WDOGLD | CCTRL_WDOGVAL);
}

/*********************************************************************

  Name:         Q8_wdogInitializePWM

  Purpose:      Initialize the counter as a PWM counter.

---------------------------------------------------------------------*/
INLINE void Q8_wdogInitializePWM(PQ8 psQ8, real_T nPeriod, real_T nDuty)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;
    int32_T      period     = (int32_T)(nPeriod / Q8_WATCHDOG_PWM_TIMEBASE);
    int32_T      duty       = (int32_T)(nDuty * nPeriod / Q8_WATCHDOG_PWM_TIMEBASE); /* high time */

    /* Set the counter preload registers. Use double-buffering to avoid glitches. Deactivate watchdog features. */
    psQ8->m_nCounterControl &= ~(CCTRL_WDOGWSET | CCTRL_WDOGRSET | CCTRL_WDOGACT);
    psQ8->m_nCounterControl |= CCTRL_WDOGWSET;
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);

    psQ8->m_nCounterControl |= (CCTRL_WDOGOUTEN | CCTRL_WDOGMODE | CCTRL_WDOGSEL);  /* enable PWM mode and output */
    psQ8->m_nCounterControl ^= (CCTRL_WDOGWSET  | CCTRL_WDOGRSET);                  /* switch register sets */

    if (duty <= 0) /* counter always low */
    {
        WriteDWordToHwMem(&pRegisters->watchdog.pwm.preloadLow,  period - 1);
        WriteDWordToHwMem(&pRegisters->watchdog.pwm.preloadHigh, 0);

        /* Enable the counter output but not the counter. Set the output to zero */
        WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
        WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_WDOGLD);
    }
    else if (duty >= period) /* counter always high */
    {
        WriteDWordToHwMem(&pRegisters->watchdog.pwm.preloadLow,  0);
        WriteDWordToHwMem(&pRegisters->watchdog.pwm.preloadHigh, period - 1);

        /* Enable the counter output but not the counter. Set the output to one */
        psQ8->m_nCounterControl |= CCTRL_WDOGOUTEN;
        WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
        WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_WDOGLD | CCTRL_WDOGVAL);
    }
    else /* counter oscillates */
    {
        /* Program the low and high pulse times */
        WriteDWordToHwMem(&pRegisters->watchdog.pwm.preloadLow,  period - duty - 1);
        WriteDWordToHwMem(&pRegisters->watchdog.pwm.preloadHigh, duty - 1);

        /* Enable the counter output and the counter. Start with the output high */
        psQ8->m_nCounterControl |= CCTRL_WDOGEN;
        WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
        WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_WDOGLD | CCTRL_WDOGVAL);
    }
}

/*********************************************************************

  Name:         Q8_wdogSetPWM

  Purpose:      Set the duty cycle for pulse-width modulation. This
                function can be called while the model is running. PWM
                mode must have been initialized using Q8_wdogInitializePWM.
                The period should not be changed.

---------------------------------------------------------------------*/
INLINE void Q8_wdogSetPWM(PQ8 psQ8, real_T nPeriod, real_T nDuty)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;
    int32_T      period     = (int32_T)(nPeriod / Q8_WATCHDOG_PWM_TIMEBASE);
    int32_T      duty       = (int32_T)(nDuty * nPeriod / Q8_WATCHDOG_PWM_TIMEBASE); /* high time */

	/* Switch register sets so that changes are double-buffered */
    psQ8->m_nCounterControl ^= (CCTRL_WDOGWSET | CCTRL_WDOGRSET);

    if (duty <= 0) /* counter always low */
    {
        /* Disable the counter. Set the output to zero */
        psQ8->m_nCounterControl &= ~CCTRL_WDOGEN;
        WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
        WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_WDOGLD);
    }
    else if (duty >= period) /* counter always high */
    {
        /* Disable the counter. Set the output to one */
        psQ8->m_nCounterControl &= ~CCTRL_WDOGEN;
        WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
        WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_WDOGLD | CCTRL_WDOGVAL);
    }
    else /* counter oscillates */
    {
        /* Program the low and high pulse times */
        WriteDWordToHwMem(&pRegisters->watchdog.pwm.preloadLow,  period - duty - 1);
        WriteDWordToHwMem(&pRegisters->watchdog.pwm.preloadHigh, duty - 1);

        /* Enable the counter. Allow the output to change after one period */
        if (!(psQ8->m_nCounterControl & CCTRL_WDOGEN)) /* output was constant */
        {
            psQ8->m_nCounterControl |= CCTRL_WDOGEN;
            WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
            WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl | CCTRL_WDOGLD | CCTRL_WDOGVAL);
        }
        else /* allow output to change at period */
            WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
    }
}

/*********************************************************************

  Name:         Q8_wdogUninitialize

  Purpose:      Uninitialize the watchdog timer. Disable it
                and deactivate the watchdog timer features.

---------------------------------------------------------------------*/
INLINE void Q8_wdogUninitialize(PQ8 psQ8)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;

    /* Disable the watchdog counter and features */
    psQ8->m_nCounterControl &= ~(CCTRL_WDOGACT | CCTRL_WDOGEN | CCTRL_WDOGOUTEN | CCTRL_WDOGMODE | CCTRL_WDOGRSET | CCTRL_WDOGWSET);
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);

    /* Clear the watchdog bit in the Interrupt Status Register */
    WriteDWordToHwMem(&pRegisters->interruptStatus, INT_WATCHDOG);
}

/*********************************************************************

  Name:         Q8_wdogEnableInterrupt

  Purpose:      Enables interrupts from the watchdog counter.

---------------------------------------------------------------------*/
INLINE void Q8_wdogEnableInterrupt(PQ8 psQ8)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;

    /* Clear any existing interrupt condition for the watchdog counter */
    WriteDWordToHwMem(&pRegisters->interruptStatus, INT_WATCHDOG);

    /* Enable interrupts from the watchdog counter */
    psQ8->m_nInterruptEnable |= INT_WATCHDOG;
    WriteDWordToHwMem(&pRegisters->interruptEnable, psQ8->m_nInterruptEnable);
}

/*********************************************************************

  Name:         Q8_wdogDisableInterrupt

  Purpose:      Disables interrupts from the watchdog counter.

---------------------------------------------------------------------*/
INLINE void Q8_wdogDisableInterrupt(PQ8 psQ8)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;

    /* Disable interrupts from the watchdog counter */
    psQ8->m_nInterruptEnable &= ~INT_WATCHDOG;
    WriteDWordToHwMem(&pRegisters->interruptEnable, psQ8->m_nInterruptEnable);
}

/*********************************************************************

  Name:         Q8_wdogClearInterrupt

  Purpose:      Clear the watchdog condition. This function clears the
                WATCHDOG bit in the Interrupt Status Register, causing
                the external WATCHDOG line to go high (inactive).

---------------------------------------------------------------------*/
INLINE void Q8_wdogClearInterrupt(Q8 *psQ8)
{
    /* Clear the watchdog bit in the Interrupt Status Register */
    WriteDWordToHwMem(&psQ8->m_pRegisters->interruptStatus, INT_WATCHDOG);
}

/*********************************************************************

  Name:         Q8_wdogEnableOutput

  Purpose:      Enables the output from the counter, _WDOG_OUT.

---------------------------------------------------------------------*/
INLINE void Q8_wdogEnableOutput(PQ8 psQ8)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;

    /* Enable the counter output */
    psQ8->m_nCounterControl |= CCTRL_WDOGOUTEN;
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
}

/*********************************************************************

  Name:         Q8_wdogDisableOutput

  Purpose:      Disables the output from the counter, _WDOG_OUT.

---------------------------------------------------------------------*/
INLINE void Q8_wdogDisableOutput(PQ8 psQ8)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;

    /* Disable the counter output */
    psQ8->m_nCounterControl &= ~CCTRL_WDOGOUTEN;
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
}

/*********************************************************************

  Name:         Q8_wdogEnable

  Purpose:      Enables the counter.

---------------------------------------------------------------------*/
INLINE void Q8_wdogEnable(PQ8 psQ8)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;

    /* Enable the counter and the output if necessary */
    psQ8->m_nCounterControl |= CCTRL_WDOGEN;
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
}

/*********************************************************************

  Name:         Q8_wdogDisable

  Purpose:      Enables the counter.

---------------------------------------------------------------------*/
INLINE void Q8_wdogDisable(PQ8 psQ8)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;

    /* Enable the counter and the output if necessary */
    psQ8->m_nCounterControl &= ~CCTRL_WDOGEN;
    WriteDWordToHwMem(&pRegisters->counterControl, psQ8->m_nCounterControl);
}

/*********************************************************************

  Name:         Q8_wdogRead

  Purpose:      Read the timer.

---------------------------------------------------------------------*/
INLINE uint32_T Q8_wdogRead(PQ8 psQ8)
{
    PQ8Registers pRegisters = psQ8->m_pRegisters;
    return ReadDWordFromHwMem(&pRegisters->watchdog.sq.value);
}

#endif


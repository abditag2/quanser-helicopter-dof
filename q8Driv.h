#ifndef _Q8DRIV_H_
#define _Q8DRIV_H_

// the linux module headers
//#include <linux/config.h>
#include <linux/pci.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
// we included this
#include <linux/irq.h>
#include <linux/interrupt.h>
// end what we added

// Include the Q8 headers 
#include "q8.h"
#include "q8da.h"
#include "q8en.h"
#include "q8dig.h"
#include "q8di.h"
#include "q8do.h"
#include "q8wd.h"
#include "q8tb.h"
#include "q8ad.h"

#define	DRV_NAME	"Q8 Driver"

static const struct pci_device_id Q8_devIDs[] = {
	{
		.vendor = VENDORID_Q8, 
		.device = DEVICEID_Q8, 
		.subvendor = SUBVENDORID_Q8,
		.subdevice = SUBDEVICEID_Q8,
		.class = 0,
		.class_mask= 0,
		.driver_data = 0
	},
	{ /* end: all zeroes */}
};

static struct CardList
{
	PQ8 BrdID[8];
	int BrdInit[8];
	int Count;
} Boards;

char BitChk (char c, char i)				// read specific bit
{
	return ((c >> i) & 1);
}

int BitCount (char c)						// count the number of set bits
{
	int i;
	int count = 0;
	for(i = 0; i < 8; i++)
	{
		if (BitChk(c,i))
			count++;
	}
	return count;
}

//****************************************************
//****************************************************
// Initialise the Q8 board.
//****************************************************
//****************************************************
int Q8_Initialise (int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
        Q8_InitializeHardware(Boards.BrdID[BoardNum]);
	else
		return -1;
	return 1;	
}
EXPORT_SYMBOL(Q8_Initialise);


//****************************************************
//****************************************************
// Q8 board Analog to Digital Conversion Functions.
//****************************************************
//****************************************************

//  Purpose:      Convert from a raw A/D value to a voltage.
real_T Q8_ADCInputToVoltage(int_T value)
{
	return Q8_adcInputToVoltage(value);
}
EXPORT_SYMBOL(Q8_ADCInputToVoltage);

//  Purpose:      Read the selected A/D channels. If the simultaneous flag
//                is specified then the inputs are sampled simultaneously.
//                Note that when only one A/D chip is used (channels 0-3 only
//                or channels 4-7 only), the inputs are always sampled
//                simultaneously. The conversion results are read after all
//                the conversions are complete.
boolean_T Q8_ADCInput(int BoardNum, uint8_T nChannels, int16_T *nValues, boolean_T bSimultaneous)
{
	if (Boards.BrdInit[BoardNum])
		return Q8_adcInput(Boards.BrdID[BoardNum], nChannels, nValues, bSimultaneous);
	return 0;
}
EXPORT_SYMBOL(Q8_ADCInput);

//  Purpose:      Read the selected A/D channels. If the simultaneous flag
//                is specified then the inputs are sampled simultaneously.
//                Note that when only one A/D chip is used (channels 0-3 only
//                or channels 4-7 only), the inputs are always sampled
//                simultaneously. The conversion results are read as they
//                become available. Note that this technique is faster
//                but may inject some noise into the measurements.
boolean_T Q8_ADCInputUsingEOC(int BoardNum, uint8_T nChannels, int16_T *nValues, boolean_T bSimultaneous)
{
	if (Boards.BrdInit[BoardNum])
		return Q8_adcInputUsingEOC(Boards.BrdID[BoardNum], nChannels, nValues, bSimultaneous);
	return 0;
}
EXPORT_SYMBOL(Q8_ADCInputUsingEOC);

//  Purpose:      Preprogram the A/D chips for the selected channels.
//                This function is used when there is only one
//                Analog Input block in the diagram.
void Q8_ADCPreprogram(int BoardNum, uint8_T nChannels, boolean_T bSimultaneous)
{
	if (Boards.BrdInit[BoardNum])
		Q8_adcPreprogram(Boards.BrdID[BoardNum], nChannels, bSimultaneous);
}
EXPORT_SYMBOL(Q8_ADCPreprogram);

//  Purpose:      Read the selected A/D channels. If the simultaneous flag
//                is specified then the inputs are sampled simultaneously.
//                Note that when only one A/D chip is used (channels 0-3 only
//                or channels 4-7 only), the inputs are always sampled
//                simultaneously. The conversion results are read after all
//                the conversions are complete.
//
//                This function assumes that the A/D chips have been
//                preprogrammed for the selected channels using
//                Q8_adcPreprogram.
boolean_T Q8_ADCPreprogrammedInput(int BoardNum, uint8_T nChannels, int16_T *nValues)
{
	if (Boards.BrdInit[BoardNum])
		return Q8_adcPreprogrammedInput(Boards.BrdID[BoardNum], nChannels, nValues);
	return 0;
}
EXPORT_SYMBOL(Q8_ADCPreprogrammedInput);

//  Purpose:      Read the selected A/D channels. If the simultaneous flag
//                is specified then the inputs are sampled simultaneously.
//                Note that when only one A/D chip is used (channels 0-3 only
//                or channels 4-7 only), the inputs are always sampled
//                simultaneously. The conversion results are read as they
//                become available. Note that this technique is faster
//                but may inject some noise into the measurements.
//
//                This function assumes that the A/D chips have been
//                preprogrammed for the selected channels using
//                Q8_adcPreprogram.
boolean_T Q8_ADCPreprogrammedInputUsingEOC(int BoardNum, uint8_T nChannels, int16_T *nValues)
{
	if (Boards.BrdInit[BoardNum])
		return Q8_adcPreprogrammedInputUsingEOC(Boards.BrdID[BoardNum], nChannels, nValues);
	return 0;
}
EXPORT_SYMBOL(Q8_ADCPreprogrammedInputUsingEOC);

//  Purpose:      Initialize the counter to drive the A/D converter(s).
//                This function is used by the Analog Time Base block
//                to perform A/D conversions periodically and interrupt
//                on conversion complete. Note that simultaneous sampling
//                is always used to guarantee that all conversion results
//                are available as soon as the interrupt occurs (only true
//                if we interrupt on the right end of conversion signal!)
void Q8_ADCPreprogramUsingCounter(int BoardNum, uint8_T nChannels, real_T period)
{
	if (Boards.BrdInit[BoardNum])
		Q8_adcPreprogramUsingCounter(Boards.BrdID[BoardNum], nChannels, period);
}
EXPORT_SYMBOL(Q8_ADCPreprogramUsingCounter);

//  Purpose:      Initialize the CNTR_EN line to drive the A/D converter(s).
//                This function is used by the Analog Input External Time Base 
//                block to perform A/D conversions on the rising or falling
//                edge of the CNTR_EN line and to interrupt on conversion complete. 
//                Note that simultaneous sampling is always used to guarantee that 
//                all conversion results are available as soon as the interrupt occurs.
void Q8_ADCPreprogramUsingCounterEnable(int BoardNum, uint8_T nChannels)
{
	if (Boards.BrdInit[BoardNum])
		Q8_adcPreprogramUsingCounterEnable(Boards.BrdID[BoardNum], nChannels);
}
EXPORT_SYMBOL(Q8_ADCPreprogramUsingCounterEnable);

//  Purpose:      Read the selected A/D channels. The conversions have
//                already taken place and are available in the A/D FIFOs.
//
//                This function assumes that the A/D chips have been
//                preprogrammed for the selected channels using
//                Q8_adcPreprogramUsingCounter(Enable).
boolean_T Q8_ADCPreprogrammedInputFromCounter(int BoardNum, uint8_T nChannels, int16_T *nValues)
{
	if (Boards.BrdInit[BoardNum])
		return Q8_adcPreprogrammedInputFromCounter(Boards.BrdID[BoardNum], nChannels, nValues);
	return 0;
}
EXPORT_SYMBOL(Q8_ADCPreprogrammedInputFromCounter);

//  Purpose:      Read the selected A/D channels. The conversions have
//                been trigger, but may not necessarily be available
//                in the A/D FIFOs yet.
//
//                This function assumes that the A/D chips have been
//                preprogrammed for the selected channels using
//                Q8_adcPreprogramUsingCounterEnable. This function
//                is used by the Analog Input External Trigger block.
boolean_T Q8_ADCPreprogrammedInputFromExternalTrigger(int BoardNum, uint8_T nChannels, int16_T *nValues)
{
	if (Boards.BrdInit[BoardNum])
		return Q8_adcPreprogrammedInputFromExternalTrigger(Boards.BrdID[BoardNum], nChannels, nValues);
	return 0;
}
EXPORT_SYMBOL(Q8_ADCPreprogrammedInputFromExternalTrigger);

//  Purpose:      Enables interrupts from the counter.
void Q8_ADCEnableInterrupt(int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		return Q8_adcEnableInterrupt(Boards.BrdID[BoardNum]);
}
EXPORT_SYMBOL(Q8_ADCEnableInterrupt);

//  Purpose:      Disables interrupts from the counter.
void Q8_ADCDisableInterrupt(int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		return Q8_adcDisableInterrupt(Boards.BrdID[BoardNum]);
}
EXPORT_SYMBOL(Q8_ADCDisableInterrupt);

//  Purpose:      Clear the A/D condition. This function clears the
void Q8_ADCClearInterrupt(int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		return Q8_adcClearInterrupt(Boards.BrdID[BoardNum]);
}
EXPORT_SYMBOL(Q8_ADCClearInterrupt);

//  Purpose:      Returns non-zero if the A/D caused the interrupt.
boolean_T Q8_ADCCausedInterrupt(int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		return Q8_adcCausedInterrupt(Boards.BrdID[BoardNum]);

	return 0;
}
EXPORT_SYMBOL(Q8_ADCCausedInterrupt);

//****************************************************
//****************************************************
// Q8 board timer functions.
//****************************************************
//****************************************************

//  Purpose:      Returns non-zero if the timer expired.
boolean_T Q8_CntrTimedOut(int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		return Q8_cntrTimedOut(Boards.BrdID[BoardNum]);

	return 0;
}
EXPORT_SYMBOL(Q8_CntrTimedOut);

//  Purpose:      Set the counter value.
void Q8_CntrSet(int BoardNum, uint32_T nCount)
{
	if (Boards.BrdInit[BoardNum])
		Q8_cntrSet(Boards.BrdID[BoardNum], nCount);
}
EXPORT_SYMBOL(Q8_CntrSet);

//  Purpose:      Initialize the timer. Set the period.
void Q8_CntrPreinitializeCount(int BoardNum, uint32_T nCount, boolean_T bEnableOutput)
{
	if (Boards.BrdInit[BoardNum])
		Q8_cntrPreinitializeCount(Boards.BrdID[BoardNum], nCount, bEnableOutput);
}
EXPORT_SYMBOL(Q8_CntrPreinitializeCount);

//  Purpose:      Initialize the timer. Set the period.
void Q8_CntrInitialize(int BoardNum, real_T period, boolean_T bEnableOutput)
{
	if (Boards.BrdInit[BoardNum])
		Q8_cntrInitialize(Boards.BrdID[BoardNum], period, bEnableOutput);
}
EXPORT_SYMBOL(Q8_CntrInitialize);

//  Purpose:      Initialize the counter as a PWM counter.
void Q8_CntrInitializePWM(int BoardNum, real_T period, real_T duty)
{
	if (Boards.BrdInit[BoardNum])
		Q8_cntrInitializePWM(Boards.BrdID[BoardNum], period, duty);
}
EXPORT_SYMBOL(Q8_CntrInitializePWM);

//  Purpose:      Set the duty cycle for pulse-width modulation. This
//                function can be called while the model is running. PWM
//                mode must have been initialized using Q8_cntrInitializePWM.
//                The period should not be changed.
void Q8_CntrSetPWM(int BoardNum, real_T period, real_T duty)
{
	if (Boards.BrdInit[BoardNum])
		Q8_cntrSetPWM(Boards.BrdID[BoardNum], period, duty);
}
EXPORT_SYMBOL(Q8_CntrSetPWM);

//  Purpose:      Uninitialize the timer.
void Q8_CntrUninitialize(int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		Q8_cntrUninitialize(Boards.BrdID[BoardNum]);
}
EXPORT_SYMBOL(Q8_CntrUninitialize);

//  Purpose:      Reload the timer by reloading the preload
//                value into the counter.
void Q8_CntrReload(int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		Q8_cntrReload(Boards.BrdID[BoardNum]);
}
EXPORT_SYMBOL(Q8_CntrReload);

//  Purpose:      Enables interrupts from the counter.
void Q8_CntrEnableInterrupt(int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		Q8_cntrEnableInterrupt(Boards.BrdID[BoardNum]);
}
EXPORT_SYMBOL(Q8_CntrEnableInterrupt);

//  Purpose:      Disables interrupts from the counter.
void Q8_CntrDisableInterrupt(int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		Q8_cntrDisableInterrupt(Boards.BrdID[BoardNum]);
}
EXPORT_SYMBOL(Q8_CntrDisableInterrupt);

//  Purpose:      Enables the output from the counter, _CNTR_OUT.
void Q8_CntrEnableOutput(int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		Q8_cntrEnableOutput(Boards.BrdID[BoardNum]);
}
EXPORT_SYMBOL(Q8_CntrEnableOutput);

//  Purpose:      Disables the output from the counter, _CNTR_OUT.
void Q8_CntrDisableOutput(int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		Q8_cntrDisableOutput(Boards.BrdID[BoardNum]);
}
EXPORT_SYMBOL(Q8_CntrDisableOutput);

//  Purpose:      Enables the counter.
void Q8_CntrEnable(int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		Q8_cntrEnable(Boards.BrdID[BoardNum]);
}
EXPORT_SYMBOL(Q8_CntrEnable);

//  Purpose:      Enables the counter.
void Q8_CntrDisable(int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		Q8_cntrDisable(Boards.BrdID[BoardNum]);
}
EXPORT_SYMBOL(Q8_CntrDisable);

//  Purpose:      Clear the counter expiration condition. This function clears the
//                CNTR_OUT bit in the Interrupt Status Register.
void Q8_CntrClearInterrupt(int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		Q8_cntrClearInterrupt(Boards.BrdID[BoardNum]);
}
EXPORT_SYMBOL(Q8_CntrClearInterrupt);

//  Purpose:      Read the timer.
uint32_T Q8_CntrRead(int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		return Q8_cntrRead(Boards.BrdID[BoardNum]);
	else 
		return 0;
}
EXPORT_SYMBOL(Q8_CntrRead);

//****************************************************
//****************************************************
// Q8 board watchdog functions.
//****************************************************
//****************************************************

// Returns non-zero if the watchdog timer expired.
int Q8_WDTimeOut (int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		return (int)Q8_wdogTimedOut(Boards.BrdID[BoardNum]);
	else
		return -1;
}
EXPORT_SYMBOL(Q8_WDTimeOut);

//  Purpose:      Set the counter value.
void Q8_WDSet (int BoardNum, uint32_T nCount)
{
	if (Boards.BrdInit[BoardNum])
		Q8_wdogSet(Boards.BrdID[BoardNum], nCount);
}
EXPORT_SYMBOL(Q8_WDSet);

//  Purpose:      Initialize the watchdog timer. Set the period and load it.
//                Do not activate the watchdog timer features.
void Q8_WDPreInit (int BoardNum, uint32_T nCount, boolean_T bEnableOutput)
{
	if (Boards.BrdInit[BoardNum])
		Q8_wdogPreinitializeCount(Boards.BrdID[BoardNum], nCount, bEnableOutput);
}
EXPORT_SYMBOL(Q8_WDPreInit);

//  Purpose:      Initialize the watchdog timer. Set the period and enable it.
//                Do not activate the watchdog timer features.
void Q8_WDInit (int BoardNum, real_T period, boolean_T bEnableOutput)
{
	if (Boards.BrdInit[BoardNum])
		Q8_wdogInitialize(Boards.BrdID[BoardNum], period, bEnableOutput);
}
EXPORT_SYMBOL(Q8_WDInit);

//  Purpose:      Initialize the watchdog timer. Set the period, enable it
//              and activate the watchdog timer features.
void Q8_WDInitActive (int BoardNum, real_T period, boolean_T bEnableOutput)
{
	if (Boards.BrdInit[BoardNum])
		Q8_wdogInitializeActive(Boards.BrdID[BoardNum], period, bEnableOutput);
}
EXPORT_SYMBOL(Q8_WDInitActive);

//  Purpose:      Reload the watchdog timer by reloading the preload
//                value into the counter. This function should be called
//                each sampling instant to prevent the watchdog timer
//                from expiring.
void Q8_WDReload (int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		Q8_wdogReload(Boards.BrdID[BoardNum]);
}
EXPORT_SYMBOL(Q8_WDReload);
//  Purpose:      Initialize the counter as a PWM counter.
void Q8_WDInitPWM (int BoardNum, real_T period, real_T duty)
{
	if (Boards.BrdInit[BoardNum])
		Q8_wdogInitializePWM(Boards.BrdID[BoardNum], period, duty);
}
EXPORT_SYMBOL(Q8_WDInitPWM);

//  Purpose:      Set the duty cycle for pulse-width modulation. This
//                function can be called while the model is running. PWM
//                mode must have been initialized using Q8_wdogInitializePWM.
//                The period should not be changed.
void Q8_WDSetPWM (int BoardNum, real_T period, real_T duty)
{
	if (Boards.BrdInit[BoardNum])
		Q8_wdogSetPWM(Boards.BrdID[BoardNum], period, duty);
}
EXPORT_SYMBOL(Q8_WDSetPWM);

//  Purpose:      Uninitialize the watchdog timer. Disable it
//                and deactivate the watchdog timer features.
void Q8_WDUnInit (int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		Q8_wdogUninitialize(Boards.BrdID[BoardNum]);
}
EXPORT_SYMBOL(Q8_WDUnInit);

//  Purpose:      Enables interrupts from the watchdog counter.
void Q8_WDInterruptEn (int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		Q8_wdogEnableInterrupt(Boards.BrdID[BoardNum]);
}
EXPORT_SYMBOL(Q8_WDInterruptEn);

//  Purpose:      Disables interrupts from the watchdog counter.
void Q8_WDInterruptDisable (int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		Q8_wdogDisableInterrupt(Boards.BrdID[BoardNum]);
}
EXPORT_SYMBOL(Q8_WDInterruptDisable);

//  Purpose:      Clear the watchdog condition. This function clears the
//                WATCHDOG bit in the Interrupt Status Register, causing
//                the external WATCHDOG line to go high (inactive).
void Q8_WDClearInterrupt (int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		Q8_wdogClearInterrupt(Boards.BrdID[BoardNum]);
}
EXPORT_SYMBOL(Q8_WDClearInterrupt);


//  Purpose:      Enables the output from the counter, _WDOG_OUT.
void Q8_WDOutputEn (int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		Q8_wdogEnableOutput(Boards.BrdID[BoardNum]);
}
EXPORT_SYMBOL(Q8_WDOutputEn);

//  Purpose:      Disables the output from the counter, _WDOG_OUT.
void Q8_WDOutputDisable (int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		Q8_wdogDisableOutput(Boards.BrdID[BoardNum]);
}
EXPORT_SYMBOL(Q8_WDOutputDisable);

//  Purpose:      Enables the counter.
void Q8_WDEnable (int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		Q8_wdogEnable(Boards.BrdID[BoardNum]);
}
EXPORT_SYMBOL(Q8_WDEnable);

//  Purpose:      Enables the counter.
void Q8_WDDisable (int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		Q8_wdogDisable(Boards.BrdID[BoardNum]);
}
EXPORT_SYMBOL(Q8_WDDisable);

//  Purpose:      Read the timer.
unsigned long Q8_WDRead (int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		return (unsigned long)Q8_wdogRead(Boards.BrdID[BoardNum]);
	return 0;
}
EXPORT_SYMBOL(Q8_WDRead);

//****************************************************
//****************************************************
//	Q8 Digital I/O Functions
//****************************************************
//****************************************************

// Digital IO Direction control
void Q8_DIGConfigure (int BoardNum, uint32_T nDirections)
{
	if (Boards.BrdInit[BoardNum])
		Q8_digConfigure(Boards.BrdID[BoardNum], nDirections);
}
EXPORT_SYMBOL(Q8_DIGConfigure);

// write to the digital IO pins
void Q8_DIOOutput (int BoardNum, uint32_T nValues)
{
	if (Boards.BrdInit[BoardNum])
		Q8_doOutput(Boards.BrdID[BoardNum], nValues);
}
EXPORT_SYMBOL(Q8_DIOOutput);

// read the status of the digital IO pins
uint32_T  Q8_DIOInput(int BoardNum)
{
	if (Boards.BrdInit[BoardNum])
		return Q8_diInput(Boards.BrdID[BoardNum]);
	else
		return 0;
}
EXPORT_SYMBOL(Q8_DIOInput);

//****************************************************
//****************************************************
//	Q8 Encoder Functions
//****************************************************
//****************************************************

// initialise the encoders
int Q8_EncInitialise(int BoardNum, uint8_T nChannel, boolean_T bcd, uint8_T mode, 
					 uint8_T quadrature, boolean_T enableIndex, boolean_T indexPolarity)
{
	if (Boards.BrdInit[BoardNum])
	    Q8_encInitialize(Boards.BrdID[BoardNum], nChannel, bcd, mode, quadrature, enableIndex, indexPolarity);
	else
		return -1;

	return 1;
}
EXPORT_SYMBOL(Q8_EncInitialise);

// reset a single encoder
void Q8_EncResetSingle (int BoardNum, uint8_T nChannel)
{
	if (Boards.BrdInit[BoardNum])
        Q8_encResetSingleCount(Boards.BrdID[BoardNum], nChannel);
}
EXPORT_SYMBOL(Q8_EncResetSingle);

// reset n encoders
void Q8_EncReset (int BoardNum, uint8_T nChannels)
{
	if (Boards.BrdInit[BoardNum])
		Q8_encResetCounts(Boards.BrdID[BoardNum], nChannels);
}
EXPORT_SYMBOL(Q8_EncReset);

void Q8_EncEnableIndex (int BoardNum, uint8_T nChannel)
{
	if (Boards.BrdInit[BoardNum])
        Q8_encEnableIndex(Boards.BrdID[BoardNum], nChannel);
}
EXPORT_SYMBOL(Q8_EncEnableIndex);

void Q8_EncDisableIndex (int BoardNum, uint8_T nChannel)
{
	if (Boards.BrdInit[BoardNum])
        Q8_encDisableIndex(Boards.BrdID[BoardNum], nChannel);
}
EXPORT_SYMBOL(Q8_EncDisableIndex);

void Q8_EncEnableIndices (int BoardNum, uint8_T nChannels)
{
	if (Boards.BrdInit[BoardNum])
        Q8_encEnableIndices(Boards.BrdID[BoardNum], nChannels);
}
EXPORT_SYMBOL(Q8_EncEnableIndices);

void Q8_EncDisableIndices (int BoardNum, uint8_T nChannels)
{
	if (Boards.BrdInit[BoardNum])
        Q8_encDisableIndices(Boards.BrdID[BoardNum], nChannels);
}
EXPORT_SYMBOL(Q8_EncDisableIndices);

void Q8_EncConfigureIO (int BoardNum, uint8_T nChannel, uint8_T nFlags, boolean_T bLoadCounterOnIndex)
{
	if (Boards.BrdInit[BoardNum])
		Q8_encConfigureIO(Boards.BrdID[BoardNum],nChannel, nFlags, bLoadCounterOnIndex);
}
EXPORT_SYMBOL(Q8_EncConfigureIO);

void Q8_EncResetFlags(int BoardNum, uint8_T nChannel)
{
	if (Boards.BrdInit[BoardNum])
		Q8_encResetFlags(Boards.BrdID[BoardNum], nChannel);
}
EXPORT_SYMBOL(Q8_EncResetFlags);

uint8_T Q8_EncReadFlags (int BoardNum, uint8_T nChannel)
{
	if (Boards.BrdInit[BoardNum])
		return Q8_encReadFlags(Boards.BrdID[BoardNum], nChannel);
	else
		return -1;
}
EXPORT_SYMBOL(Q8_EncReadFlags);

int32_T Q8_EncReadSingleLatch (int BoardNum, uint8_T nChannel)
{
	if (Boards.BrdInit[BoardNum])
		return Q8_encReadSingleLatch(Boards.BrdID[BoardNum], nChannel);
	else
		return -1;
}
EXPORT_SYMBOL(Q8_EncReadSingleLatch);

int32_T Q8_EncInputSingle (int BoardNum, uint8_T nChannel)
{
	if (Boards.BrdInit[BoardNum])
		return Q8_encInputSingle(Boards.BrdID[BoardNum], nChannel);
	else
		return -1;
}
EXPORT_SYMBOL(Q8_EncInputSingle);

void Q8_EncInput (int BoardNum, uint8_T nChannels, int32_T *pnValues)
{
	if (Boards.BrdInit[BoardNum])
		Q8_encInput(Boards.BrdID[BoardNum], nChannels, pnValues);
}
EXPORT_SYMBOL(Q8_EncInput);

void Q8_EncSetCount (int BoardNum, uint8_T nChannel, uint32_T nValue)
{
	if (Boards.BrdInit[BoardNum])
		Q8_encSetCount(Boards.BrdID[BoardNum], nChannel, nValue);
}
EXPORT_SYMBOL(Q8_EncSetCount);

void Q8_EncSetFilterPrescaler (int BoardNum, uint8_T nChannel, uint32_T nValue)
{
	if (Boards.BrdInit[BoardNum])
		Q8_encSetFilterPrescaler(Boards.BrdID[BoardNum], nChannel, nValue);
}
EXPORT_SYMBOL(Q8_EncSetFilterPrescaler);

void Q8_EncRestore (int BoardNum, uint8_T nChannel)
{
	if (Boards.BrdInit[BoardNum])
		Q8_encRestore(Boards.BrdID[BoardNum], nChannel);
}
EXPORT_SYMBOL(Q8_encRestore);

//****************************************************
//****************************************************
//	Q8 DAC Functions
//****************************************************
//****************************************************

uint16_T Q8_DACVoltageToOutput( real_T nVoltage, boolean_T bBipolar, real_T nRange )
{
	return Q8_dacVoltageToOutput( nVoltage, bBipolar, nRange);
}
EXPORT_SYMBOL(Q8_DACVoltageToOutput);

// Configure the DAC channels for bi/unipolar and range
int Q8_DACPreConfigure (int BoardNum, uint8_T nChannel, boolean_T bBipolar, int16_T nRange)
{
	if (Boards.BrdInit[BoardNum])
	    Q8_dacPreconfigure(Boards.BrdID[BoardNum], nChannel, bBipolar, nRange);
	else
		return -1;

	return 1;
}
EXPORT_SYMBOL(Q8_DACPreConfigure);

// Configure the DAC channels for bi/unipolar and range
int Q8_DACConfigure (int BoardNum, uint8_T nChannel, boolean_T bBipolar, int16_T nRange)
{
	if (Boards.BrdInit[BoardNum])
	    Q8_dacConfigure(Boards.BrdID[BoardNum], nChannel, bBipolar, nRange);
	else
		return -1;

	return 1;
}
EXPORT_SYMBOL(Q8_DACConfigure);

// Set the DAC Values via a bitmask Mask which defines the channels
// and an array of ints with the actual values for those channels
void Q8_SetDAC(int BoardNum, uint8_T nChannels, uint16_T *v)
{
	if (Boards.BrdInit[BoardNum])
		Q8_dacOutput(Boards.BrdID[BoardNum], nChannels, v);
}
EXPORT_SYMBOL(Q8_SetDAC);

real_T Q8_DACOutputToVoltage( uint16_T nOutput, real_T nRange )
{
	return Q8_dacOutputToVoltage(nOutput, nRange);
}
EXPORT_SYMBOL(Q8_DACOutputToVoltage);

// read the DAC values that have been set
// read the channels specified by the bitmask Mask
// store them to the int array val for those channels
void Q8_ReadDAC (int BoardNum, uint8_T nChannels, uint16_T *v)
{
	if (Boards.BrdInit[BoardNum])
		Q8_dacRead(Boards.BrdID[BoardNum], nChannels, v);
}
EXPORT_SYMBOL(Q8_ReadDAC);

int16_T Q8_DACRange(int BoardNum, uint8_T nChannel)
{
	if (Boards.BrdInit[BoardNum])
	    return Q8_dacRange(Boards.BrdID[BoardNum], nChannel);

	return 0;
}
EXPORT_SYMBOL(Q8_DACRange);


#endif

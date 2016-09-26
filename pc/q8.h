/*++
Copyright (c) 2002 Quanser Consulting Inc. All Rights Reserved

Module Name:

    Q8.h

Abstract:

    This module contains the common declarations shared by device
    drivers.

Author:

    Daniel R. Madill
     
Environment:

    kernel

Notes:


Revision History:


--*/

#if !defined(_Q8_h)
#define _Q8_h

//#pragma ident	"@(#)q8.h 1.1	04/03/01 SMI"


#ifdef sun
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#endif

#include "Hardware.h"
#include "PlugAndPlay.h"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* Q4 PCI device and vendor IDs. */
#define VENDORID_Q4    0x11E3 /* PCI VendorID */
#define DEVICEID_Q4    0x0010 /* PCI DeviceID */
#define SUBVENDORID_Q4 0x5155 /* PCI SubVendorID */
#define SUBDEVICEID_Q4 0x02AC /* PCI SubDeviceID */

/* Q8 PCI device and vendor IDs. */
#define VENDORID_Q8               0x11E3            /* PCI VendorID */
#define DEVICEID_Q8               0x0010            /* PCI DeviceID */
#define SUBVENDORID_Q8            0x5155            /* PCI SubVendorID */
//#define SUBDEVICEID_Q8            0x0200            /* PCI SubDeviceID */

/* REMARK: modification to support the Q4 */
#define SUBDEVICEID_Q8            SUBDEVICEID_Q4


/* A data structure in which each member maps to */
/* a register on the card. Ensure byte alignment */
/* of fields so that the members are at the correct */
/* offsets. This data structure is the most convenient */
/* way to access the registers on the card. Note that */
/* anonmyous unions and structs are required, which not */
/* all compilers support. Add names to all unions and */
/* structs for such compilers. */

#if defined(_MSC_VER)
#pragma pack(push, registers, 1)
#endif

#ifdef linux
#define		IOMEM	__iomem
#else
#define		IOMEM	volatile 
#endif


typedef IOMEM struct tagQ8Registers
{
    uint32_T interruptEnable;
    int32_T  interruptStatus;
    uint32_T control;
    uint32_T status;

    union
    {
        struct
        {
            uint32_T preload;
            uint32_T value;
        } sq;

        struct
        {
            uint32_T preloadLow;
            uint32_T preloadHigh;
        } pwm;
    } counter;

    union
    {
        struct
        {
            uint32_T preload;
            uint32_T value;
        } sq;

        struct
        {
            uint32_T preloadLow;
            uint32_T preloadHigh;
        } pwm;
    } watchdog;

    uint32_T counterControl;
    uint32_T digitalIO;
    uint32_T digitalDirection;

    union
    {
        struct {
            int16_T adc03;
            int16_T adc47;
        } one;

        int32_T two;      /* on a read */
        int32_T select;   /* on a write, channel selection */
    } analogInput;

    union
    {
        struct {
            uint8_T enc0;
            uint8_T enc2;
            uint8_T enc4;
            uint8_T enc6;

            uint8_T enc1;
            uint8_T enc3;
            uint8_T enc5;
            uint8_T enc7;
        } one;

        uint8_T encs[8];

        struct {
            uint16_T enc02;
            uint16_T enc46;

            uint16_T enc13;
            uint16_T enc57;
        } two;

        struct {
            uint32_T enc0246;   /* ENCODER_DATA_A */
            uint32_T enc1357;   /* ENCODER_DATA_B */
        } four;

        uint64_T all;
    } encoderData;

    union
    {
        struct {
            uint8_T enc0;
            uint8_T enc2;
            uint8_T enc4;
            uint8_T enc6;

            uint8_T enc1;
            uint8_T enc3;
            uint8_T enc5;
            uint8_T enc7;
        } one;

        uint8_T encs[8];

        struct {
            uint16_T enc02;
            uint16_T enc46;

            uint16_T enc13;
            uint16_T enc57;
        } two;

        struct {
            uint32_T enc0246;   /* ENCODER_CONTROL_A */
            uint32_T enc1357;   /* ENCODER_CONTROL_B */
        } four;

        uint64_T all;
    } encoderControl;

    union
    {
        struct {
            uint16_T dac0;
            uint16_T dac4;

            uint16_T dac1;
            uint16_T dac5;

            uint16_T dac2;
            uint16_T dac6;

            uint16_T dac3;
            uint16_T dac7;
        } one;

        uint16_T dacs[8];

        struct {
            uint32_T dac04;
            uint32_T dac15;
            uint32_T dac26;
            uint32_T dac37;
        } two;

        uint32_T pairs[4];

        struct
        {
            uint64_T dac0415;
            uint64_T dac2637;
        } four;
    } analogOutput;

    union
    {
        struct {
            uint16_T dac03;
            uint16_T dac47;
        } four;

        uint32_T all;
    } analogUpdate;

    uint32_T reserved2[6];
    
    union
    {
        struct {
            uint16_T dac03;
            uint16_T dac47;
        } four;

        uint32_T all;
    } analogMode;

    union
    {
        struct {
            uint16_T dac03;
            uint16_T dac47;
        } four;

        uint32_T all;
    } analogModeUpdate;

    uint32_T reserved3[227];

} Q8Registers, * PQ8Registers;

#if defined(_MSC_VER)
#pragma pack(pop, registers)
#endif

/* Interrupt Status Register bit definitions */

#define INT_PEND       0x80000000   /* Card is asserting interrupt line (enabled interrupt source detected) */
#define INT_EXTINT     0x00800000   /* Edge detected on EXT_INT line (external interrupt signalled) */
#define INT_FUSE       0x00400000   /* Rising edge detected on FUSE input (fuse blown) */
#define INT_WATCHDOG   0x00200000   /* Watchdog counter expired */
#define INT_CNTROUT    0x00100000   /* General purpose counter expired */
#define INT_ADC47RDY   0x00080000   /* A/D converter on channels 4-7 is finished conversions (and ready to do more) */
#define INT_ADC03RDY   0x00040000   /* A/D converter on channels 0-3 is finished conversions (and ready to do more) */
#define INT_ADC47EOC   0x00020000   /* A/D converter on channels 4-7 has completed a conversion */
#define INT_ADC03EOC   0x00010000   /* A/D converter on channels 0-3 has completed a conversion */
#define INT_ENC7FLG2   0x00008000   /* Rising edge detected on encoder channel 7 flag 2  */
#define INT_ENC7FLG1   0x00004000   /* Rising edge detected on encoder channel 7 flag 1 */
#define INT_ENC6FLG2   0x00002000   /* Rising edge detected on encoder channel 6 flag 2 */
#define INT_ENC6FLG1   0x00001000   /* Rising edge detected on encoder channel 6 flag 1 */
#define INT_ENC5FLG2   0x00000800   /* Rising edge detected on encoder channel 5 flag 2 */
#define INT_ENC5FLG1   0x00000400   /* Rising edge detected on encoder channel 5 flag 1 */
#define INT_ENC4FLG2   0x00000200   /* Rising edge detected on encoder channel 4 flag 2 */
#define INT_ENC4FLG1   0x00000100   /* Rising edge detected on encoder channel 4 flag 1 */
#define INT_ENC3FLG2   0x00000080   /* Rising edge detected on encoder channel 3 flag 2 */
#define INT_ENC3FLG1   0x00000040   /* Rising edge detected on encoder channel 3 flag 1 */
#define INT_ENC2FLG2   0x00000020   /* Rising edge detected on encoder channel 2 flag 2 */
#define INT_ENC2FLG1   0x00000010   /* Rising edge detected on encoder channel 2 flag 1 */
#define INT_ENC1FLG2   0x00000008   /* Rising edge detected on encoder channel 1 flag 2 */
#define INT_ENC1FLG1   0x00000004   /* Rising edge detected on encoder channel 1 flag 1 */
#define INT_ENC0FLG2   0x00000002   /* Rising edge detected on encoder channel 0 flag 2 */
#define INT_ENC0FLG1   0x00000001   /* Rising edge detected on encoder channel 0 flag 1 */

/* Control Register bit definitions */

#define CTRL_CNTRENCV   0x10000000  /* 0 = use COUNTER to trigger auto conversions, 1 = use CNTR_EN to trigger auto conversions */
#define CTRL_EXTACT     0x08000000  /* 0 = deactivate watchdog features of EXT_INT, 1 = activate watchdog features of EXT_INT */
#define CTRL_EXTPOL     0x04000000  /* 0 = EXT_INT active low, 1 = EXT_INT active high */
#define CTRL_DAC47TR    0x02000000  /* 0 = double-buffer DAC47, 1 = transparent mode on DAC47 */
#define CTRL_DAC03TR    0x01000000  /* 0 = double-buffer DAC03, 1 = transparent mode on DAC03 */
#define CTRL_ADC47CV    0x00800000  /* 0 = no conversions on ADC47, 1 = start conversions on ADC47 */
#define CTRL_ADCSTBY    0x00400000  /* 0 = full power mode, 1 = A/D standby mode */
#define CTRL_ADC47CT    0x00200000  /* 0 = manual conversions on ADC47, 1 = use auto conversions */
#define CTRL_ADC47HS    0x00100000  /* 0 = Control Register selects channels, 1 = A/D Register selects channels */
#define CTRL_ADCSL7     0x00080000  /* 0 = do not include channel 7, 1 = include channel 7 in conversions */
#define CTRL_ADCSL6     0x00040000  /* 0 = do not include channel 6, 1 = include channel 6 in conversions */
#define CTRL_ADCSL5     0x00020000  /* 0 = do not include channel 5, 1 = include channel 5 in conversions */
#define CTRL_ADCSL4     0x00010000  /* 0 = do not include channel 4, 1 = include channel 4 in conversions */
#define CTRL_ADC03CV    0x00008000  /* 0 = no conversions on ADC03, 1 = start conversions on ADC03 */
                     /* 0x00004000  // reserved. Set to zero. */
#define CTRL_ADC03CT    0x00002000  /* 0 = manual conversions on ADC03, 1 = use auto conversions */
#define CTRL_ADC03HS    0x00001000  /* 0 = Control Register selects channels, 1 = A/D Register selects channels */
#define CTRL_ADCSL3     0x00000800  /* 0 = do not include channel 3, 1 = include channel 3 in conversions */
#define CTRL_ADCSL2     0x00000400  /* 0 = do not include channel 2, 1 = include channel 2 in conversions */
#define CTRL_ADCSL1     0x00000200  /* 0 = do not include channel 1, 1 = include channel 1 in conversions */
#define CTRL_ADCSL0     0x00000100  /* 0 = do not include channel 0, 1 = include channel 0 in conversions */
#define CTRL_ENC7IDX    0x00000080  /* 0 = disable channel 7 index pulse, 1 = enable channel 7 index pulse */
#define CTRL_ENC6IDX    0x00000040  /* 0 = disable channel 6 index pulse, 1 = enable channel 6 index pulse */
#define CTRL_ENC5IDX    0x00000020  /* 0 = disable channel 5 index pulse, 1 = enable channel 5 index pulse */
#define CTRL_ENC4IDX    0x00000010  /* 0 = disable channel 4 index pulse, 1 = enable channel 4 index pulse */
#define CTRL_ENC3IDX    0x00000008  /* 0 = disable channel 3 index pulse, 1 = enable channel 3 index pulse */
#define CTRL_ENC2IDX    0x00000004  /* 0 = disable channel 2 index pulse, 1 = enable channel 2 index pulse */
#define CTRL_ENC1IDX    0x00000002  /* 0 = disable channel 1 index pulse, 1 = enable channel 1 index pulse */
#define CTRL_ENC0IDX    0x00000001  /* 0 = disable channel 0 index pulse, 1 = enable channel 0 index pulse */

#define CTRL_ADC47SCK   CTRL_ADCSL5 /* 0 = use internal clock for ADC47, 1 = use common clock */
#define CTRL_ADC03SCK   CTRL_ADCSL1 /* 0 = use internal clock for ADC03, 1 = use common clock */

#define CTRL_ADC03MASK  0x0000ff00  /* bitmask extract ADC03 bits */
#define CTRL_ADC47MASK  0x00ff0000  /* bitmask extract ADC47 bits */
#define CTRL_ADCMASK    (CTRL_ADC03MASK | CTRL_ADC47MASK)

/* Status Register bit definitions */

#define STAT_CNTREN     0x01000000
#define STAT_EXTINT     0x00800000
#define STAT_FUSE       0x00400000
#define STAT_ADC47FST   0x00200000
#define STAT_ADC03FST   0x00100000
#define STAT_ADC47RDY   0x00080000
#define STAT_ADC03RDY   0x00040000
#define STAT_ADC47EOC   0x00020000
#define STAT_ADC03EOC   0x00010000
#define STAT_ENC7FLG2   0x00008000
#define STAT_ENC7FLG1   0x00004000
#define STAT_ENC6FLG2   0x00002000
#define STAT_ENC6FLG1   0x00001000
#define STAT_ENC5FLG2   0x00000800
#define STAT_ENC5FLG1   0x00000400
#define STAT_ENC4FLG2   0x00000200
#define STAT_ENC4FLG1   0x00000100
#define STAT_ENC3FLG2   0x00000080
#define STAT_ENC3FLG1   0x00000040
#define STAT_ENC2FLG2   0x00000020
#define STAT_ENC2FLG1   0x00000010
#define STAT_ENC1FLG2   0x00000008
#define STAT_ENC1FLG1   0x00000004
#define STAT_ENC0FLG2   0x00000002
#define STAT_ENC0FLG1   0x00000001

/* Counter Control Register bit definitions */

#define CCTRL_WDOGLD    0x02000000  /* 0 = no load operation, 1 = load watchdog counter from active preload and WDOGVAL */
#define CCTRL_WDOGVAL   0x01000000  /* value of watchdog counter output (ignored if WDOGLD = 0) */
#define CCTRL_WDOGACT   0x00800000  /* 0 = deactive watchdog features of WATCHDOG counter, 1 = active watchdog features */
#define CCTRL_WDOGSEL   0x00400000  /* 0 = WATCHDOG output reflects watchdog interrupt state (active low), 1 = output is output of WATCHDOG counter */
#define CCTRL_WDOGOUTEN 0x00200000  /* 0 = disable WATCHDOG output (output always high), 1 = enable WATCHDOG output. Value determined by WDOGSEL */
#define CCTRL_WDOGPRSEL 0x00100000  /* 0 = use Watchdog Preload Low Register, 1 = use Watchdog Preload High Register. Ignored if WDOGMODE = 1. */
#define CCTRL_WDOGWSET  0x00080000  /* 0 = use watchdog register set #0 for writes to preload registers, 1 = use register set #1 */
#define CCTRL_WDOGRSET  0x00040000  /* 0 = use watchdog register set #0 for active set and reads, 1 = use register set #1 */
#define CCTRL_WDOGMODE  0x00020000  /* 0 = square wave mode (WDOGPRSEL selects preload register), 1 = PWM mode (both preload low and high registers used) */
#define CCTRL_WDOGEN    0x00010000  /* 0 = disable counting of watchdog counter, 1 = enable watchdog counter */

#define CCTRL_CNTRLD    0x00000200  /* 0 = no load operation, 1 = load counter from active preload and CNTRVAL */
#define CCTRL_CNTRVAL   0x00000100  /* value of counter output (ignored if CNTRLD = 0) */
#define CCTRL_CNTRENPOL 0x00000040  /* 0 = CNTREN active high (CNTRENCV=0) or falling edge (CNTRENCV=1), 1 = opposite */
#define CCTRL_CNTROUTEN 0x00000020  /* 0 = disable CNTR_OUT output (output always high), 1 = enable CNTR_OUT output. Value is output of COUNTER */
#define CCTRL_CNTRPRSEL 0x00000010  /* 0 = use Counter Preload Low Register, 1 = use Counter Preload High Register. Ignored if CNTRMODE = 1 */
#define CCTRL_CNTRWSET  0x00000008  /* 0 = use counter register set #0 for writes to preload registers, 1 = use register set #1 */
#define CCTRL_CNTRRSET  0x00000004  /* 0 = use counter register set #0 for active set and reads, 1 = use register set #1 */
#define CCTRL_CNTRMODE  0x00000002  /* 0 = square wave mode (CNTRPRSEL selects preload register), 1 = PWM mode (both preload low and high registers used) */
#define CCTRL_CNTREN    0x00000001  /* 0 = disable counting of counter, 1 = enable counter */

/* Encoder Control Byte bit definitions */
#define ENC_ONE_CHANNEL         0x00    /* operate on only one channel (according to which byte accessed) */
#define ENC_BOTH_CHANNELS       0x80    /* operate on both the even and odd channel */

#define ENC_RLD_REGISTER        0x00    /* select Reset and Load Signal Decoders Register (RLD) */

#define ENC_RLD_RESET_BP        0x01    /* reset byte pointer (BP) */

#define ENC_RLD_RESET_CNTR      0x02    /* reset count value (CNTR) to zero  */
#define ENC_RLD_RESET_FLAGS     0x04    /* reset flags (borrow (BT), carry (CT), compare (CPT) and sign (S) flags) */
#define ENC_RLD_RESET_E         0x06    /* reset error flag (E) */

#define ENC_RLD_SET_CNTR        0x08    /* transfer preload register (PR) to counter (CNTR) */
#define ENC_RLD_GET_CNTR        0x10    /* transfer counter (CNTR) to output latch (OL) */
#define ENC_RLD_SET_PSC         0x18    /* transfer preload register LSB (PR0) to filter prescaler (PSC) */

#define ENC_CMR_REGISTER        0x20    /* select Counter Mode Register (CMR) */

#define ENC_CMR_BINARY          0x00    /* binary count mode */
#define ENC_CMR_BCD             0x01    /* BCD count mode */

#define ENC_CMR_NORMAL          0x00    /* normal count (24-bit value which wraps on overflow or underflow) */
#define ENC_CMR_RANGE           0x02    /* range limit (counter freezes at zero (CNTR=0) or preload value (CNTR=PR), count resumes when direction reversed) */
#define ENC_CMR_NONRECYCLE      0x04    /* non-recycle count (counter disabled on underflow or overflow, re-enabled by reset or load operation on CNTR) */
#define ENC_CMR_MODULO          0x06    /* modulo-N (counter reset to zero when CNTR=PR, set to PR when CNTR=0). */

#define ENC_CMR_NONQUADRATURE   0x00    /* non-quadrature mode (digital count and direction inputs instead of encoder A and B signals) */
#define ENC_CMR_QUADRATURE_1X   0x08    /* quadrature 1X mode */
#define ENC_CMR_QUADRATURE_2X   0x10    /* quadrature 1X mode */
#define ENC_CMR_QUADRATURE_4X   0x18    /* quadrature 1X mode */

#define ENC_IOR_REGISTER        0x40    /* select Input/Output Control Register (IOR) */

#define ENC_IOR_DISABLE_AB      0x00    /* disable A and B inputs */
#define ENC_IOR_ENABLE_AB       0x01    /* enable A and B inputs */

#define ENC_IOR_LCNTR_LOAD      0x00    /* LCNTR/LOL pin is Load CNTR */
#define ENC_IOR_LCNTR_LATCH     0x02    /* LCNTR/LOL pin is Latch CNTR in OL */

#define ENC_IOR_RCNTR_RESET     0x00    /* RCNTR/ABG pin is Reset CNTR */
#define ENC_IOR_RCNTR_GATE      0x04    /* RCNTR/ABG pin is A and B Enable gate */

#define ENC_IOR_CARRY_BORROW    0x00    /* FLG1 is CARRY, FLG2 is BORROW */
#define ENC_IOR_COMPARE_BORROW  0x08    /* FLG1 is COMPARE, FLG2 is BORROW */
#define ENC_IOR_CARRY_UPDOWN    0x10    /* FLG1 is CARRY/BORROW, FLG2 is UP/DOWN (FLAG bit 5) */
#define ENC_IOR_INDEX_ERROR     0x18    /* FLG1 is IDX (FLAG bit 6), FLG2 is E (FLAG bit 4) */

#define ENC_IDR_REGISTER        0x60    /* Index Control Register (IDR) */

#define ENC_IDR_DISABLE_INDEX   0x00    /* Disable index */
#define ENC_IDR_ENABLE_INDEX    0x01    /* Enable index */

#define ENC_IDR_NEG_INDEX       0x00    /* Negative index polarity */
#define ENC_IDR_POS_INDEX       0x02    /* Positive index polarity */

#define ENC_IDR_LCNTR_INDEX     0x00    /* LCNTR/LOL pin is indexed */
#define ENC_IDR_RCNTR_INDEX     0x04    /* RCNTR/ABG pin is indexed (*** do not use with Q8 card ***) */

/* Analog Mode bit definitions */

#define DAC0_UNIPOLAR_10V       0x0000
#define DAC0_BIPOLAR_5V         0x0080
#define DAC0_BIPOLAR_10V        0x0880

#define DAC1_UNIPOLAR_10V       0x0000
#define DAC1_BIPOLAR_5V         0x0040
#define DAC1_BIPOLAR_10V        0x0440

#define DAC2_UNIPOLAR_10V       0x0000
#define DAC2_BIPOLAR_5V         0x0020
#define DAC2_BIPOLAR_10V        0x0220

#define DAC3_UNIPOLAR_10V       0x0000
#define DAC3_BIPOLAR_5V         0x0010
#define DAC3_BIPOLAR_10V        0x0110

#define DAC4_UNIPOLAR_10V       0x00000000
#define DAC4_BIPOLAR_5V         0x00800000
#define DAC4_BIPOLAR_10V        0x08800000

#define DAC5_UNIPOLAR_10V       0x00000000
#define DAC5_BIPOLAR_5V         0x00400000
#define DAC5_BIPOLAR_10V        0x04400000

#define DAC6_UNIPOLAR_10V       0x00000000
#define DAC6_BIPOLAR_5V         0x00200000
#define DAC6_BIPOLAR_10V        0x02200000

#define DAC7_UNIPOLAR_10V       0x00000000
#define DAC7_BIPOLAR_5V         0x00100000
#define DAC7_BIPOLAR_10V        0x01100000

/* Conversion factors */
#define ADC_FACTOR          (10.0 / 8192)   /* converts analog input value to a voltage */

/* Register offsets from base address (equivalent to Q8Registers structure) */

#define Q8_INTERRUPT_ENABLE_REGISTER(base)      ((base) + 0x00)
#define Q8_INTERRUPT_STATUS_REGISTER(base)      ((base) + 0x04)
#define Q8_CONTROL_REGISTER(base)               ((base) + 0x08)
#define Q8_STATUS_REGISTER(base)                ((base) + 0x0C)
#define Q8_COUNTER_PRELOAD_REGISTER(base)       ((base) + 0x10)
#define Q8_COUNTER_REGISTER(base)               ((base) + 0x14)
#define Q8_COUNTER_PRELOAD_LOW_REGISTER(base)   ((base) + 0x10)
#define Q8_COUNTER_PRELOAD_HIGH_REGISTER(base)  ((base) + 0x14)
#define Q8_WATCHDOG_PRELOAD_REGISTER(base)      ((base) + 0x18)
#define Q8_WATCHDOG_REGISTER(base)              ((base) + 0x1C)
#define Q8_WATCHDOG_PRELOAD_LOW_REGISTER(base)  ((base) + 0x18)
#define Q8_WATCHDOG_PRELOAD_HIGH_REGISTER(base) ((base) + 0x1C)
#define Q8_COUNTER_CONTROL_REGISTER(base)       ((base) + 0x20)
#define Q8_DIGITAL_IO_REGISTER(base)            ((base) + 0x24)
#define Q8_DIGITAL_DIRECTION_REGISTER(base)     ((base) + 0x28)
#define Q8_AD_REGISTER(base)                    ((base) + 0x2C)
#define Q8_ENCODER_DATA_REGISTER_A(base)        ((base) + 0x30)
#define Q8_ENCODER_DATA_REGISTER_B(base)        ((base) + 0x34)
#define Q8_ENCODER_CONTROL_REGISTER_A(base)     ((base) + 0x38)
#define Q8_ENCODER_CONTROL_REGISTER_B(base)     ((base) + 0x3C)
#define Q8_DA_OUTPUT_REGISTER_A(base)           ((base) + 0x40)
#define Q8_DA_OUTPUT_REGISTER_B(base)           ((base) + 0x44)
#define Q8_DA_OUTPUT_REGISTER_C(base)           ((base) + 0x48)
#define Q8_DA_OUTPUT_REGISTER_D(base)           ((base) + 0x4C)
#define Q8_DA_UPDATE_REGISTER(base)             ((base) + 0x50)
                                                                /* 0x54-0x6B reserved */
#define Q8_DA_MODE_REGISTER(base)               ((base) + 0x6C)
#define Q8_DA_MODE_UPDATE_REGISTER(base)        ((base) + 0x70)
                                                                /* 0x74-0x3FF reserved */

#define Q8_SIZEOF_ADDRESS_SPACE                 0x0400          /* Size of address space in bytes */

/* Data structure for state information needed to control the Q8 card */
typedef struct tagQ8
{
#ifdef sun
    int          m_nRegsHdlr;          /* Memory-mapped registers Handler */
#elif linux
#else
    Card         m_sCard;              /* Card information */
#endif
    PQ8Registers m_pRegisters;         /* Virtual base address of card */
    uint32_T     m_nDigitalDirections; /* Direction of each digital channel */
    uint32_T     m_nDacModes;          /* D/A modes */
    uint32_T     m_nControl;           /* Contents of Control Register (could be read but slower to do so) */
    uint32_T     m_nCounterControl;    /* Contents of Counter Control Register (could be read but slower to do so) */
    uint32_T     m_nInterruptEnable;   /* Contents of Interrupt Enable Register (could be read but slower to do so) */
    int32_T      m_nEncoderCounts[8];  /* Encoder counts prior to initialization (for preserving counts across runs) */
    uint8_T      m_nAdc03Reads;        /* Number of channels being converted on ADC03 (for preprogrammed I/O only) */
    uint8_T      m_nAdc47Reads;        /* Number of channels being converted on ADC47 (for preprogrammed I/O only) */
} Q8, *PQ8;

INLINE boolean_T Q8_IsValid(PQ8 psQ8)
{
    return (psQ8 != NULL && psQ8->m_pRegisters != 0L);
}

INLINE boolean_T Q8_isBoardNumberParamOK(int_T nBoard)
{
    /* Restrict the number of boards to 16. This limit is really only */
    /* imposed to catch people entering a base address like 0x320 as */
    /* the board number. */
    return (nBoard >= 0 && nBoard < 16);
}

INLINE void Q8_encSave(PQ8 psQ8)
{
    const uint32_T rldLatch = ENC_BOTH_CHANNELS | ENC_RLD_REGISTER | ENC_RLD_GET_CNTR | ENC_RLD_RESET_BP;

    PQ8Registers pRegisters = psQ8->m_pRegisters;
    uint32_T nEvenLSBs;
    uint32_T nEvenISBs;
    uint32_T nEvenMSBs;
    uint32_T nOddLSBs;
    uint32_T nOddISBs;
    uint32_T nOddMSBs;
    register int32_T nValue;

    /* Latch the counter values of all channels */
    WriteDWordToHwMem(&pRegisters->encoderControl.four.enc0246, 
        (rldLatch << 24) | (rldLatch << 16) | (rldLatch << 8) | rldLatch);

    /* Read the count values of the selected channels */
    nEvenLSBs = ReadDWordFromHwMem(&pRegisters->encoderData.four.enc0246);
    nOddLSBs  = ReadDWordFromHwMem(&pRegisters->encoderData.four.enc1357);

    nEvenISBs = ReadDWordFromHwMem(&pRegisters->encoderData.four.enc0246);
    nOddISBs  = ReadDWordFromHwMem(&pRegisters->encoderData.four.enc1357);
    
    nEvenMSBs = ReadDWordFromHwMem(&pRegisters->encoderData.four.enc0246);
    nOddMSBs  = ReadDWordFromHwMem(&pRegisters->encoderData.four.enc1357);

    /* Extract the desired count values */
    psQ8->m_nEncoderCounts[0] = ((int8_T)(nEvenMSBs & 0xff) << 16) | ((nEvenISBs & 0xff) << 8) | (nEvenLSBs & 0xff);
    psQ8->m_nEncoderCounts[1] = ((int8_T)(nOddMSBs & 0xff) << 16)  | ((nOddISBs & 0xff) << 8)  | (nOddLSBs & 0xff);
    psQ8->m_nEncoderCounts[2] = ((int16_T)(nEvenMSBs & 0xff00) << 8) | (nEvenISBs & 0xff00) | ((nEvenLSBs & 0xff00) >> 8);
    psQ8->m_nEncoderCounts[3] = ((int16_T)(nOddMSBs & 0xff00) << 8)  | (nOddISBs & 0xff00)  | ((nOddLSBs & 0xff00) >> 8);
        
    nValue = (nEvenMSBs & 0xff0000) | ((nEvenISBs & 0xff0000) >> 8) | ((nEvenLSBs & 0xff0000) >> 16);
    if (nEvenMSBs & 0x800000)
        nValue |= 0xff000000;
    psQ8->m_nEncoderCounts[4] = nValue;

    nValue = (nOddMSBs & 0xff0000) | ((nOddISBs & 0xff0000) >> 8) | ((nOddLSBs & 0xff0000) >> 16);
    if (nOddMSBs & 0x800000)
        nValue |= 0xff000000;
    psQ8->m_nEncoderCounts[5] = nValue;

    psQ8->m_nEncoderCounts[6] = ((int32_T)(nEvenMSBs & 0xff000000) >> 8) | ((nEvenISBs & 0xff000000) >> 16) | ((nEvenLSBs & 0xff000000) >> 24);
    psQ8->m_nEncoderCounts[7] = ((int32_T)(nOddMSBs & 0xff000000) >> 8) | ((nOddISBs & 0xff000000) >> 16) | ((nOddLSBs & 0xff000000) >> 24);
}

INLINE void Q8_InitializeHardware(PQ8 psQ8)
{
    static const uint32_T cmrInit  = ENC_BOTH_CHANNELS | ENC_CMR_REGISTER | ENC_CMR_BINARY | ENC_CMR_NORMAL | ENC_CMR_QUADRATURE_4X;
    static const uint32_T rldInit1 = ENC_BOTH_CHANNELS | ENC_RLD_REGISTER | ENC_RLD_RESET_BP | ENC_RLD_RESET_E;
    static const uint32_T rldInit2 = ENC_BOTH_CHANNELS | ENC_RLD_REGISTER | ENC_RLD_RESET_BP | ENC_RLD_RESET_FLAGS | ENC_RLD_SET_PSC;
    static const uint32_T rldInit3 = ENC_BOTH_CHANNELS | ENC_RLD_REGISTER | ENC_RLD_RESET_BP | ENC_RLD_RESET_CNTR;
    static const uint32_T iorInit  = ENC_BOTH_CHANNELS | ENC_IOR_REGISTER | ENC_IOR_ENABLE_AB | ENC_IOR_LCNTR_LATCH | ENC_IOR_INDEX_ERROR;
    static const uint32_T idrInit  = ENC_BOTH_CHANNELS | ENC_IDR_REGISTER | ENC_IDR_DISABLE_INDEX | ENC_IDR_POS_INDEX | ENC_IDR_LCNTR_INDEX;

    register PQ8Registers pRegisters = psQ8->m_pRegisters;

    /* 
        Assume all digital I/O channels are inputs, but do not reprogram the card itself until the user
        specifically programs the directions. This prevents glitches when the users programs a digital I/O
        as zero for the initial or final value. Also read the current analog output modes.
    */
    psQ8->m_nDigitalDirections = 0;                       /* Record digital I/O configuration in device extension */
    psQ8->m_nDacModes = pRegisters->analogMode.all;       /* Read the current analog output modes to prevent glitches */

    /* Initialize the Control Register such that A/D conversions are performed using the */
    /* internal clocks and the channels are programmed using the A/D Register. Note that the */
    /* watchdog timer is also disabled and deactivated, all encoder index pulses are disabled, */
    /* all polarities are positive and the D/A converters are placed in non-transparent mode. */
    psQ8->m_nControl = CTRL_ADC47HS | CTRL_ADC03HS; /* use internal clocks, manual conversions, A/D Register selects channels */
    WriteDWordToHwMem(&pRegisters->control, psQ8->m_nControl);

    /* Initialize the Counter Control Register such that both counters are disabled and in the default modes */
    /* Also initialize all counter preload registers to their minimum count value */
    psQ8->m_nCounterControl = 0;
    WriteDWordToHwMem(&pRegisters->counterControl, CCTRL_CNTRWSET | CCTRL_WDOGWSET);
    WriteDWordToHwMem(&pRegisters->counter.pwm.preloadLow,   0);
    WriteDWordToHwMem(&pRegisters->counter.pwm.preloadHigh,  0);
    WriteDWordToHwMem(&pRegisters->watchdog.pwm.preloadLow,  0);
    WriteDWordToHwMem(&pRegisters->watchdog.pwm.preloadHigh, 0);

    WriteDWordToHwMem(&pRegisters->counterControl, 0);
    WriteDWordToHwMem(&pRegisters->counter.pwm.preloadLow,   0);
    WriteDWordToHwMem(&pRegisters->counter.pwm.preloadHigh,  0);
    WriteDWordToHwMem(&pRegisters->watchdog.pwm.preloadLow,  0);
    WriteDWordToHwMem(&pRegisters->watchdog.pwm.preloadHigh, 0);

    /* 
        Save the encoder counts in case we want to restore their value after initialization. Note that
        they must be saved because programming the encoder modes can corrupt the count
    */
    Q8_encSave(psQ8);

    /* Initialize the encoders. Initialize all registers of the encoders since the encoders come up in */
    /* an indeterminate state after reset. */
    WriteDWordToHwMem(&pRegisters->encoderControl.four.enc0246, (cmrInit << 24) | (cmrInit << 16) | (cmrInit << 8) | cmrInit);

    /* Reset the BP and E flags. */
    WriteDWordToHwMem(&pRegisters->encoderControl.four.enc0246, (rldInit1 << 24) | (rldInit1 << 16) | (rldInit1 << 8) | rldInit1);

    /* Set the preload registers to zero so that the prescaler can be set to zero, and so that the */
    /* preload registers have a predefined value at the outset */
    WriteDWordToHwMem(&pRegisters->encoderData.four.enc0246, 0);
    WriteDWordToHwMem(&pRegisters->encoderData.four.enc1357, 0);
    WriteDWordToHwMem(&pRegisters->encoderData.four.enc0246, 0);
    WriteDWordToHwMem(&pRegisters->encoderData.four.enc1357, 0);
    WriteDWordToHwMem(&pRegisters->encoderData.four.enc0246, 0);
    WriteDWordToHwMem(&pRegisters->encoderData.four.enc1357, 0);

    /* Transfer PR0 to the filter prescaler (PSC), and reset the BP again, as well as the flags */
    WriteDWordToHwMem(&pRegisters->encoderControl.four.enc0246, (rldInit2 << 24) | (rldInit2 << 16) | (rldInit2 << 8) | rldInit2);
    WriteDWordToHwMem(&pRegisters->encoderControl.four.enc0246, (rldInit3 << 24) | (rldInit3 << 16) | (rldInit3 << 8) | rldInit3);

    /* Enable the A and B encoder inputs, and configure the index input (LCNTR) as a latch CNTR signal. Also set the flags */
    /* so that FLG1 is the index pulse and FLG2 is the error signal. */
    WriteDWordToHwMem(&pRegisters->encoderControl.four.enc0246, (iorInit << 24) | (iorInit << 16) | (iorInit << 8) | iorInit);

    /* Disable the index pulse. */
    WriteDWordToHwMem(&pRegisters->encoderControl.four.enc0246, (idrInit << 24) | (idrInit << 16) | (idrInit << 8) | idrInit);

    /* Disable all interrupts and clear the interrupt status register */
    WriteDWordToHwMem(&pRegisters->interruptEnable, 0);
    WriteDWordToHwMem(&pRegisters->interruptStatus, 0xffffffff);
    psQ8->m_nInterruptEnable = 0; /* Record interrupts as being disabled */
}

#ifndef linux

INLINE boolean_T Q8_Initialize(uint8_T nBoardNumber, PQ8 psQ8)
{

	/* Clear structure initially */
    memset(psQ8, 0, sizeof(Q8));

#if !defined(sun)
    /* Find the Q8 board. Return FALSE if the specified board can't be found. */
    if (QcFindCard(BUS_PCI, INTERFACE_PCI, VENDORID_Q8, DEVICEID_Q8,
            SUBVENDORID_Q8, SUBDEVICEID_Q8, nBoardNeumber, &psQ8->m_sCard))
#endif
    {
#ifdef sun
	char device[20];
	sprintf(device, "/dev/q8_%d", nBoardNumber);
	if ((psQ8->m_nRegsHdlr = open(device, O_RDWR)) < 0) {
	    perror("can't open Q8");
	    return (FALSE);
	}

        if ((psQ8->m_pRegisters = (PQ8Registers) mmap(NULL, Q8_SIZEOF_ADDRESS_SPACE,
	    PROT_READ | PROT_WRITE, MAP_SHARED, psQ8->m_nRegsHdlr, 0)) == MAP_FAILED) {
	    perror("cant map Q8");
	    close(psQ8->m_nRegsHdlr);
	    return (FALSE);
	}
#else
	/* Initialize the Q8 state information as we set up the card. */
	psQ8->m_pRegisters = (PQ8Registers)QcMapToVirtualMemory(
	    psQ8->m_sCard.m_sConfig.m_uHeader.Type0.m_BaseAddresses[0],
	    Q8_SIZEOF_ADDRESS_SPACE, FALSE);

#endif /* sun */
        if (psQ8->m_pRegisters)
		{
			Q8_InitializeHardware(psQ8);
			return TRUE;
		}
    }

    /* Mark the Q8 structure as invalid by setting the base address to zero. */
    psQ8->m_pRegisters = NULL;
    return FALSE;
}

INLINE void Q8_Uninitialize(PQ8 psQ8)
{
    if (Q8_IsValid(psQ8))
    {
        /* Unmap the virtual memory */
#ifdef sun
	close(psQ8->m_nRegsHdlr);
#else /* sun */
    QcUnmapFromVirtualMemory((void *)(psQ8->m_pRegisters), Q8_SIZEOF_ADDRESS_SPACE);
#endif /* sun */
    }
}

#endif

#endif

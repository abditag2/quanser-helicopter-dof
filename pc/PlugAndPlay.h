#if !defined(_PlugAndPlay_h)
#define _PlugAndPlay_h

//#pragma ident	"@(#)PlugAndPlay.h 1.1	04/03/01 SMI"


#if !defined(_DATATYPES_H)
#include "DataTypes.h"
#endif

typedef enum tagBusType
{
    BUS_UNDEFINED = -1,
    BUS_CMOS,
    BUS_EISA,
    BUS_POS,
    BUS_CBUS,
    BUS_PCI,
    BUS_VME,
    BUS_NUBUS,
    BUS_PCMCIA,
    BUS_MPI,
    BUS_MPSA,
    BUS_PNPISA,

	NUMBER_OF_BUS_TYPES
} BusType;

typedef enum tagInterfaceType {
    INTERFACE_UNDEFINED = -1,
    INTERFACE_INTERNAL,
    INTERFACE_ISA,
    INTERFACE_EISA,
    INTERFACE_MICROCHANNEL,
    INTERFACE_TURBOCHANNEL,
    INTERFACE_PCI,
    INTERFACE_VME,
    INTERFACE_NUBUS,
    INTERFACE_PCMCIA,
    INTERFACE_CBUS,
    INTERFACE_MPI,
    INTERFACE_MPSA,
    INTERFACE_PROCESSOR_INTERNAL,
    INTERFACE_INTERNAL_POWER,
    INTERFACE_PNPISA,

	NUMBER_OF_INTERFACE_TYPES
} InterfaceType;

#define NUMBER_OF_PCI_HEADER0_ADDRESSES		6
#define NUMBER_OF_PCI_HEADER1_ADDRESSES		2

typedef struct tagConfig
{
    uint16_T m_VendorID;
    uint16_T m_DeviceID;
    uint16_T m_Command;
    uint16_T m_Status;
    uint8_T  m_RevisionID;
    uint8_T  m_ProgIf;
    uint8_T  m_SubClass;
    uint8_T  m_BaseClass;
    uint8_T  m_CacheLineSize;
    uint8_T  m_LatencyTimer;
    uint8_T  m_HeaderType;
    uint8_T  m_BIST;

    union 
	{
        struct tagPciHeader0 
		{
            uint32_T m_ConfigBaseAddresses[NUMBER_OF_PCI_HEADER0_ADDRESSES];
            uint32_T m_CIS;
            uint16_T m_SubVendorID;
            uint16_T m_SubSystemID;
            uint32_T m_ROMBaseAddress;
			uint8_T  m_Capabilities;
			uint8_T  m_Reserved0[7];
            uint8_T  m_InterruptLine;
            uint8_T  m_InterruptPin;
            uint8_T  m_MinimumGrant;
            uint8_T  m_MaximumLatency;
			uint8_T  m_DeviceSpecific[192];
            uint64_T m_BaseAddresses[NUMBER_OF_PCI_HEADER0_ADDRESSES];
			uint8_T  m_nBaseAddresses;
        } Type0;

        struct tagPciHeader1 
		{
            uint32_T m_ConfigBaseAddresses[NUMBER_OF_PCI_HEADER1_ADDRESSES];
			uint8_T  m_PrimaryBusNumber;
			uint8_T  m_SecondaryBusNumber;
			uint8_T  m_SubordinateBusNumber;
			uint8_T  m_SecondaryLatencyTimer;
			uint8_T  m_IOBase;
			uint8_T  m_IOLimit;
			uint16_T m_SecondaryStatus;
			uint16_T m_MemoryBase;
			uint16_T m_MemoryLimit;
			uint16_T m_PrefetchableMemoryBase;
			uint16_T m_PrefetchableMemoryLimit;
			uint32_T m_PrefetchableBaseMSL;
			uint32_T m_PrefetchableLimitMSL;
			uint16_T m_IOBaseMSW;
			uint16_T m_IOLimitMSW;
			uint8_T  m_Capabilities;
			uint8_T  m_Reserved1[3];
			uint32_T m_ROMBaseAddress;
			uint8_T  m_InterruptLine;
			uint8_T  m_InterruptPin;
			uint16_T m_BridgeControl;
			uint8_T  m_DeviceSpecific[192];
            uint64_T m_BaseAddresses[NUMBER_OF_PCI_HEADER1_ADDRESSES];
			uint8_T  m_nBaseAddresses;
        } Type1;
    } m_uHeader;


} PciConfig;

typedef struct tagCard
{
	BusType   m_eBusType;
	uint_T    m_nBusNumber;
	uint8_T   m_nDeviceNumber;
	uint8_T   m_nFunctionNumber;
	uint8_T   m_nBoardNumber;		/* card number (if more than one card) */
    void *    m_pDevice;            /* device-specific information */
	PciConfig m_sConfig;
} Card, *LPCard;

typedef struct tagDmaBuffer
{
	uint8_T *m_pBuffer;		/* virtual buffer address */
	uint32_T m_nBusAddress;	/* PCI bus buffer address */
} DmaBuffer, LPDmaBuffer;


/**
 *  Structure used for passing interrupt information to SimuLinux from
 *  TimeBase blocks.
 */
typedef struct tagInterrupt
{
    InterfaceType m_InterfaceType;
    uint_T        m_BusNumber;
    uint_T        m_InterruptLine;
    uint_T        m_InterruptVector;
} Interrupt, *PInterrupt;

/**
 * QcFindCard
 *
 * Parameters:
 *	eType		 = type of bus (eg. BUS_PCI)
 *  nVendorID	 = vendor ID of card
 *  nDeviceID    = device ID of card
 *  nSubVendorID = subvendor ID of card, or -1 to ignore subvendor ID
 *  nSubDeviceID = subdevice ID of card, or -1 to ignore subdevice ID
 *  nBoardNumber = number of board if more than one board of this type in the
 *                 system.
 *  psCard       = information returned about the card
 *
 * Return value:
 *   boolean_T   = TRUE if card found, otherwise FALSE.
 */
EXTERN boolean_T QcFindCard(BusType eType, InterfaceType eInterface, 
					 uint16_T nVendorID, uint16_T nDeviceID,
					 uint16_T nSubVendorID, uint16_T nSubDeviceID, 
					 uint8_T nBoardNumber, LPCard psCard);

/**
 * QcMapToVirtualMemory
 *
 * This function must be called to use any memory base addresses returned by
 * QcFindCard. Note that the m_BaseAddress array should be used as the source
 * of addresses, not the m_ConfigBaseAddress array, in the Card structure.
 *
 * When you are finished with the memory it must be unmapped using
 * QcUnmapFromVirtualMemory.
 *
 * Parameters:
 *    qwBaseAddress = the base address to map from physical to virtual memory
 *    cbLength      = the amount of physical memory to map into virtual memory,
 *                    starting at the base address.
 *    bCacheEnable  = set to true to enable caching of the memory. This flag
 *                    should only be set to true for devices that support
 *                    caching.
 *
 * Return value:
 *    The return value is a pointer to the virtual memory address
 *    corresponding to the given base address.
 */
EXTERN void * QcMapToVirtualMemory(uint64_T qwBaseAddress, uint32_T cbLength, 
				boolean_T bCacheEnable);

/**
 * QcUnmapFromVirtualMemory
 *
 * This function must be called to unmap any virtual memory that was mapped
 * using QcMapToVirtualMemory. If the memory is not unmapped it may he lost
 * until the next reboot!
 *
 * Parameters:
 *    pBaseAddress = virtual address returned by QcMapToVirtualMemory.
 *
 * Return value:
 *    The return value is TRUE if the memory was unmapped successfully and
 *    FALSE otherwise.
 */
EXTERN boolean_T QcUnmapFromVirtualMemory(void *pBaseAddress,
                uint32_T cbLength);

/**
 * QcAllocateDmaBuffer
 *
 * This function allocates memory for a DMA buffer. It returns the virtual
 * address of the buffer as well as the physical address in the DmaBuffer
 * structure. Buffers allocated using this function must be freed using
 * QcFreeDmaBuffer.
 *
 * The DMA buffer is guaranteed to be in contiguous physical memory. Since
 * this memory is in short supply, this function should not be used lightly.
 *
 * Parameters:
 *    nBytes  = number of bytes to allocate
 *    pDmaBuf = structure filled in with details of allocated DMA buffer
 *
 * Return value:
 *    The return value is TRUE if the buffer was allocated successfully.
 */
EXTERN boolean_T QcAllocateDmaBuffer(LPCard psCard, uint32_T nBytes,
        DmaBuffer *pDmaBuf);

/**
 * QcFreeDmaBuffer
 *
 * This function is used to free a DMA buffer allocated by QcAllocateDmaBuffer.
 *
 * Parameters:
 *    pDmaBuf = the DmaBuffer structure returned by QcAllocateDmaBuffer.
 *
 * Return value:
 *    The return value is TRUE if the memory was freed successfully and FALSE
 *    otherwise.
 */
EXTERN boolean_T QcFreeDmaBuffer(LPCard psCard, uint32_T nBytes,
        DmaBuffer *pDmaBuf);

#endif


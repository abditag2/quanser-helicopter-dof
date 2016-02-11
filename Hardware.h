#if !defined(_HARDWARE_H)
#define _HARDWARE_H

//#pragma ident	"@(#)Hardware.h 1.1	04/03/01 SMI"

#ifdef _MSC_VER

#include <conio.h>

#define inb(port)           _inp((uint16_T)(port))
#define inw(port)           _inpw((uint16_T)(port))
#define inl(port)           _inpd((uint16_T)(port))
#define outb(value, port)   _outp((uint16_T)(port),  value)
#define outw(value, port)   _outpw((uint16_T)(port), value)
#define outl(value, port)   _outpd((uint16_T)(port), value)

#define nanopause()           _outp(0x80, _inp(0x80))
#define inb_p(port)           (inb(port), nanopause())
#define inw_p(port)           (inw(port), nanopause())
#define inl_p(port)           (inl(port), nanopause())
#define outb_p(value, port)   do { outb(value, port); nanopause(); } while (0)
#define outw_p(value, port)   do { outw(value, port); nanopause(); } while (0)
#define outl_p(value, port)   do { outl(value, port); nanopause(); } while (0)

#define readb(addr)         *(volatile unsigned char *) (addr)
#define readw(addr)         *(volatile unsigned short *)(addr)
#define readl(addr)         *(volatile unsigned long *) (addr)
#define writeb(value, addr) do { *(volatile unsigned char *) (addr) = (value); } while(0)
#define writew(value, addr) do { *(volatile unsigned short *)(addr) = (value); } while(0)
#define writel(value, addr) do { *(volatile unsigned long *) (addr) = (value); } while(0)

#elif defined (sun)
/*
 * If one access several times the same address in a raw,
 * make sure an aggressive optimizer will not discard
 * all successive accesses but the first one.
 */
static inline unsigned char readb(volatile unsigned char *addr) { return *addr; }
static inline unsigned short readw(volatile unsigned short *addr) { return *addr; }
static inline unsigned long readl(volatile unsigned long *addr) { return *addr; }
static inline void writeb(unsigned char value, volatile unsigned char *addr) { *addr = value; }
static inline void writew(unsigned short value, volatile unsigned short *addr) { *addr = value; }
static inline void writel(unsigned long value, volatile unsigned long *addr) { *addr = value; }

#elif defined(linux)

#include <asm/io.h>

#else

#error "platform not supported"

#endif

#ifndef sun
#define ReadByteFromHwPort(port)            inb(port)
#define WriteByteToHwPort(port,val)         outb(val, port)
#define ReadWordFromHwPort(port)            inw(port)
#define WriteWordToHwPort(port,val)         outw(val, port)
#define ReadDWordFromHwPort(port)           inl(port)
#define WriteDWordToHwPort(port,val)        outl(val, port)

#define SlowReadByteFromHwPort(port)        inb_p(port)
#define SlowWriteByteToHwPort(port,val)     outb_p(val, port)
#define SlowReadWordFromHwPort(port)        inw_p(port)
#define SlowWriteWordToHwPort(port,val)     outw_p(val, port)
#define SlowReadDWordFromHwPort(port)       inl_p(port)
#define SlowWriteDWordToHwPort(port,val)    outl_p(val, port)

#define ReadByteFromHwMem(addr)             readb(addr)
#define WriteByteToHwMem(addr,val)          writeb(val, addr)
#define ReadWordFromHwMem(addr)             readw(addr)
#define WriteWordToHwMem(addr,val)          writew(val, addr)
#define ReadDWordFromHwMem(addr)            readl(addr)
#define WriteDWordToHwMem(addr,val)         writel(val, addr)
#else /* sun */
#define ReadByteFromHwMem(addr)             readb((volatile unsigned char *)(addr))
#define WriteByteToHwMem(addr,val)          writeb((unsigned char)(val), (volatile unsigned char *)(addr))
#define ReadWordFromHwMem(addr)             readw((volatile unsigned short *)(addr))
#define WriteWordToHwMem(addr,val)          writew((unsigned short)(val), (volatile unsigned short *)(addr))
#define ReadDWordFromHwMem(addr)            readl((volatile unsigned long *)(addr))
#define WriteDWordToHwMem(addr,val)         writel((unsigned long)(val), (volatile unsigned long *)(addr))
#endif /* sun */

#if !defined(LoByte)
#define LoByte(word)   ((word) & 0x00ff)
#endif
#if !defined(HiByte)
#define HiByte(word)   (((word) >> 8) & 0x00ff)
#endif

#endif

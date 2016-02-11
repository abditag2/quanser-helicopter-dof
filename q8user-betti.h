#ifndef _Q8USER_H_
#define _Q8USER_H_

#include <linux/ioctl.h>

/* free slot in Documentation/ioctl-number.txt */
#define Q8_IOCTL_MAGIC 'g'


/* Max len in bytes for example 2 argument */
#define Q8_EXAMPLE2_MAXLEN 9


/* ioctl commands */
#define Q8_IOCTL_EXAMPLE1 _IO(Q8_IOCTL_MAGIC, 0)
#define Q8_IOCTL_EXAMPLE2 _IOR(Q8_IOCTL_MAGIC, 1, \
					char[Q8_EXAMPLE2_MAXLEN])
#define Q8_IOCTL_EXAMPLE3 _IOW(Q8_IOCTL_MAGIC, 2, unsigned int)
#define Q8_RD_ENC1 _IOR(Q8_IOCTL_MAGIC, 3, int)
#define Q8_WR_DAC _IOW(Q8_IOCTL_MAGIC, 4, int[4])
#define Q8_RD_ENCS _IOR(Q8_IOCTL_MAGIC, 5, int[3])
#define Q8_WR_DACS _IOW(Q8_IOCTL_MAGIC, 6, int[2])
#define Q8_ENC_0 _IOR(Q8_IOCTL_MAGIC, 7, int)
#define Q8_ENC_1 _IOR(Q8_IOCTL_MAGIC, 8, int)
#define Q8_ENC_2 _IOR(Q8_IOCTL_MAGIC, 9, int)
#define Q8_DAC_0 _IOW(Q8_IOCTL_MAGIC, 10, int)
#define Q8_DAC_1 _IOW(Q8_IOCTL_MAGIC, 11, int)
#define Q8_ENC _IOR(Q8_IOCTL_MAGIC, 12, int[3])
#define Q8_IOCTL_MINNR 0
#define Q8_IOCTL_MAXNR 12

/* May need to move this back to get_enc.c*/
#define DRIVER_NAME "Q8driver"
#define DEVICE_FILE_NAME "/dev/q8"

#endif


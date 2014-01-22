/*
 *		Century <www.centurysys.co.jp>
 *			- Century's push switch driver header
 */
#ifndef __OBSPUSHSW_H__
#define __OBSPUSHSW_H__

#include <linux/ioctl.h>
#include <linux/major.h>

#define	PUSHSW_INT			25
#define	PUSHSW_MAJOR		(MISC_MAJOR)

#define	PUSHSW_IOCTL_BASE	'P'

#define	PSWIOC_GETSTATUS	_IOR(PUSHSW_IOCTL_BASE, 0, int)
#define	PSWIOC_WAITPUSH		_IOR(PUSHSW_IOCTL_BASE, 1, int)
#define	PSWIOC_GETWAITPID	_IOR(PUSHSW_IOCTL_BASE, 2, int)

#define	PSWIOF_PUSHED		(1)
#define	PSWIOF_NOTPUSHED	(0)
#endif	/* __OBSPUSHSW_H__ */

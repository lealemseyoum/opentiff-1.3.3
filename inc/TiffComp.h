/*
 * Copyright (c) 2000-2009, Eastman Kodak Company
 * All rights reserved.
 * 
 * Portions of the Original Code is
 * Copyright (c) 1988-1996 Sam Leffler
 * Copyright (c) 1991-1996 Silicon Graphics, Inc.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification,are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright notice, 
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the 
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Eastman Kodak Company nor the names of its 
 *       contributors may be used to endorse or promote products derived from 
 *       this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 * 
 *
 * Creation Date: 2/18/2000
 *
 * Original Author:
 * George Sotak <george.sotak@kodak.com>
 * 
 * Contributor(s): 
 * Chris Lin <ti.lin@kodak.com> 
 */ 


#ifndef _COMPAT_
#define	_COMPAT_
/*
 * This file contains a hodgepodge of definitions and
 * declarations that are needed to provide compatibility
 * between the native system and the base implementation
 * that the library assumes.
 *
 * NB: This file is a mess.
 */

/*
 * Setup basic type definitions and function declaratations.
 */

/*
 * Simplify Acorn RISC OS identifier (to avoid confusion with Acorn RISC iX
 * and with defunct Unix Risc OS)
 * No need to specify __arm - hey, Acorn might port the OS, no problem here!
 */
#ifdef __acornriscos
#undef __acornriscos
#endif
#if defined(__acorn) && defined(__riscos)
#define __acornriscos
#endif

#if defined(__MWERKS__) || defined(THINK_C)
#include <unix.h>
#include <math.h>
#endif

#include <stdio.h>

#if defined(__PPCC__) || defined(__SC__) || defined(__MRC__)
#include <types.h>
#elif !defined(__MWERKS__) && !defined(THINK_C) && !defined(__acornriscos) && !defined(applec)
#include <sys/types.h>
#endif

#if defined(VMS)
#include <file.h>
#include <unixio.h>
#elif !defined(__acornriscos)
#include <fcntl.h>
#endif

/*
 * This maze of checks controls defines or not the
 * target system has BSD-style typdedefs declared in
 * an include file and/or whether or not to include
 * <unistd.h> to get the SEEK_* definitions.  Some
 * additional includes are also done to pull in the
 * appropriate definitions we're looking for.
 */
#if defined(__MWERKS__) || defined(THINK_C) || defined(__PPCC__) || defined(__SC__) || defined(__MRC__)
#include <stdlib.h>
#define	BSDTYPES
#define	HAVE_UNISTD_H	0
#elif defined(_WINDOWS) || defined(WIN32) || defined(_Windows)
#define	BSDTYPES
#include <windows.h>
#elif defined(OS2_16) || defined(OS2_32)
#define	BSDTYPES
#elif defined(__acornriscos)
#include <stdlib.h>
#define	BSDTYPES
#define	HAVE_UNISTD_H	0
#elif defined(VMS)
#define	HAVE_UNISTD_H	0
#else
#define	HAVE_UNISTD_H	1
#endif

/*
 * The library uses the ANSI C/POSIX SEEK_*
 * definitions that should be defined in unistd.h
 * (except on system where they are in stdio.h and
 * there is no unistd.h).
 */
#if !defined(SEEK_SET) && HAVE_UNISTD_H
#include <unistd.h>
#endif

/*
 * The library uses memset, memcpy, and memcmp.
 * ANSI C and System V define these in string.h.
 */
#include <string.h>

/*
 * dblparam_t is the type that a double precision
 * floating point value will have on the parameter
 * stack (when coerced by the compiler).
 */
/* Note: on MacPowerPC "extended" is undefined. So only use it for 68K-Macs */
#if defined(__SC__) || defined(THINK_C)
typedef extended dblparam_t;
#else
typedef double dblparam_t;
#endif

/*
 * If your compiler supports inline functions, then
 * set INLINE appropriately to get the known hotspots
 * in the library expanded inline.
 */
#if defined(__GNUC__)
#if defined(__STRICT_ANSI__)
#define	INLINE	__inline__
#else
#define	INLINE	inline
#endif
#else /* !__GNUC__ */
#define	INLINE
#endif


/*
 * GCC needs some of this stuff in later versions
 */
#ifdef __GNUC__
#if ( __GNUC__ > 3 )
#include <string.h>
#endif
#endif

/*
 * GLOBALDATA is a macro that is used to define global variables
 * private to the library.  We use this indirection to hide
 * brain-damage in VAXC (and GCC) under VAX/VMS.  In these
 * environments the macro places the variable in a non-shareable
 * program section, which ought to be done by default (sigh!)
 *
 * Apparently DEC are aware of the problem as this behaviour is the
 * default under VMS on AXP.
 *
 * The GNU C variant is untested.
 */
#if defined(VAX) && defined(VMS)
#if defined(VAXC)
#define GLOBALDATA(TYPE,NAME)	extern noshare TYPE NAME
#endif
#if defined(__GNUC__)
#define GLOBALDATA(TYPE,NAME)	extern TYPE NAME \
	asm("_$$PsectAttributes_NOSHR$$" #NAME)
#endif
#else	/* !VAX/VMS */
#define GLOBALDATA(TYPE,NAME)	extern TYPE NAME
#endif

#if defined(__acornriscos)
/*
 * osfcn.h is part of C++Lib on Acorn C/C++, and as such can't be used
 * on C alone. For that reason, the relevant functions are
 * implemented in tif_acorn.c, and the elements from the header
 * file are included here.
 */
#if defined(__cplusplus)
#include <osfcn.h>
#else
#define	O_RDONLY	0
#define	O_WRONLY	1
#define	O_RDWR		2
#define	O_APPEND	8
#define	O_CREAT		0x200
#define	O_TRUNC		0x400
typedef long off_t;
extern int open(const char *name, int flags, int mode);
extern int close(int fd);
extern int write(int fd, const char *buf, int nbytes);
extern int read(int fd, char *buf, int nbytes);
extern off_t lseek(int fd, off_t offset, int whence);
extern int creat(const char *path, int mode);
#endif /* __cplusplus */
#endif /* __acornriscos */

#endif /* _COMPAT_ */

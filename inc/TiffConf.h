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


#ifndef _TIFFCONF_
#define	_TIFFCONF_
/*
 * Library Configuration Definitions.
 *
 * This file defines the default configuration for the library.
 * If the target system does not have make or a way to specify
 * #defines on the command line, this file can be edited to
 * configure the library.  Otherwise, one can override portability
 * and configuration-related definitions from a Makefile or command
 * line by defining FEATURE_SUPPORT and COMPRESSION_SUPPORT (see below).
 */


/*
 * UNIX systems should run the configure script to generate
 * a TiffConfig.h file that reflects the system capabilities.
 * Doing this obviates all the dreck done in TiffComp.h.
 * This only needs to be called when we're building the library
 */
#if defined(OPENTIFF_BUILDING)
#include "TiffConfig.h"
#else
#include "TiffComp.h"
#endif

#include <cstdio>
#ifndef macintosh
#include <sys/types.h>
#endif

#if defined( HAVE_FCNTL_H )
#include <fcntl.h>
#endif

#if defined( HAVE_UNISTD_H )
#include <unistd.h>
#endif

/*
 *  * GCC needs some of this stuff in later versions
 *   */
#ifdef __GNUC__
#if ( __GNUC__ > 3 )
#include <string.h>
#endif
#endif


/*
 * General portability-related defines:
 *
 * HAVE_IEEEFP		define as 0 or 1 according to the floating point
 *			format suported by the machine
 * BSDTYPES		define this if your system does NOT define the
 *			usual 4BSD typedefs u_int et. al.
 * HAVE_MMAP		enable support for memory mapping read-only files;
 *			this is typically deduced by the configure script
 * HOST_FILLORDER	native cpu bit order: one of FILLORDER_MSB2LSB
 *			or FILLODER_LSB2MSB; this is typically set by the
 *			configure script
 * HOST_BIGENDIAN	native cpu byte order: 1 if big-endian (Motorola)
 *			or 0 if little-endian (Intel); this may be used
 *			in codecs to optimize code
 */
#if !defined( HAVE_IEEEFP )
#define	HAVE_IEEEFP	1
#endif

#if defined(HAVE_MMAP) || ((defined _MSC_VER) && (defined WIN32))
#if !defined(ENABLE_MEM_MAP) // allow user to override
#define ENABLE_MEM_MAP 1
#if defined(unix) || defined(__unix)
#include <sys/mman.h>
#endif
#endif // !defined(ENABLE_MEM_MAP)
#else // no support for mem mapping
#define ENABLE_MEM_MAP 0
#endif

#if defined( HAVE_FILLORDER_LSB2MSB )
#define	HOST_FILLORDER	FILLORDER_LSB2MSB
#else
#define	HOST_FILLORDER	FILLORDER_MSB2LSB
#endif

#if defined( WORDS_BIGENDIAN )
#define	HOST_BIGENDIAN 1
#elif defined (macintosh)
#define HOST_BIGENDIAN 1
#else
#define	HOST_BIGENDIAN 0
#endif

#ifndef FEATURE_SUPPORT
/*
 * Feature support definitions:
 *
 *    COLORIMETRY_SUPPORT enable support for 6.0 colorimetry tags
 *    YCBCR_SUPPORT	enable support for 6.0 YCbCr tags
 *    CMYK_SUPPORT	enable support for 6.0 CMYK tags
 */
#define	COLORIMETRY_SUPPORT
#define	YCBCR_SUPPORT
#define	CMYK_SUPPORT
#endif /* FEATURE_SUPPORT */

#ifndef COMPRESSION_SUPPORT
/*
 * Compression support defines:
 *
 *    CCITT_SUPPORT	enable support for CCITT Group 3 & 4 algorithms
 *    PACKBITS_SUPPORT	enable support for Macintosh PackBits algorithm
 *    LZW_SUPPORT	enable support for LZW algorithm
 *    THUNDER_SUPPORT	enable support for ThunderScan 4-bit RLE algorithm
 *    NEXT_SUPPORT	enable support for NeXT 2-bit RLE algorithm
 *    OJPEG_SUPPORT	enable support for 6.0-style JPEG DCT algorithms
 *			(no builtin support, only a codec hook)
 *    JPEG_SUPPORT	enable support for post-6.0-style JPEG DCT algorithms
 *			(requires freely available IJG software, see tif_jpeg.c)
 *    ZIP_SUPPORT	enable support for Deflate algorithm
 *			(requires freely available zlib software, see tif_zip.c)
 *    PIXARLOG_SUPPORT	enable support for Pixar log-format algorithm
 */
#define	CCITT_SUPPORT
#define	PACKBITS_SUPPORT
#define	LZW_SUPPORT
#define	THUNDER_SUPPORT
#define	NEXT_SUPPORT
#endif /* COMPRESSION_SUPPORT */

/*
 * If JPEG compression is enabled then we must also include
 * support for the colorimetry and YCbCr-related tags.
 */
#ifdef JPEG_SUPPORT
#ifndef YCBCR_SUPPORT
#define	YCBCR_SUPPORT
#endif
#ifndef COLORIMETRY_SUPPORT
#define	COLORIMETRY_SUPPORT
#endif
#endif /* JPEG_SUPPORT */

/*
 * ``Orthogonal Features''
 *
 * STRIPCHOP_SUPPORT	automatically convert single-strip uncompressed images
 *			to mutiple strips of ~8Kb (for reducing memory use)
 * SUBIFD_SUPPORT	enable support for SubIFD tag (thumbnails and such)
 */
#ifndef STRIPCHOP_SUPPORT
#define	STRIPCHOP_SUPPORT	1	/* enable strip chopping */
#endif
#ifndef SUBIFD_SUPPORT
#define	SUBIFD_SUPPORT		1	/* enable SubIFD tag (330) support */
#endif

#endif /* _TIFFCONF_ */

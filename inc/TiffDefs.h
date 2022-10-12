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
 * Chris Lin <ti.lin@kodak.com> 
 * 
 * Contributor(s): 
 * George Sotak <george.sotak@kodak.com>
 */ 

#ifndef _TIFF_DEFS_H
#define _TIFF_DEFS_H
/*
 * This define can be used in code that requires
 * compilation-related definitions specific to a
 * version or versions of the library.  Runtime
 * version checking should be done based on the
 * string returned by TIFFGetVersion.
 */
#define	TIFFLIB_VERSION	19960307	/* March 7, 1996 */


#define	TIFF_VERSION	42

#define	TIFF_BIGENDIAN		0x4d4d
#define	TIFF_LITTLEENDIAN	0x4949


#ifndef NULL
#define	NULL	0
#endif

#define TIFF_MAINIFD 0  // used in location field of tagsxk

    
/* for tiff.tif_flags */
#define	TIFF_FILLORDER		0x0003	/* natural bit fill order for machine */
#define	TIFF_DIRTYHEADER	0x0004	/* header must be written on close */
//#define	TIFF_DIRTYDIRECT	0x0008	/* current directory must be written */
#define	TIFF_BUFFERSETUP	0x0010	/* data buffers setup */
#define	TIFF_CODERSETUP		0x0020	/* encoder/decoder setup done */
#define	TIFF_BEENWRITING	0x0040	/* written 1+ scanlines to file */
#define	TIFF_SWAB		0x0080	/* byte swap file information */
#define	TIFF_NOBITREV		0x0100	/* inhibit bit reversal logic */
#define	TIFF_MYBUFFER		0x0200	/* my raw data buffer; free on close */
#define	TIFF_ISTILED		0x0400	/* file is tile, not strip- based */
#define	TIFF_MAPPED		0x0800	/* file is mapped into memory */
#define	TIFF_POSTENCODE		0x1000	/* need call to postencode routine */
#define	TIFF_INSUBIFD		0x2000	/* currently writing a subifd */
#define	TIFF_UPSAMPLED		0x4000	/* library is doing data up-sampling */ 
#define	TIFF_STRIPCHOP		0x8000	/* enable strip chopping support */
#ifdef _EK_TIFF_
#define TIFF_NONIMAGEDIR	0x10000 /* non-image bearing dir, e.g., EXIF, Sound*/
#define TIFF_INEXIFIFD      0x20000
#define TIFF_INSOUNDIFD     0x40000
#define TIFF_INGPSIFD		0x80000
#define TIFF_ININTERIFD		0x100000
#endif


#endif // _TIFF_DEFS_H

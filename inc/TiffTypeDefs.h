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

#ifndef _TIFF_TYPEDEFS_H
#define _TIFF_TYPEDEFS_H

#ifdef __APPLE_CC__
#define unix
#endif

#include "TiffDefs.h"

////////////////////////
//	WARNING DISABLING
////////////////////////
#ifdef _MSC_VER
#pragma warning (disable:4503)
#pragma warning (disable:4786)
#pragma warning (disable:4251)
#endif // _MSC_VER

/////////////////////////
// WINDDOW DLL STUFF
/////////////////////////
#ifdef WIN32

#if defined( EKTIFF_BUILD_DLL)
#define EKTIFF_DECL __declspec(dllexport)
#define EKTIFF_EXPLICIT 'e'
#elif  (defined( EKTIFF_DLL))
#define EKTIFF_DECL __declspec(dllimport)
#define EKTIFF_EXPLICIT 'i'
#define EKTIFF_TMPL_EXT extern
#endif // defined( EKTIFF_BUILD_DLL)

#endif // WIN32

#ifndef EKTIFF_DECL
#define EKTIFF_DECL
#endif

#ifndef EKTIFF_TMPL_EXT
#define EKTIFF_TMPL_EXT
#endif


///////////////////////////
// NAMESPACE 
///////////////////////////
#ifdef __mips
#define EKTIFF_USING_STD
#endif

#ifdef __SUNPRO_CC
#define EKTIFF_USING_STD
#endif

#ifdef __GNUC__
#ifdef EKTIFF_USE_NAMESPACE
#define EKTIFF_USING_STD namespace std { }; using namespace std;
#else
#define EKTIFF_USING_STD
#endif
#endif

#ifdef _WIN32_WCE
#define EKTIFF_USING_STD
#endif

#ifdef _MSC_VER
#if (_MSC_VER < 1100)
#define EKTIFF_USING_STD
#else
#define EKTIFF_USING_STD namespace std { }; using namespace std;
#endif
#endif

#ifndef EKTIFF_USING_STD
#define EKTIFF_USING_STD namespace std { }; using namespace std;
#endif  // default

EKTIFF_USING_STD


/////////////////////////
// TYPES
/////////////////////////
#ifdef WIN32
typedef void* HINTERNET;
#endif

/*
 * Intrinsic data types required by the file format:
 *
 * 8-bit quantities	int8/uint8
 * 16-bit quantities	int16/uint16
 * 32-bit quantities	tiff_int32/tiff_uint32
 * strings		unsigned char*
 */
typedef	char int8;
typedef	unsigned char uint8;
typedef	short int16;
typedef	unsigned short uint16;	/* sizeof (uint16) must == 2 */
typedef	int tiff_int32;
typedef	unsigned int tiff_uint32;	/* sizeof (tiff_uint32) must == 4 */


/*
 * The following typedefs define the intrinsic size of
 * data types used in the *exported* interfaces.  These
 * definitions depend on the proper definition of types
 * in tiff.h.  Note also that the varargs interface used
 * pass tag types and values uses the types defined in
 * tiff.h directly.
 *
 * NB: ttag_t is unsigned int and not unsigned short because
 *     ANSI C requires that the type before the ellipsis be a
 *     promoted type (i.e. one of int, unsigned int, pointer,
 *     or double) and because we defined pseudo-tags that are
 *     outside the range of legal Aldus-assigned tags.
 * NB: tsize_t is tiff_int32 and not tiff_uint32 because some functions
 *     return -1.
 * NB: toff_t is not off_t for many reasons; TIFFs max out at
 *     32-bit file offsets being the most important
 */
typedef	tiff_uint32 ttag_t;	    /* directory tag */
typedef	uint16 ttype_t;		/* directory type */
typedef	uint16 tdir_t;		/* directory index */
typedef	uint16 tsample_t;	/* sample number */
typedef	tiff_uint32 tstrip_t;	/* strip number */
typedef tiff_uint32 ttile_t;		/* tile number */
typedef	tiff_int32 tsize_t;		/* i/o size in bytes */

#if (defined _MSC_VER)
#if (!defined _WINDOWS_) && (!defined __AFXWIN_H__)
#include <windows.h>
#endif
#ifdef WIN32
DECLARE_HANDLE(ekthandle_t);	/* Win32 file handle */
/* Get the POSIX access persissions defined */
#if !defined( S_IRUSR)
#define S_IRUSR _S_IREAD
#endif
#if !defined( S_IWUSR )
#define S_IWUSR _S_IWRITE
#endif
#if !defined( S_IXUSR )
#define S_IXUSR _S_IEXEC
#endif
#else
typedef	HFILE ekthandle_t;	/* client data handle */
#endif
#else
	typedef	int ekthandle_t;	/* client data handle */
#endif

typedef	void* tdata_t;		/* image data ref */
typedef	unsigned long toff_t;		/* file offset */

/*
 * These type sizes were necessary as some operating systems have
 * different type sizes than the TIFF spec specifies
 * (i.e., sizeof (toff_t) may be 8 or 4 )
 */
#define SIZEOF_TOFF_T 4
#define SIZEOF_UINT16 2
#define SIZEOF_UINT32 4

/*
 * Typedefs for ``method pointers'' used internally.
 */
typedef	unsigned char tidataval_t;	/* internal image data value type */
typedef	tidataval_t* tidata_t;		/* reference to internal image data */


typedef	enum {
	EKTIFF_NOTYPE	= 0,	    /* placeholder */
	EKTIFF_UBYTE	= 1,	    /* 8-bit unsigned integer */
	EKTIFF_ASCII	= 2,	    /* 8-bit bytes w/ last byte null */
	EKTIFF_USHORT	= 3,	    /* 16-bit unsigned integer */
	EKTIFF_ULONG	= 4,	    /* 32-bit unsigned integer */
	EKTIFF_URATIONAL	= 5,	/* 64-bit unsigned fraction */
	EKTIFF_BYTE	= 6,	    /* 8-bit signed integer */
	EKTIFF_UNDEFINED	= 7,	/* 8-bit untyped data */
	EKTIFF_SHORT	= 8,	    /* 16-bit signed integer */
	EKTIFF_LONG	= 9,	    /* 32-bit signed integer */
	EKTIFF_RATIONAL	= 10,	/* 64-bit signed fraction */
	EKTIFF_FLOAT	= 11,	    /* 32-bit IEEE floating point */
	EKTIFF_DOUBLE	= 12	    /* 64-bit IEEE floating point */
} TiffDataType;

#if (defined _MSC_VER) && (defined WIN32) 
#pragma pack( push,2 )
#endif

typedef	struct
{
        uint16  tag;	
        uint16  type;	
        tiff_uint32  count;	
        tiff_uint32  offset;
} TiffDirEntry;


typedef	struct 
{
	uint16	magic;	    // magic number (defines byte order)
	uint16	version;	// TIFF version number
	tiff_uint32	diroff;	    // byte offset to first directory
} TiffHeader;

#if (defined _MSC_VER) && (defined WIN32) 
#pragma pack( pop )
#endif


#endif // _TIFF_TYPEDEFS_H

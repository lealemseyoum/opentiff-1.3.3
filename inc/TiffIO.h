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


#ifndef _TIFFIO_H
#define	_TIFFIO_H

#include "TiffTypeDefs.h"
#include <string>

// Notify the compiler that we're using the standard namespace here
using namespace std;


class TiffIO ;

class TiffHandleIO
{
    public:
        TiffHandleIO( TiffIO& theParent)
            : parent( theParent ) {}
        virtual ~TiffHandleIO( ) {}

        virtual tsize_t read( tdata_t buf, tsize_t size ) = 0 ;
        virtual tsize_t write( tdata_t buf, tsize_t size ) = 0 ;
        virtual toff_t seek( toff_t off, int whence ) = 0 ;
        virtual toff_t size() = 0;
        virtual void close() = 0;

    protected:
        TiffIO& parent ;

};


class TiffIO
{
    public:
        TiffIO( ekthandle_t theFileHandle, const string& filename, tiff_uint32 imode ) ;
	TiffIO(void* imgBuf, tsize_t bufSize, tiff_uint32 imode );
#ifdef WIN32
        TiffIO( HINTERNET hInet, const string& filename, tiff_uint32 imode) ;
#endif  
        ~TiffIO( void ) { close(); }
        
        static TiffIO* open( const char* filename, const char* mode ) ;
        static tiff_uint32 convertMode( const char* cmode ) ;


        // A lot of classes use this method, so leave
        // it here even if memory mapping is disabled.
        // It returns the correct result in either case.
        bool isMapped() const
            { 
#if ENABLE_MEM_MAP
				return (_flags & TIFF_MAPPED)?true:false ;
#else
				return false;
#endif
			}
        void memMapFile( toff_t& mapSize ) ;
        void memUnMapFile( void ) ;
        tidata_t mapBase();


        tiff_uint32& flags( void ) 
            { return _flags ; }
        tiff_uint32& mode( void )
            { return _mode ; }
        
        string& name() 
            { return _name; }
        
        toff_t& lastNextOffset() 
            { return _lastNextOffset; }
        
        ekthandle_t& getFileHandle( void )
            {return fileHandle ; }
        
        tsize_t read( tdata_t buf, tsize_t size )
            { return handleIO->read( buf, size ) ; }
        tsize_t write( tdata_t buf, tsize_t size )
            { return handleIO->write( buf, size ) ; }
        toff_t seek( toff_t off, int whence )
            { return handleIO->seek( off, whence ) ; }
        
        void close( void ) ;
        toff_t size( void ) ;
        
        bool bigEndian( void ) const
            { return _bigEndian ; }
        void bigEndian( bool state )
            { _bigEndian = state ;  return ; }
        
        bool doSwab( void ) const
            { return (_flags & TIFF_SWAB)?true:false ; }
        
        void swabShort(uint16* wp) ;
        void swabLong(tiff_uint32* lp) ;
        void swabArrayOfShort(uint16* wp, tiff_uint32 n) ;
        void swabArrayOfLong(tiff_uint32* lp, tiff_uint32 n) ;
        void swabDouble(double *dp) ;
        void swabArrayOfDouble(double* dp, tiff_uint32 n) ;
        
    private:
        TiffHandleIO *handleIO ;
    
        tiff_uint32  _flags ;           // was tif->tif_flags
        string  _name ;            // was tif->tif_name
        tiff_uint32  _mode ;            // was tif->tif_mode
        bool    _bigEndian ;

        ekthandle_t  fileHandle ;      // was tif->tif_fd, clientdata

	toff_t _lastNextOffset;	// offset to the last unwritten "Offset to the next dir"
};



// ****** KODAK - OLD STUFF WAITING TO BE RE-WORKED *******

/*
 * RGBA-style image support.
 */
// typedef	unsigned char TIFFRGBValue;		/* 8-bit samples */
// typedef struct _TIFFRGBAImage TIFFRGBAImage;
/*
 * The image reading and conversion routines invoke
 * ``put routines'' to copy/image/whatever tiles of
 * raw image data.  A default set of routines are 
 * provided to convert/copy raw image data to 8-bit
 * packed ABGR format rasters.  Applications can supply
 * alternate routines that unpack the data into a
 * different format or, for example, unpack the data
 * and draw the unpacked raster on the display.
 */
//  typedef void (*tileContigRoutine)
//      (TIFFRGBAImage*, tiff_uint32*, tiff_uint32, tiff_uint32, tiff_uint32, tiff_uint32, tiff_int32, tiff_int32,
//  	unsigned char*);
//  typedef void (*tileSeparateRoutine)
//      (TIFFRGBAImage*, tiff_uint32*, tiff_uint32, tiff_uint32, tiff_uint32, tiff_uint32, tiff_int32, tiff_int32,
//  	unsigned char*, unsigned char*, unsigned char*, unsigned char*);
/*
 * RGBA-reader state.
 */

//  typedef struct {				/* YCbCr->RGB support */
//  	TIFFRGBValue* clamptab;			/* range clamping table */
//  	int*	Cr_r_tab;
//  	int*	Cb_b_tab;
//  	tiff_int32*	Cr_g_tab;
//  	tiff_int32*	Cb_g_tab;
//  	float	coeffs[3];			/* cached for repeated use */
//  } TIFFYCbCrToRGB;

//  struct _TIFFRGBAImage {
//  	TIFF*	tif;				/* image handle */
//  	int	stoponerr;			/* stop on read error */
//  	int	isContig;			/* data is packed/separate */
//  	int	alpha;				/* type of alpha data present */
//  	tiff_uint32	width;				/* image width */
//  	tiff_uint32	height;				/* image height */
//  	uint16	bitspersample;			/* image bits/sample */
//  	uint16	samplesperpixel;		/* image samples/pixel */
//  	uint16	orientation;			/* image orientation */
//  	uint16	photometric;			/* image photometric interp */
//  	uint16*	redcmap;			/* colormap pallete */
//  	uint16*	greencmap;
//  	uint16*	bluecmap;
//  						/* get image data routine */
//  	int	(*get)(TIFFRGBAImage*, tiff_uint32*, tiff_uint32, tiff_uint32);
//  	union {
//  	    void (*any)(TIFFRGBAImage*);
//  	    tileContigRoutine	contig;
//  	    tileSeparateRoutine	separate;
//  	} put;					/* put decoded strip/tile */
//  	TIFFRGBValue* Map;			/* sample mapping array */
//  	tiff_uint32** BWmap;				/* black&white map */
//  	tiff_uint32** PALmap;			/* palette image map */
//  	TIFFYCbCrToRGB* ycbcr;			/* YCbCr conversion state */
//  };

/*
 * Macros for extracting components from the
 * packed ABGR form returned by TIFFReadRGBAImage.
 */
//  #define	TIFFGetR(abgr)	((abgr) & 0xff)
//  #define	TIFFGetG(abgr)	(((abgr) >> 8) & 0xff)
//  #define	TIFFGetB(abgr)	(((abgr) >> 16) & 0xff)
//  #define	TIFFGetA(abgr)	(((abgr) >> 24) & 0xff)

/*
 * A CODEC is a software package that implements decoding,
 * encoding, or decoding+encoding of a compression algorithm.
 * The library provides a collection of builtin codecs.
 * More codecs may be registered through calls to the library
 * and/or the builtin implementations may be overridden.
 */
//  typedef	int (*TIFFInitMethod)(TIFF*, int);
//  typedef struct {
//  	char*		name;
//  	uint16		scheme;
//  	TIFFInitMethod	init;
//  } TIFFCodec;

#endif // _TIFFIO_H

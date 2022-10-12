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


#ifndef _TIFFCODEC_H
#define _TIFFCODEC_H

#include "TiffDirectory.h"
#include "TiffError.h"

class TiffImage;

class TiffCodec
{
    friend class TiffImage;
    friend class TiffStripImage;
    friend class TiffTileImage;
        
        
    public:
        virtual ~TiffCodec();

	// --- virtual methods
	virtual int init(TiffDirectory* tdir, TiffIO* tio, TiffImage* img) = 0;
	virtual int setupDecode() = 0;
	virtual int setupEncode() = 0;
	virtual int preDecode(tsample_t s) = 0;
	virtual int preEncode(tsample_t s) = 0;
	virtual int postEncode() = 0;
	virtual void postDecode(tidata_t buf, tsize_t cc);
	virtual int decodeRow(tidata_t buf, tsize_t cc, tsample_t s) = 0; 
	virtual int encodeRow(tidata_t buf, tsize_t cc, tsample_t s) = 0; 
	virtual int decodeStrip(tidata_t buf, tsize_t cc, tsample_t s) = 0; 
	virtual int encodeStrip(tidata_t buf, tsize_t cc, tsample_t s) = 0; 
	virtual int decodeTile(tidata_t buf, tsize_t cc, tsample_t s) = 0; 
	virtual int encodeTile(tidata_t buf, tsize_t cc, tsample_t s) = 0;
        void reverseBits( void ) ;
        void reverseBits( register tiff_uint32 nBytes ) ;
        
	virtual void close() {}
	virtual int seek(tiff_uint32 nrows)
	{
            string m = "Compression algorithm does not support random access";
            throw TiffError(TIFF_ERROR, m, EKTIFF_WHERE);
	}
	virtual void cleanup() = 0;
	virtual tiff_uint32 defStripSize(tiff_uint32 s)
	{
            return defaultStripSize(s);
	}
	virtual void defTileSize(tiff_uint32* tw, tiff_uint32* th)
	{
            defaultTileSize(tw, th);
	}


protected:
	TiffCodec()
            :tiffdir(0), tif_row((tiff_uint32)-1), tif_data(0), tif_rawdata(0),
             tif_rawdatasize(0), tif_rawcp(0), tif_rawcc(0)
            { }

	tiff_uint32 defaultStripSize(tiff_uint32 s);
	void defaultTileSize(tiff_uint32 *tw, tiff_uint32 *th);

	tsize_t scanlineSize();
	tsize_t tileRowSize();	

	// --- data members
	TiffDirectory*	tiffdir;
	TiffIO*			tiffio;
	TiffImage*		tiffimg;

	tiff_uint32		tif_row;	/* current scanline */

	tidata_t	tif_data;	// compression scheme private data.
	tidata_t	tif_rawdata;	/* raw data buffer */
	tsize_t		tif_rawdatasize;/* # of bytes in raw data buffer */
	tidata_t	tif_rawcp;	/* current spot in raw buffer */
	tsize_t		tif_rawcc;	/* bytes unread from raw buffer */


};

#endif


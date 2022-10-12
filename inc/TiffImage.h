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


#ifndef _TIFFIMAGE_H
#define _TIFFIMAGE_H

//! header="TiffImageFile.h"

#include "TiffConf.h"
#include "TiffTypeDefs.h"
#include "TiffTagDefs.h"

#define	BUFFERCHECK					\
	((codec->tif_rawdata) ||		\
	    writeBufferSetup(NULL, (tsize_t) -1))

class TiffDirectory;
class TiffIO;
class TiffCodec;


//:
// The TiffImage class is the base class for TiffStripImage and TiffTileImage.
// Its constructor is protected so it can't be instantiated directly by the users.
//
class EKTIFF_DECL TiffImage
{
    public:
	// Destructor
	virtual ~TiffImage();
        
	// Initialize the TiffImage object. This method is used by the library internally       
	// and not intended to be called by the users.
	//!return: non-zero if fail; otherwise zero.
	int init();

	// Check if the image is a tiled image.
	//!return: True if the image is a tiled image; otherwise false
	bool isTiled()	{ return bTiled; }

	// Flush out the image data from internal buffer to the file. The method must be
	// called after finishing writing the image data.
	void flush();

	// Flush out the image data. This method is used by the library internally
	// and not intended to be called by the users.
	//!return: non-zero if successful; otherwise zero.
	int flushData1();

	// Flush out the image data. This method is used by the library internally
	// and not intended to be called by the users.
	//!return: non-zero if successful; otherwise zero.
	int flushData();

	// todo: 
	// 1. interface to read a block of image data without knowing if
	// the image is tiled. Q: format of the data returned
	// 2. interface to write the whole image (not support for update so
	// can't only write a portion of the image	

    // check if need to swap the data buffer before writing image data out.
    bool needSwapForWriting();

    protected:
	//!i: data members
	TiffDirectory*	tiffdir;
	TiffIO*	        tiffio;
	TiffCodec*      codec;

	tstrip_t        stripsperimage; // tilesperimage if TiffImage is tiled
	tstrip_t        nstrips;        // size of the strip/tile offset array, can grow

	bool	bCoderSetup;
	bool	bPostEncode;

	// Constructor.
	TiffImage(TiffDirectory* tdir, TiffIO* tio, bool tile);

	// Setup strip/tile offsets
	int setupOffsets();

	// Init scanlinesize or tilesize
	virtual void initSize() = 0;

	// Get scanlinesize or tilesize
	virtual tsize_t getSize() = 0; 

	int writeCheck(int tiles);
	int writeBufferSetup(tdata_t bp, tsize_t size);

	// Append to strip/tile
	int appendToStrip(tstrip_t strip, tidata_t data, tsize_t cc);

	// Grow strips/tiles
	int growStrips(int delta);

	//!i: methods for reading
	int checkRead(int tiles);
	int readBufferSetup(tdata_t bp, tsize_t size);



    private:
	bool	bTiled;
};

#endif // _TIFFIMAGE_H



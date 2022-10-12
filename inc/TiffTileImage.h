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

#ifndef _TIFFTILEIMAGE_H
#define _TIFFTILEIMAGE_H

//! header="TiffImageFile.h"

#include "TiffImage.h"

//:
// This class subclasses TiffImage to provide interface to handle tile-based image. 
// <p>
// Example:
// <pre>
//  #include "TiffImageFile.h"
//  #include "TiffDirectory.h"
//  #include "TiffTileImage.h"
// 
//  int main()
//  {
//    TiffImageFile inImgFile;
//    inImgFile.open("test.tif", "r");
//    TiffImageFile outImgFile;
//    outImgFile.open("out.tif", "w");
//    TiffDirectory* indir = inImgFile.getDirectory(0);
//    TiffDirectory* outdir = outImgFile.getDirectory(0);
//    
//    outdir->copy(*indir)
//
//    TiffImage* inImg = indir->getImage();
//    TiffImage* outImg = outdir->getImage(); 
//    if (inImg && outImg && inImg->isTiled())
//    {
//      int size = inImg->tileSize();
//      tdata_t buf = new unsigned char[size];
//      tstrip_t numTiles = inImg->numOfTiles();
//      for (int i=0; i < numTiles; i++)
//      {
//        size = inImg->readRawTile(i, buf, -1);
//        outImg->writeRawTile(i, buf, size);
//      }
//
//      outImg->flush();
//    }
//
//    inImgFile.close();
//    outImgFile.close();
//  }
// </pre>
//
class EKTIFF_DECL TiffTileImage : public TiffImage
{
    public:

	// Create a TiffTileImage object. This constructor is used by the libray internally
	// and not intended to be called by the users.
	//!param: tdir - Pointer to the IFD object containing this image.
	//!param: tio - Pointer to the TiffIO object.
	TiffTileImage(TiffDirectory* tdir, TiffIO* tio)
            : TiffImage(tdir, tio, true)
            { }

        ~TiffTileImage() {}

	// Read a tile to the user-supplied buffer.
	//!param: tile - The tile number.
	//!param: buf - Data buffer.
	//!param: size - The count of data in byte.
	//!return: The number of bytes read.
	tsize_t readRawTile(ttile_t tile, tdata_t buf, tsize_t size);

	// Write the supplied data to the specified tile.
	//!param: tile - The tile number.
	//!param: data - Data buffer.
	//!param: cc - The count of data in byte.
	//!return: The number of bytes wrote.
	tsize_t writeRawTile(ttile_t tile, tdata_t data, tsize_t cc);

	// Read a tile of data and decompress the specified amount into the user-supplied
	// buffer
	//!param: tile - The tile number.
	//!param: buf - Data buffer.
	//!param: size - The count of data in byte.
	//!return: The number of bytes read.
	tsize_t readEncodedTile(ttile_t tile, tdata_t buf, tsize_t size);

	// Encode the supplied data and write it to the specified tile.
	//!param: tile - The tile number.
	//!param: data - Data buffer.
	//!param: cc - The count of data in byte.
	//!return: The number of bytes wrote.
	tsize_t writeEncodedTile(ttile_t tile, tdata_t data, tsize_t cc);

	// Compute which tile a (x, y, z, s) value is in.
	//!param: x - X coordinate.
	//!param: y - Y coordinate.
	//!param: z - Z coordinate.
	//!param: s - Sample number.
	//!return: Tile number.
	ttile_t computeTile(tiff_uint32 x, tiff_uint32 y, tiff_uint32 z, tsample_t s) const;

	// Compute the number of tiles in the image
	//!return: The number of tiles
	ttile_t numOfTiles();

	// Compute the size of a row for a tile in byte
	//!return: Size in byte
	tsize_t tileRowSize();	
	
	// Compute the number of bytes in a variable length, row-aligned tile. 
	//!return: size in byte
	tsize_t vTileSize(tiff_uint32 nrows);
	
	// Compute the number of bytes in a row-aligned tile.
	//!return: size in byte
	tsize_t tileSize();

	// Retrieve the current tile number.
	//!return: Tile number
	ttile_t currentTile() { return curtile; }

    protected:

	virtual void initSize() { tilesize = tileSize(); }
	virtual tsize_t getSize() { return tilesize; }

	int startTile(ttile_t tile);
	tsize_t readRawTile1(ttile_t tile, tdata_t buf, tsize_t size);
	int fillTile(ttile_t tile);

    private:
	tsize_t		tilesize;
	tiff_uint32 		tif_col;	// was tif_col, current column (offset by row too) 
	ttile_t		curtile;	// current tile for read/write 

};

#endif // _TIFFTILEIMAGE_H

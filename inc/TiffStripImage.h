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

#ifndef _TIFFSTRIPIMAGE_H
#define _TIFFSTRIPIMAGE_H

//! header="TiffImageFile.h"

#include "TiffImage.h"
#include "TiffDirectory.h"

//:
// This class subclasses TiffImage to provide interface to handle strip-based image. 
// <p>
// Example:
// <pre>
//  #include "TiffImageFile.h"
//  #include "TiffDirectory.h"
//  #include "TiffStripImage.h"
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
//    if (inImg && outImg && !inImg->isTiled())
//    {
//      int size = inImg->scanlineSize();
//      tdata_t buf = new unsigned char[size];
//      tiff_uint32 length = indir->imageLength();
//
//      for (int row=0; row < length; row++)
//      {
//        inImg->readScanline(buf, row);
//        outImg->writeScanline(buf, row);
//      }
//      outImg->flush();
//
//      delete (unsigned char*)buf;
//    }
//
//    inImgFile.close();
//    outImgFile.close();
//  }
// </pre>
//
class EKTIFF_DECL TiffStripImage : public TiffImage
{
    public:

	// Create a TiffStripImage object. This constructor is used by the libray internally
	// and not intended to be called by the users.
	//!param: tdir - Pointer to the IFD object containing this image.
	//!param: tio - Pointer to the TiffIO object.
	TiffStripImage(TiffDirectory* tdir, TiffIO* tio)
    : TiffImage(tdir, tio, false)
    {
        stripsperimage = tdir->numOfStrips();
        scanlinesize = tdir->scanlineSize();
        curstrip = (tstrip_t)-1;
    }

    ~TiffStripImage() {}
    
	// Write the supplied data to the specified scanline.
	//!param: buf - Data buffer.
	//!param: row - The row number of the scanline.
	//!param: sample - Indicate which band of data.
	//!return: 1 if succeed; otherwise zero.
	int writeScanline(tdata_t buf, tiff_uint32 row, tsample_t sample = 0);

	// Read data from the specified scanline.
	//!param: buf - Data buffer.
	//!param: row - The row number of the scanline.
	//!param: sample - Indicate which band of data.
	//!return: 1 if succeed; otherwise zero.
	int readScanline(tdata_t buf, tiff_uint32 row, tsample_t sample = 0);

	// Encode the supplied data and write it to the specified strip.
	//!param: strip - Strip number.
	//!param: data - Data buffer.
	//!param: cc - The count of of data in byte.
	//!return: The number of bytes wrote.
	tsize_t writeEncodedStrip(tstrip_t strip, tdata_t data, tsize_t cc);

	// Read a strip of data and decompress the specified amount into the user-supplied
	// buffer.
	//!param: strip - Strip number.
	//!param: data - Data buffer.
	//!param: cc - The count of data in byte.
	//!return: The number of bytes read.
	tsize_t readEncodedStrip(tstrip_t strip, tdata_t buf, tsize_t size);

	// Write the supplied data to the specified strip.
	//!param: strip - Strip number.
	//!param: data - Data buffer.
	//!param: cc - The count of data in byte.
	//!return: The number of bytes wrote.
	tsize_t writeRawStrip(tstrip_t strip, tdata_t data, tsize_t cc);

	// Read a strip to the user-supplied buffer.
	//!param: strip - Strip number.
	//!param: data - Data buffer.
	//!param: cc - The count of data in byte.
	//!return: The number of bytes read.
	tsize_t readRawStrip(tstrip_t strip, tdata_t buf, tsize_t size);

	// Compute which strip a (row, sample) value is in.
	//!param: row - Row number.
	//!param: sample - Sample number.
	tstrip_t computeStrip(tiff_uint32 row, tsample_t sample) const;

	// Compute how many strips in an image.
	//!return: The number of strips.
	tstrip_t numOfStrips();

	// Compute the size of scanline in byte.
	//!return: The size of scanline in byte.
	tsize_t scanlineSize();

	// Retieve the current strip number.
	//!return: Strip number.
	tstrip_t currentStrip() { return curstrip; }

	// Compute the number of bytes in a variable height, row-aligned strip.
	//!return: Size in bytes
	tsize_t vStripSize(tiff_uint32 nrows);

	// Compute the number of bytes in a (row-aligned) strip.
	//!return: Size in bytes
	tsize_t stripSize();

    protected:

	virtual void initSize() { scanlinesize = scanlineSize(); }
	virtual tsize_t getSize() { return scanlinesize; }

	int seek(tiff_uint32 row, tsample_t sample);
	tsize_t readRawStrip1(tstrip_t strip, tdata_t buf, tsize_t size);
	int fillStrip(tstrip_t strip);	
	int startStrip(tstrip_t strip);

    private:
	tsize_t		scanlinesize;
	tstrip_t	curstrip;	/* current strip for read/write */

};

#endif // _TIFFSTRIPIMAGE_H

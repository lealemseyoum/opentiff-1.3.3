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


#ifndef _TIFFFILE_H
#define _TIFFFILE_H

//! header="TiffImageFile.h"

#include "TiffImageFile.h"
#include <map>

typedef enum
{
    FMT_ULTRATIFF,	// The UltraTiff format of DC290
    FMT_DCS			// The format for DCS315, 330, 420 460 520 560 620 660
} TiffFormat;

typedef enum
{
    CIN_PRIMARYIMAGE,	// Full resolution image
    CIN_THUMBNAIL,
    CIN_SCREENNAIL,
    CIN_EXIF,
    CIN_SOUND
} CommonIFDName;


class FormatMapWrapper;

//:
// This is a wrapper class for TiffImageFile. It provides a simple means of accessing
// metadata in a TIFF knowledge layer. In this layer. the library establishes the 
// mapping from the common IFD name to the specific IFD tag for the supported TIFF 
// formats. The users specify the TIFF foramt to handle and use the common IFD name 
// to access the metadata. The interface of this class is easy to use and greatly 
// relieve the average users from knowing the TIFF IFD structure. 
// <p>
// Example:
// <pre>
//   #include "TiffFile.h"
//   #include "TiffTagEntry.h"
//
//   int main()
//   {
//     TiffTagEntry* entry;
//     TiffFile tFile;
//     tFile.open("DC290.tif", "r", FMT_ULTRATIFF);
//
//     // Get image width of full res image
//     entry = tFile.getTag(TIFFTAG_IMAGEWIDTH, CIN_PRIMARYIMAGE);
//     // Get image width of thumbnail
//     entry = tFile.getTag(TIFFTAG_IMAGEWIDTH, CIN_THUMBNAIL);
//     ...
//     tFile.close();
//   }
// </pre>
//
class EKTIFF_DECL TiffFile
{
    public:
	typedef map<CommonIFDName, TiffImageFile::DirPath>  DirMap;
	typedef DirMap::iterator                            DirMapIter;
	typedef DirMap::const_iterator                      ConstDirMapIter;
	typedef map<TiffFormat, DirMap*>                    FormatMap;
	typedef FormatMap::iterator                         FormatMapIter;
	typedef FormatMap::const_iterator                   ConstFormatMapIter;

    public:

	// Constructor.
	TiffFile();

	// Destructor.
	~TiffFile() { close(); }

	// Open an image.
	//!param: name - File name
	//!param: cmode - Open mode
	//!param: fmt - Tiff format
	void open(const char* name, const char* cmode, TiffFormat fmt);

	// Open an image with the specified handle.
	//!param: hFile - Handle of image file.
	//!param: name - File name
	//!param: cmode - Open mode
	//!param: fmt - Tiff format
	void open(ekthandle_t hFile, const char* name, const char* cmode, TiffFormat fmt);

	// Retrieve tag entry.
	//!param: tag - Tag number.
	//!param: cname - Common IFD name.
	//!return: Pointer to the tag entry if the tag is found; otherwise NULL.
	TiffTagEntry* getTag(ttag_t tag, CommonIFDName cname);

	// Set tag entry.
	//!param: entry - Tag entry.
	//!param: cname - Common IFD name.
	void setTag(const TiffTagEntry& entry, CommonIFDName cname);

	// Get the directory with the specified name
	//!param: cname - Common IFD name.
	//!return: Pointer to the IFD if the IFD is found; otherwise NULL.
	TiffDirectory* getDirectory(CommonIFDName cname);

	// Get the image file object wrapped.
	//!return: Pointer to the image file object.
	TiffImageFile*	getImageFile() { return &imgFile; }

	// Close the file.
	void close();

    protected:

	void initRegistry();

	DirMap*				    format;
	static FormatMapWrapper	registry;
	TiffImageFile		    imgFile;
};


// wrapper of FormatMap so the DirMap can be freed when program exits.
class FormatMapWrapper 
{
    public:
        FormatMapWrapper() {};
        ~FormatMapWrapper();

        TiffFile::FormatMap map;
};

#endif // _TIFFFILE_H

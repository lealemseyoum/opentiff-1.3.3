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


#include "TiffFile.h"
#include "TiffTagDefs.h"
#include "TiffError.h"

FormatMapWrapper TiffFile::registry;


TiffFile::TiffFile(): format(0)
{
    if (registry.map.empty())
    {
        initRegistry();
    }
}


void TiffFile::initRegistry()
{
#define DirPathType TiffImageFile::DirPath::value_type

    TiffImageFile::DirPath path;

    // ------ create dir map for UltraTiff ------

    DirMap* dirMap = new DirMap();

    // dir info for Primary Image
    path.clear();
    path.push_back(DirPathType(0, 0));
    (*dirMap).insert(DirMap::value_type(CIN_PRIMARYIMAGE, path));

    // dir info for Thumbnail
    path.clear();
    path.push_back(DirPathType(0, 0));
    path.push_back(DirPathType(TIFFTAG_SUBIFD, 0));
    (*dirMap).insert(DirMap::value_type(CIN_THUMBNAIL, path));

    // dir info for Screennail
    path.clear();
    path.push_back(DirPathType(0, 0));
    path.push_back(DirPathType(TIFFTAG_SUBIFD, 1));
    (*dirMap).insert(DirMap::value_type(CIN_SCREENNAIL, path));

    // dir info for ExifIFD
    path.clear();
    path.push_back(DirPathType(0, 0));
    path.push_back(DirPathType(TIFFTAG_EXIFIFD, 0));
    (*dirMap).insert(DirMap::value_type(CIN_EXIF, path));

    // dir info for SoundIFD
    path.clear();
    path.push_back(DirPathType(0, 0));
    path.push_back(DirPathType(TIFFTAG_SOUNDIFD, 0));
    (*dirMap).insert(DirMap::value_type(CIN_SOUND, path));

    registry.map.insert(FormatMap::value_type(FMT_ULTRATIFF, dirMap));

    // ------ Create dir map for TIFF/EP ------
    dirMap = new DirMap();

    // dir info for Primary Image
    path.clear();
    path.push_back(DirPathType(0, 0));
    path.push_back(DirPathType(TIFFTAG_SUBIFD, 1));
    (*dirMap).insert(DirMap::value_type(CIN_PRIMARYIMAGE, path));

    // dir info for Thumbnail
    path.clear();
    path.push_back(DirPathType(0, 0));
    (*dirMap).insert(DirMap::value_type(CIN_THUMBNAIL, path));

    registry.map.insert(FormatMap::value_type(FMT_DCS, dirMap));

}


void TiffFile::open(const char* name, const char* cmode, TiffFormat fmt)
{
    FormatMapIter it = registry.map.find(fmt);
    if (it != registry.map.end())
    {
        format = (*it).second;
    }
    else
    {
        string m = "TiffFile::open -- un-supported TIFF format.";
        throw TiffError(TIFF_ERROR, m, EKTIFF_WHERE);
    }

    close();
	
    imgFile.open(name, cmode);
}


void TiffFile::open(ekthandle_t hFile, const char* name, const char* cmode, TiffFormat fmt)
{
    FormatMapIter it = registry.map.find(fmt);
    if (it != registry.map.end())
    {
        format = (*it).second;
    }
    else
    {
        string m = "TiffFile::open -- un-supported TIFF format.";
        throw TiffError(TIFF_ERROR, m, EKTIFF_WHERE);
    }

    close();
	
    imgFile.open(hFile, name, cmode);
}


TiffTagEntry* TiffFile::getTag(ttag_t tag, CommonIFDName cname)
{
    DirMapIter it = format->find(cname);
    if (it != format->end())
    {
        return (imgFile.getGenericTag(tag, (*it).second));
    }
    else
        return 0;
}


void TiffFile::setTag(const TiffTagEntry& entry, CommonIFDName cname)
{
    DirMapIter it = format->find(cname);
    if (it != format->end())
    {
        imgFile.setGenericTag(entry, (*it).second);
    }
}


void TiffFile::close()
{
    imgFile.close();
}


FormatMapWrapper::~FormatMapWrapper()
{
    for (TiffFile::FormatMapIter iter = map.begin(); iter != map.end(); iter++)
        delete (*iter).second;
}


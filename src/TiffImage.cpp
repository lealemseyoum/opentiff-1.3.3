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


#include "TiffImage.h"
#include "TiffDirectory.h"
#include "TiffError.h"
#include "TiffStripImage.h"
#include "TiffTileImage.h"
#include "TiffCodec.h"
#include "TiffCodecNone.h"
#include "TiffCodecJpeg.h"
#include "TiffCodecPackBits.h"


TiffImage::TiffImage(TiffDirectory* tdir, TiffIO* tio, bool tile)
 : tiffdir(tdir), tiffio(tio), codec(0), nstrips(0),
   bCoderSetup(false), bPostEncode(false), bTiled(tile)
{

}


TiffImage::~TiffImage()
{
    if (codec)
        delete codec;
}


int TiffImage::init()
{
    if (isTiled())
    {
        stripsperimage = tiffdir->getGenericTag(TIFFTAG_TILELENGTH) ?
            (dynamic_cast<TiffTileImage*>(this))->numOfTiles() : 
            tiffdir->samplesPerPixel();
    }
    else
    {
        stripsperimage = tiffdir->getGenericTag(TIFFTAG_ROWSPERSTRIP) ?
            (dynamic_cast<TiffStripImage*>(this))->numOfStrips() : 
            tiffdir->samplesPerPixel();
    }

    nstrips = stripsperimage;
    if (tiffdir->planarConfig() == PLANARCONFIG_SEPARATE)
        stripsperimage /= tiffdir->samplesPerPixel();
    
    // todo: add more codecs
    switch (tiffdir->compression())
    {
	case COMPRESSION_NONE:
            codec = new TiffCodecNone();
            break;
#ifdef JPEG_SUPPORT
	case COMPRESSION_OJPEG:
	case COMPRESSION_JPEG:
            codec = new TiffCodecJpeg();
            break;
#endif  
#ifdef PACKBITS_SUPPORT
	case COMPRESSION_PACKBITS:
            codec = new TiffCodecPackBits();
            break;
#endif  

	default:
		{
			char m[256];
			sprintf(m, "%s: Unsupported tiff compression in file ;", tiffio->name().c_str());
			throw TiffError(TIFF_ERROR, m, EKTIFF_WHERE);
		}
    }
    
    if (!codec->init(tiffdir, tiffio, this))
	{
		char m[256];
		sprintf(m, "%s: Problem intializing codec in;", tiffio->name().c_str());
		throw TiffError(TIFF_ERROR, m, EKTIFF_WHERE);

	}
     
    initSize();
    
    return 1;	
}



int TiffImage::setupOffsets()
{
	// setup stripoffsets
    if ( nstrips > 1 )
    {
        TiffTagEntry* off = isTiled()?
            new TiffTagEntryT<vector<tiff_uint32> >(TIFFTAG_TILEOFFSETS, EKTIFF_ULONG, nstrips, vector<tiff_uint32>(nstrips, 0)):
            new TiffTagEntryT<vector<tiff_uint32> >(TIFFTAG_STRIPOFFSETS, EKTIFF_ULONG, nstrips, vector<tiff_uint32>(nstrips, 0));
        tiffdir->setGenericTag(*off);
        delete off;
    
        // setup stripbytecounts
        TiffTagEntry* cnt = isTiled()?
            new TiffTagEntryT<vector<tiff_uint32> >(TIFFTAG_TILEBYTECOUNTS, EKTIFF_ULONG, nstrips, vector<tiff_uint32>(nstrips, 0)):
            new TiffTagEntryT<vector<tiff_uint32> >(TIFFTAG_STRIPBYTECOUNTS, EKTIFF_ULONG, nstrips, vector<tiff_uint32>(nstrips, 0));
        tiffdir->setGenericTag(*cnt);
        delete cnt;
    }
    else
    {
        TiffTagEntry* off = isTiled()?
            new TiffTagEntryT<tiff_uint32>(TIFFTAG_TILEOFFSETS, EKTIFF_ULONG, nstrips, 0):
            new TiffTagEntryT<tiff_uint32>(TIFFTAG_STRIPOFFSETS, EKTIFF_ULONG, nstrips, 0);
        tiffdir->setGenericTag(*off);
        delete off;
    
        // setup stripbytecounts
        TiffTagEntry* cnt = isTiled()?
            new TiffTagEntryT<tiff_uint32>(TIFFTAG_TILEBYTECOUNTS, EKTIFF_ULONG, nstrips, 0):
            new TiffTagEntryT<tiff_uint32>(TIFFTAG_STRIPBYTECOUNTS, EKTIFF_ULONG, nstrips, 0);
        tiffdir->setGenericTag(*cnt);
        delete cnt;
    }

    return (1);
}


int TiffImage::writeCheck(int tiles)
{
    if (tiffio->mode() == O_RDONLY) 
    {
        char m[256];
        sprintf(m, "%s: File not open for writing", tiffio->name().c_str());
        throw TiffError(TIFF_ERROR, m, EKTIFF_WHERE);
    }
    /*
     * On the first write verify all the required information
     * has been setup and initialize any data structures that
     * had to wait until directory information was set.
     * Note that a lot of our work is assumed to remain valid
     * because we disallow any of the important parameters
     * from changing after we start writing (i.e. once
     * TIFF_BEENWRITING is set, TIFFSetField will only allow
     * the image's length to be changed).
     */
    if (tiffdir->getGenericTag(TIFFTAG_IMAGEWIDTH) == NULL) 
    {
        char m[256];
        sprintf(m, "%s: Must set \"ImageWidth\" before writing data", tiffio->name().c_str());
        throw TiffError(TIFF_MISSING_IMAGEWIDTH, m, EKTIFF_WHERE);
    }
    if (tiffdir->getGenericTag(TIFFTAG_PLANARCONFIG) == NULL) 
    {
        char m[256];
        sprintf(m, "%s: Must set \"PlanarConfiguration\" before writing data", tiffio->name().c_str());
        throw TiffError(TIFF_MISSING_PLANARCFG, m, EKTIFF_WHERE);
    }
    
    TiffTagEntry* p =  tiles ?
        tiffdir->getGenericTag(TIFFTAG_TILEOFFSETS) :
        tiffdir->getGenericTag(TIFFTAG_STRIPOFFSETS);
     
    if (p != 0) 
    {
        if (isTiled()) 
        {
            stripsperimage = tiffdir->getGenericTag(TIFFTAG_TILELENGTH) ?
                (dynamic_cast<TiffTileImage*>(this))->numOfTiles() : 
                tiffdir->samplesPerPixel();
        }
        else 
        {
            stripsperimage = tiffdir->getGenericTag(TIFFTAG_ROWSPERSTRIP) ?
                (dynamic_cast<TiffStripImage*>(this))->numOfStrips() : 
                tiffdir->samplesPerPixel();
        }
        nstrips = stripsperimage;
        if (tiffdir->planarConfig() == PLANARCONFIG_SEPARATE)
            stripsperimage /= tiffdir->samplesPerPixel();
        
        vector<tiff_uint32> bytecounts;
        
        bytecounts.clear();
        for (tiff_uint32 i=0; i<nstrips; i++)
            bytecounts.push_back(0);

        if ( nstrips > 1 )
        {
            TiffTagEntry* cnt = isTiled()?
                new TiffTagEntryT<vector<tiff_uint32> >(TIFFTAG_TILEBYTECOUNTS, EKTIFF_ULONG, nstrips, bytecounts ):
                new TiffTagEntryT<vector<tiff_uint32> >(TIFFTAG_STRIPBYTECOUNTS, EKTIFF_ULONG, nstrips, bytecounts );
            tiffdir->setGenericTag(*cnt);
            delete cnt;
        }
        else
        {
            TiffTagEntry* cnt = isTiled()?
                new TiffTagEntryT<tiff_uint32>(TIFFTAG_TILEBYTECOUNTS, EKTIFF_ULONG, nstrips, bytecounts.front() ):
                new TiffTagEntryT<tiff_uint32>(TIFFTAG_STRIPBYTECOUNTS, EKTIFF_ULONG, nstrips, bytecounts.front() );
            tiffdir->setGenericTag(*cnt);
            delete cnt;
        }
    }
    else 
    {
        if (!setupOffsets()) 
        {
            char m[256];
            sprintf(m, "%s: No space for strip/tile arrays", tiffio->name().c_str());
            throw TiffError(TIFF_MEM_ALLOC_FAIL, m, EKTIFF_WHERE);
        }
    }
    
    initSize();
    
    tiffio->flags() |= TIFF_BEENWRITING;
    return (1);
}


int TiffImage::writeBufferSetup(tdata_t bp, tsize_t size)
{
    if (codec->tif_rawdata) 
    {
        if (tiffio->flags() & TIFF_MYBUFFER) 
        {
            free(codec->tif_rawdata);
            tiffio->flags() &= ~TIFF_MYBUFFER;
        }
        codec->tif_rawdata = NULL;
    }
    if (size == (tsize_t) -1) 
    {
        size = getSize();
        if (size < 16*1024)
            size = 16*1024;
        bp = NULL; // force a malloc
    }
    if (bp == NULL) 
    {
        bp = malloc(size);
        if (bp == NULL) 
        {
            char m[256];
            sprintf(m, "%s: No space for output buffer", tiffio->name().c_str());
			throw TiffError(TIFF_MEM_ALLOC_FAIL, m, EKTIFF_WHERE);
        }

        tiffio->flags() |= TIFF_MYBUFFER;
    }
    else
        tiffio->flags() &= ~TIFF_MYBUFFER;

    codec->tif_rawdata = (tidata_t) bp;
    codec->tif_rawdatasize = size;
    codec->tif_rawcc = 0;
    codec->tif_rawcp = codec->tif_rawdata;

    return (1);
}


int TiffImage::appendToStrip(tstrip_t strip, tidata_t data, tsize_t cc)
{
    vector<tiff_uint32> offset = isTiled() ? 
        tiffdir->tileOffsets() :
        tiffdir->stripOffsets();
    TiffTagEntry* off = isTiled() ? tiffdir->getGenericTag( TIFFTAG_TILEOFFSETS ) : 
        tiffdir->getGenericTag( TIFFTAG_STRIPOFFSETS );
    assert(offset.size());

    vector<tiff_uint32> bytecount = isTiled() ?
        tiffdir->tileByteCounts() :
        tiffdir->stripByteCounts();
    TiffTagEntry* cnt = isTiled() ? tiffdir->getGenericTag( TIFFTAG_TILEBYTECOUNTS ) : 
        tiffdir->getGenericTag( TIFFTAG_STRIPBYTECOUNTS );

    tiff_uint32 stripPtr = offset[strip] + bytecount[strip];
    /*
     * No current offset, set the current strip.
     */
    if (offset[strip] != 0) {
        if (tiffio->seek(stripPtr, SEEK_SET )!= stripPtr ) 
        {
            char m[256];
            sprintf(m, "%s: Seek error at scanline %lu", tiffio->name().c_str(), codec->tif_row);
            throw TiffError(TIFF_ERROR, m, EKTIFF_WHERE);
        }
    } 
    else
        offset[strip] = tiffio->seek((toff_t) 0, SEEK_END);


    if (tiffio->write(data, cc) != cc) 
    {
        char m[256];
        sprintf(m, "%s: Write error at scanline %lu", tiffio->name().c_str(), codec->tif_row);
        throw TiffError(TIFF_ERROR, m, EKTIFF_WHERE);
    }
    bytecount[strip] += cc;

    if( off->getCount() > 1 )
    {
        TiffTagEntryT< vector<tiff_uint32> >* temp = (TiffTagEntryT< vector<tiff_uint32> >*)off;
        temp->setValue(offset);
    }
    else
    {
        TiffTagEntryT<tiff_uint32>* temp = (TiffTagEntryT<tiff_uint32>*)off;
        temp->setValue(offset.front()) ;
    }
    if( cnt->getCount() > 1 )
    {
        TiffTagEntryT< vector<tiff_uint32> >* temp = (TiffTagEntryT< vector<tiff_uint32> >*)cnt;
        temp->setValue(bytecount);
    }
    else
    {
        TiffTagEntryT<tiff_uint32>* temp = (TiffTagEntryT<tiff_uint32>*)cnt;
        temp->setValue(bytecount.front()) ;
    }
    return (1);
}


int TiffImage::growStrips(int delta)
{

    assert(tiffdir->planarConfig() == PLANARCONFIG_CONTIG);
    vector<tiff_uint32> offset = isTiled() ? tiffdir->tileOffsets() : tiffdir->stripOffsets();
    vector<tiff_uint32> bytecount = isTiled() ? tiffdir->tileByteCounts() : tiffdir->stripByteCounts();
    TiffTagEntry* off = isTiled() ? tiffdir->getGenericTag( TIFFTAG_TILEOFFSETS ) : 
        tiffdir->getGenericTag( TIFFTAG_STRIPOFFSETS );
    TiffTagEntry* cnt = isTiled() ? tiffdir->getGenericTag( TIFFTAG_TILEBYTECOUNTS ) : 
        tiffdir->getGenericTag( TIFFTAG_STRIPBYTECOUNTS );

    for (int i=0; i<delta; i++) 
    {
        offset.push_back(0);
        bytecount.push_back(0);	
    }    
    nstrips += delta;

    if( off->getCount() > 1 )
    {
        TiffTagEntryT< vector<tiff_uint32> >* temp = (TiffTagEntryT< vector<tiff_uint32> >*)off;
        temp->setValue( offset );
        temp->setCount( nstrips );
    }
    else if (off->getCount() == 1 && delta > 0)
    {
        TiffTagEntryT< vector<tiff_uint32> >* temp = isTiled() ?
            new TiffTagEntryT< vector<tiff_uint32> >(TIFFTAG_TILEOFFSETS, EKTIFF_ULONG, nstrips, offset):
            new TiffTagEntryT< vector<tiff_uint32> >(TIFFTAG_STRIPOFFSETS, EKTIFF_ULONG, nstrips, offset);
        tiffdir->setGenericTag( *temp );
        delete temp;
    }
    if( cnt->getCount() > 1 )
    {
        TiffTagEntryT< vector<tiff_uint32> >* temp = (TiffTagEntryT< vector<tiff_uint32> >*)cnt;
        temp->setValue( bytecount );
        temp->setCount( nstrips );
    }
    else if (off->getCount() == 1 && delta > 0)
    {
        TiffTagEntryT< vector<tiff_uint32> >* temp = isTiled() ?
            new TiffTagEntryT< vector<tiff_uint32> >(TIFFTAG_TILEBYTECOUNTS, EKTIFF_ULONG, nstrips, offset):
            new TiffTagEntryT< vector<tiff_uint32> >(TIFFTAG_STRIPBYTECOUNTS, EKTIFF_ULONG, nstrips, offset);
        tiffdir->setGenericTag( *temp );
        delete temp;
    }

    return (1);
}


int TiffImage::flushData1()
{
    if (codec->tif_rawcc > 0) 
    {
        if (!tiffdir->isFillOrder(tiffdir->fillOrder()) &&
            (tiffio->flags() & TIFF_NOBITREV) == 0)
        {
            codec->reverseBits();
        }

        tstrip_t curstrip= tiffdir->isTiled() ? 
            (dynamic_cast<TiffTileImage*>(this))->currentTile() :
            (dynamic_cast<TiffStripImage*>(this))->currentStrip();

        if (!appendToStrip(curstrip, codec->tif_rawdata, codec->tif_rawcc))
            return (0);

        codec->tif_rawcc = 0;
        codec->tif_rawcp = codec->tif_rawdata;
    }

    return (1);
}


void TiffImage::flush()
{
    if (!flushData())
    {
        throw TiffError(TIFF_ERROR, "Failed to flush out image data.", EKTIFF_WHERE);
    }
    
    tiffio->flags() &= ~TIFF_BEENWRITING;
}


int TiffImage::flushData()
{
    if ((tiffio->flags() & TIFF_BEENWRITING) == 0)
        return (0);
    if (bPostEncode) 
    {
        bPostEncode = false;
        if (!codec->postEncode())
            return (0);
    }
	// DAG MOD
//	if ((tif->tif_flags & TIFF_DIRTYDIRECT) &&
//		    !TIFFWriteDirectory(tif))
//			return (0);
//		tif->tif_flags &= ~TIFF_DIRTYDIRECT;

    return (flushData1());
}


// Setup the raw data buffer in preparation for
// reading a strip of raw data.  If the buffer
// is specified as zero, then a buffer of appropriate
// size is allocated by the library.  Otherwise,
// the client must guarantee that the buffer is
// large enough to hold any individual strip of
// raw data.
int TiffImage::readBufferSetup(tdata_t bp, tsize_t size)
{
    if (codec->tif_rawdata) 
    {
        if (tiffio->flags() & TIFF_MYBUFFER)
            free(codec->tif_rawdata);

        codec->tif_rawdata = NULL;
    }

    if (bp) 
    {
        codec->tif_rawdatasize = size;
        codec->tif_rawdata = (tidata_t) bp;
        tiffio->flags() &= ~TIFF_MYBUFFER;
    } 
    else 
    {
        codec->tif_rawdatasize = TIFFroundup(size, 1024);
        codec->tif_rawdata = (tidata_t) malloc(codec->tif_rawdatasize);
        tiffio->flags() |= TIFF_MYBUFFER;
    }

    if (codec->tif_rawdata == NULL) 
    {
        char m[256];
        sprintf(m, "%s: No space for data buffer at scanline %ld", tiffio->name().c_str(), codec->tif_row);
        throw TiffError(TIFF_MEM_ALLOC_FAIL, m, EKTIFF_WHERE);
    }

    return (1);
}




int TiffImage::checkRead(int tiles)
{
    if (tiffio->mode() == O_WRONLY) 
    {
        string m = tiffio->name();
        m += ": File not open for reading";
        throw TiffError(TIFF_ERROR, m, EKTIFF_WHERE);
    }

    if (tiles ^ (int)isTiled()) 
    {
        string m = tiffio->name();
        m += tiles ?
            ": Can not read tiles from a stripped image":
            ": Can not read scanlines from a tiled image";
        throw TiffError(TIFF_ERROR, m, EKTIFF_WHERE);
    }

    return (1);
}


bool TiffImage::needSwapForWriting()
{
    return tiffio->doSwab();
}

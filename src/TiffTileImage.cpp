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

#include "TiffConf.h"
#include "TiffTileImage.h"
#include "TiffCodec.h"
#include "TiffDirectory.h"
#include "TiffError.h"

#define	NOTILE	((ttile_t) -1)			/* undefined state */

#define	WRITECHECKTILES				\
	((tiffio->flags() & TIFF_BEENWRITING) || writeCheck(1))


/*
 * Read a tile of data from the file.
 */
tsize_t TiffTileImage::readRawTile(ttile_t tile, tdata_t buf, tsize_t size)
{
    tsize_t bytecount;
    
    if (!checkRead(1))
        return ((tsize_t) -1);

    if (tile >= nstrips)
    {
        char m[256];
        sprintf(m, "%lu: Tile out of range, max %lu", 
                (tiff_int32) tile, (tiff_int32) nstrips);
        throw TiffError(TIFF_ERROR, m, EKTIFF_WHERE);
    }

    vector<tiff_uint32> tilebytecount = tiffdir->tileByteCounts();
    bytecount = tilebytecount[tile];

    if (size != (tsize_t) -1 && size < bytecount)
        bytecount = size;

    return (readRawTile1(tile, buf, bytecount));
}


tsize_t TiffTileImage::writeRawTile(ttile_t tile, tdata_t data, tsize_t cc)
{
    if (!WRITECHECKTILES)
        return ((tsize_t) -1);
    if (tile >= nstrips) 
    {
        char m[256];
        sprintf(m, "%s: Tile %lu out of range, max %lu",
                tiffio->name().c_str(), (tiff_int32) tile, (tiff_uint32) nstrips);
        throw TiffError(TIFF_ERROR, m, EKTIFF_WHERE);
    }
    return (appendToStrip(tile, (tidata_t) data, cc) ? cc : (tsize_t) -1);
}


tsize_t TiffTileImage::readEncodedTile(ttile_t tile, tdata_t buf, tsize_t size)
{
    if (!checkRead(1))
        return (0);

    if (tile >= nstrips) 
    {
        char msg[100];
        sprintf(msg, "%ld: Tile out of range, max %ld", (tiff_int32) tile, (tiff_uint32) nstrips);
        throw TiffError(TIFF_ERROR, msg, EKTIFF_WHERE);
    }

    if (size == (tsize_t) 0)
        size = tilesize;
    else 
        if (size > tilesize)
            size = tilesize;

    if (fillTile(tile) && codec->decodeTile((tidata_t) buf, size, 
                                            (tsample_t)(tile/stripsperimage))) 
    {
        codec->postDecode((tidata_t) buf, size);
        return (size);
    }
    else
        return (0);
}


tsize_t TiffTileImage::writeEncodedTile(ttile_t tile, tdata_t data, tsize_t cc)
{
    tsample_t sample;

    if (!WRITECHECKTILES)
        return ((tsize_t) -1);

    if (tile >= nstrips) 
    {
        char msg[100];
        sprintf(msg, "%ld: Tile out of range, max %ld", (tiff_int32) tile, (tiff_uint32) nstrips);
        throw TiffError(TIFF_ERROR, msg, EKTIFF_WHERE);
    }

    /*
     * Handle delayed allocation of data buffer.  This
     * permits it to be sized more intelligently (using
     * directory information).
     */
    if (!BUFFERCHECK)
        return ((tsize_t) -1);

    curtile = tile;
    /* 
     * Compute tiles per row & per column to compute
     * current row and column
     */
    codec->tif_row = (tile % TIFFhowmany(tiffdir->imageLength(), tiffdir->tileLength()))
        * tiffdir->tileLength();
    tif_col = (tile % TIFFhowmany(tiffdir->imageWidth(), tiffdir->tileWidth()))
        * tiffdir->tileWidth();

    if (!bCoderSetup) 
    {
        if (!codec->setupEncode())
            return ((tsize_t) -1);
        bCoderSetup = true;
    }

    bPostEncode = false;
    sample = (tsample_t)(tile/stripsperimage);
    if (!codec->preEncode(sample))
        return ((tsize_t) -1);
    /*
     * Clamp write amount to the tile size.  This is mostly
     * done so that callers can pass in some large number
     * (e.g. -1) and have the tile size used instead.
     */
    if ( cc > tilesize)
        cc = tilesize;

    if (!codec->encodeTile((tidata_t) data, cc, sample))
        return ((tsize_t) 0);

    if (!codec->postEncode())
        return ((tsize_t) -1);

    if (!tiffdir->isFillOrder(tiffdir->fillOrder()) &&
        (tiffio->flags() & TIFF_NOBITREV) == 0)
        codec->reverseBits();

    if (codec->tif_rawcc > 0 && 
        !appendToStrip(tile, codec->tif_rawdata, codec->tif_rawcc))
        return ((tsize_t) -1);

    codec->tif_rawcc = 0;
    codec->tif_rawcp = codec->tif_rawdata;

    return (cc);
}


ttile_t TiffTileImage::computeTile(tiff_uint32 x, tiff_uint32 y, tiff_uint32 z, tsample_t s) const
{
    tiff_uint32 dx = tiffdir->tileWidth();
    tiff_uint32 dy = tiffdir->tileLength();
    tiff_uint32 dz = tiffdir->tileDepth();
    ttile_t tile = 1;

    if (tiffdir->imageDepth() == 1)
        z = 0;

    if (dx == 0)
        dx = tiffdir->imageWidth();

    if (dy == 0)
        dy = tiffdir->imageLength();

    if (dz == 0)
        dz = tiffdir->imageDepth();

    if (dx != 0 && dy != 0 && dz != 0) 
    {
        tiff_uint32 xpt = TIFFhowmany(tiffdir->imageWidth(), dx); 
        tiff_uint32 ypt = TIFFhowmany(tiffdir->imageLength(), dy); 
        tiff_uint32 zpt = TIFFhowmany(tiffdir->imageDepth(), dz); 
        
        if (tiffdir->planarConfig() == PLANARCONFIG_SEPARATE) 
            tile = (xpt*ypt*zpt)*s +
                (xpt*ypt)*(z/dz) +
                xpt*(y/dy) +
                x/dx;
        else
            tile = (xpt*ypt)*(z/dz) + xpt*(y/dy) + x/dx + s;
    }
    return (tile);
}


ttile_t TiffTileImage::numOfTiles()
{
    return tiffdir->numOfTiles();
}


tsize_t TiffTileImage::tileRowSize()
{
    return tiffdir->tileRowSize();
}


tsize_t TiffTileImage::vTileSize(tiff_uint32 nrows)
{
    return tiffdir->vTileSize(nrows);
}


tsize_t TiffTileImage::tileSize()
{
    return (vTileSize(tiffdir->tileLength()));
}


// Set state to appear as if a
// tile has just been read in.
int TiffTileImage::startTile(ttile_t tile)
{
    if (!bCoderSetup) 
    {
        if (!codec->setupDecode())
            return (0);
        bCoderSetup = true;
    }

    curtile = tile;
    codec->tif_row = 
        (tile % TIFFhowmany(tiffdir->imageWidth(), tiffdir->tileWidth())) *
        tiffdir->tileLength();
    tif_col =
        (tile % TIFFhowmany(tiffdir->imageLength(), tiffdir->tileLength())) *
        tiffdir->tileWidth();
    codec->tif_rawcp = codec->tif_rawdata;
    vector<tiff_uint32> tilebytecount = tiffdir->tileByteCounts() ;
    codec->tif_rawcc = tilebytecount[tile];
    return (codec->preDecode((tsample_t)(tile/stripsperimage)));
}


// Read the specified tile and setup for decoding. 
// The data buffer is expanded, as necessary, to
// hold the tile's data.
int TiffTileImage::fillTile(ttile_t tile)
{
    tsize_t bytecount;

    vector<tiff_uint32> tilebytecount = tiffdir->tileByteCounts();
    bytecount = tilebytecount[tile];
    if (bytecount <= 0) 
    {
        char m[256];
        sprintf(m, "%lu: Invalid tile byte count, tile %lu", 
                (tiff_uint32) bytecount, (tiff_int32) tile);
        throw TiffError(TIFF_INVALID_BYTECOUNTS, m, EKTIFF_WHERE);
    }

    if (tiffio->isMapped() && (tiffdir->isFillOrder(tiffdir->fillOrder()) ||
                               (tiffio->flags() & TIFF_NOBITREV))) 
    {
        /*
         * The image is mapped into memory and we either don't
         * need to flip bits or the compression routine is going
         * to handle this operation itself.  In this case, avoid
         * copying the raw data and instead just reference the
         * data from the memory mapped file image.  This assumes
         * that the decompression routines do not modify the
         * contents of the raw data buffer (if they try to,
         * the application will get a fault since the file is
         * mapped read-only).
         */
        if ((tiffio->flags() & TIFF_MYBUFFER) && codec->tif_rawdata)
            free(codec->tif_rawdata);

        tiffio->flags() &= ~TIFF_MYBUFFER;
        vector<tiff_uint32> tileoffset = tiffdir->tileOffsets();

        if (tileoffset[tile] + bytecount > (tiff_uint32)tiffio->size()) 
        {
            curtile = NOTILE;
            return (0);
        }

        codec->tif_rawdatasize = bytecount;
        codec->tif_rawdata = tiffio->mapBase() + tileoffset[tile];
    } 
    else 
    {
        /*
         * Expand raw data buffer, if needed, to
         * hold data tile coming from file
         * (perhaps should set upper bound on
         *  the size of a buffer we'll use?).
         */
        if (bytecount > codec->tif_rawdatasize) 
        {
            curtile = NOTILE;
            if ((tiffio->flags() & TIFF_MYBUFFER) == 0) 
            {
                char m[256];
                sprintf(m, "%s: Data buffer too small to hold tile %ld",
                        tiffio->name().c_str(), (tiff_int32) tile);
                throw TiffError(TIFF_ERROR, m, EKTIFF_WHERE);
            }

            if (!readBufferSetup(0, TIFFroundup(bytecount, 1024)))
                return (0);

        }
        if (readRawTile1(tile, (uint8 *)codec->tif_rawdata, bytecount) != bytecount)
            return (0);

        if (!tiffdir->isFillOrder(tiffdir->fillOrder()) &&
            (tiffio->flags() & TIFF_NOBITREV) == 0)
            codec->reverseBits(bytecount);

    }
    return (startTile(tile));
}


tsize_t TiffTileImage::readRawTile1(ttile_t tile, tdata_t buf, tsize_t size)
{
    vector<tiff_uint32> tileoffset = tiffdir->tileOffsets();

    if (tiffio->seek(tileoffset[tile], SEEK_SET) != tileoffset[tile]) 
    {
        char m[256];
        sprintf(m, "%s: Seek error at row %ld, col %ld, tile %ld",
                tiffio->name().c_str(), codec->tif_row, tif_col, tile);
        throw TiffError(TIFF_IO_SEEK, m, EKTIFF_WHERE);
    }
    if (tiffio->read(buf, size) != size) 
    {
        char m[256];
        sprintf(m, "%s: Read error at row %ld, col %ld",
                tiffio->name().c_str(), codec->tif_row, tif_col);
        throw TiffError(TIFF_IO_READ, m, EKTIFF_WHERE);
    }

    return (size);
}


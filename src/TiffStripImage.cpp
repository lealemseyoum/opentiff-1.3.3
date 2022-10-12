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
#include "TiffStripImage.h"
#include "TiffDirectory.h"
#include "TiffIO.h"
#include "TiffError.h"
#include "TiffCodec.h"

#define	NOSTRIP	((tstrip_t) -1)			/* undefined state */

#define	WRITECHECKSTRIPS				\
	((tiffio->flags() & TIFF_BEENWRITING) || writeCheck(0))


int TiffStripImage::writeScanline(tdata_t buf, tiff_uint32 row, tsample_t sample)
{
    int status, imagegrew = 0;
    tstrip_t strip;

    if (!WRITECHECKSTRIPS)
        return (-1);
    /*
     * Handle delayed allocation of data buffer.  This
     * permits it to be sized more intelligently (using
     * directory information).
     */
    if (!BUFFERCHECK)
        return (-1);

    /*
     * Extend image length if needed
     * (but only for PlanarConfig=1).
     */
    if (row >= tiffdir->imageLength()) {	/* extend image */
        if (tiffdir->planarConfig() == PLANARCONFIG_SEPARATE) 
        {
            throw TiffError(TIFF_ERROR, "Can not change \"ImageLength\" when using separate planes", EKTIFF_WHERE);
        }
        tiffdir->imageLength() = row+1;
        imagegrew = 1;
    }
    /*
     * Calculate strip and check for crossings.
     */
    if (tiffdir->planarConfig() == PLANARCONFIG_SEPARATE) 
    {
        if (sample >= tiffdir->samplesPerPixel()) 
        {
            char m[256];
            sprintf(m, "%d: Sample out of range, max %d", sample, tiffdir->samplesPerPixel());
            throw TiffError(TIFF_ERROR, m, EKTIFF_WHERE);
        }
        strip = sample*stripsperimage + row/tiffdir->rowsPerStrip();
    } 
    else
        strip = row / tiffdir->rowsPerStrip();

    if (strip != curstrip) 
    {
        /*
         * Changing strips -- flush any data present.
         */
        if (!flushData())
            return (-1);
        curstrip = strip;
        /*
         * Watch out for a growing image.  The value of
         * strips/image will initially be 1 (since it
         * can't be deduced until the imagelength is known).
         */
        if (strip >= stripsperimage && imagegrew)
            stripsperimage =
                TIFFhowmany(tiffdir->imageLength(),tiffdir->rowsPerStrip());
        codec->tif_row =
            (strip % stripsperimage) * tiffdir->rowsPerStrip();
        if (!bCoderSetup) 
        {
            if (!codec->setupEncode())
                return (-1);
            bCoderSetup = true;
        }
        if (!codec->preEncode(sample))
            return (-1);
        bPostEncode = true;
    }
    /*
     * Check strip array to make sure there's space.
     * We don't support dynamically growing files that
     * have data organized in separate bitplanes because
     * it's too painful.  In that case we require that
     * the imagelength be set properly before the first
     * write (so that the strips array will be fully
     * allocated above).
     */
    // todo: make sure that is not needed
    if (strip >= nstrips && !growStrips(1))
        return (-1);
    /*
     * Ensure the write is either sequential or at the
     * beginning of a strip (or that we can randomly
     * access the data -- i.e. no encoding).
     */
    if (row != codec->tif_row) 
    {
        if (row < codec->tif_row) 
        {
            /*
             * Moving backwards within the same strip:
             * backup to the start and then decode
             * forward (below).
             */
            codec->tif_row = (strip % stripsperimage) * tiffdir->rowsPerStrip();
            codec->tif_rawcp = codec->tif_rawdata;
        }
        /*
         * Seek forward to the desired row.
         */
        if (!codec->seek(row - codec->tif_row))
            return (-1);
        codec->tif_row = row;
    }
    status = codec->encodeRow((tidata_t) buf, scanlinesize, sample);
    codec->tif_row++;
    return (status);
}



int TiffStripImage::readScanline(tdata_t buf, tiff_uint32 row, tsample_t sample)
{
    int e;

    if (!checkRead(0))
        return (-1);
    if ((e = seek(row, sample))) 
    {
        /*
         * Decompress desired row into user buffer.
         */
        e = codec->decodeRow((tidata_t) buf, scanlinesize, sample);
        codec->tif_row++;
        if (e)
            codec->postDecode((tidata_t) buf, scanlinesize);
    }
    return (e ? 1 : -1);
}


/*
 * Encode the supplied data and write it to the
 * specified strip.  There must be space for the
 * data; we don't check if strips overlap!
 *
 * NB: Image length must be setup before writing.
 */
tsize_t TiffStripImage::writeEncodedStrip(tstrip_t strip, tdata_t data, tsize_t cc)
{
    tsample_t sample;

    if (!WRITECHECKSTRIPS)
        return ( 0);
    /*
     * Check strip array to make sure there's space.
     * We don't support dynamically growing files that
     * have data organized in separate bitplanes because
     * it's too painful.  In that case we require that
     * the imagelength be set properly before the first
     * write (so that the strips array will be fully
     * allocated above).
     */
    if (strip >= nstrips) 
    {
        if (tiffdir->planarConfig() == PLANARCONFIG_SEPARATE) 
        {
            throw TiffError(TIFF_ERROR, "Can not grow image by strips when using separate planes", EKTIFF_WHERE);
        }
        if (!growStrips(1))
            return ( 0);
        stripsperimage =
            TIFFhowmany(tiffdir->imageLength(), tiffdir->rowsPerStrip());
    }
    /*
     * Handle delayed allocation of data buffer.  This
     * permits it to be sized according to the directory
     * info.
     */
    if (!BUFFERCHECK)
        return ( 0);
    curstrip = strip;
    codec->tif_row = (strip % stripsperimage) * tiffdir->rowsPerStrip();
    if (!bCoderSetup) 
    {
        if (!codec->setupEncode())
            return ( 0);
        bCoderSetup = true;
    }
    bPostEncode = false;
    sample = (tsample_t)(strip / stripsperimage);
    if (!codec->preEncode(sample))
        return ( 0);
    if (!codec->encodeStrip((tidata_t) data, cc, sample))
        return ( 0);
    if (!codec->postEncode())
        return (0);
    if (!tiffdir->isFillOrder(tiffdir->fillOrder()) &&
        (tiffio->flags() & TIFF_NOBITREV) == 0)
        codec->reverseBits();
    if (codec->tif_rawcc > 0 &&
        !appendToStrip(strip, codec->tif_rawdata, codec->tif_rawcc))
        return (0);
    codec->tif_rawcc = 0;
    codec->tif_rawcp = codec->tif_rawdata;
    return (cc);
}


tsize_t TiffStripImage::readEncodedStrip(tstrip_t strip, tdata_t buf, tsize_t size)
{
    tiff_uint32 nrows;
    tsize_t stripsize;

    if (!checkRead(0))
        return (0);
    if (strip >= nstrips) 
    {
        char msg[100];
        sprintf(msg, "%ld: Strip out of range, max %ld", strip, nstrips);
        throw TiffError(TIFF_ERROR, msg, EKTIFF_WHERE);
    }
    /*
     * Calculate the strip size according to the number of
     * rows in the strip (check for truncated last strip).
     */
    if (strip != nstrips-1 ||
        (nrows = tiffdir->imageLength() % tiffdir->rowsPerStrip()) == 0)
        nrows = tiffdir->rowsPerStrip();
    stripsize = vStripSize(nrows);
    if (size == (tsize_t) 0)
        size = stripsize;
    else if (size > stripsize)
        size = stripsize;
    if (fillStrip(strip) && codec->decodeStrip(
        (tidata_t) buf, size, (tsample_t)(strip / stripsperimage))) 
    {
        codec->postDecode((tidata_t) buf, size);
        return (size);
    } else
        return ((tsize_t) 0);
}


/*
 * Write the supplied data to the specified strip.
 * There must be space for the data; we don't check
 * if strips overlap!
 *
 * NB: Image length must be setup before writing.
 */
tsize_t TiffStripImage::writeRawStrip(tstrip_t strip, tdata_t data, tsize_t cc)
{
    if (!WRITECHECKSTRIPS)
        return ((tsize_t) -1);
    /*
     * Check strip array to make sure there's space.
     * We don't support dynamically growing files that
     * have data organized in separate bitplanes because
     * it's too painful.  In that case we require that
     * the imagelength be set properly before the first
     * write (so that the strips array will be fully
     * allocated above).
     */
    if (strip >= nstrips) 
    {
        if (tiffdir->planarConfig() == PLANARCONFIG_SEPARATE) 
        {
            throw TiffError(TIFF_ERROR, "Can not grow image by strips when using separate planes", EKTIFF_WHERE);
        }
        /*
         * Watch out for a growing image.  The value of
         * strips/image will initially be 1 (since it
         * can't be deduced until the imagelength is known).
         */
        if (strip >= stripsperimage)
            stripsperimage =
                TIFFhowmany(tiffdir->imageLength(),tiffdir->rowsPerStrip());
        if (!growStrips(1))
            return ((tsize_t) -1);
    }
    curstrip = strip;
    codec->tif_row = (strip % stripsperimage) * tiffdir->rowsPerStrip();
    return (appendToStrip(strip, (tidata_t) data, cc) ?
	    cc : (tsize_t) -1);
}


tsize_t TiffStripImage::readRawStrip(tstrip_t strip, tdata_t buf, tsize_t size)
{
    tsize_t bytecount;

    if (!checkRead(0))
        return ((tsize_t) -1);
    if (strip >= nstrips) 
    {
        char msg[100];
        sprintf(msg, "%ld: Strip out of range, max %ld", strip, nstrips);
        throw TiffError(TIFF_ERROR, msg, EKTIFF_WHERE);
    }
    vector<tiff_uint32> stripbytecount = tiffdir->stripByteCounts();
    bytecount = stripbytecount[strip];
    if (bytecount <= 0) {
        char msg[100];
        sprintf(msg, "%lu: Invalid strip byte count, strip %lu", (tiff_uint32) bytecount, (tiff_uint32) strip);
        throw TiffError(TIFF_ERROR, msg, EKTIFF_WHERE);
    }
    if (size != (tsize_t)-1 && size < bytecount)
        bytecount = size;
    return (readRawStrip1(strip, buf, bytecount));
}


tstrip_t TiffStripImage::computeStrip(tiff_uint32 row, tsample_t sample) const
{
    tstrip_t strip;

    strip = row / tiffdir->rowsPerStrip();
    if (tiffdir->planarConfig() == PLANARCONFIG_SEPARATE) 
    {
        if (sample >= tiffdir->samplesPerPixel()) 
        {
            char msg[100];
            sprintf(msg, "%u, Sample out of range, max %u", sample, tiffdir->samplesPerPixel());
            throw TiffError(TIFF_ERROR, msg, EKTIFF_WHERE);
        }
        strip += sample*stripsperimage;
    }
    return (strip);
}


tstrip_t TiffStripImage::numOfStrips()
{
    return tiffdir->numOfStrips();
}


tsize_t TiffStripImage::scanlineSize()
{
    return tiffdir->scanlineSize();
}


tsize_t TiffStripImage::vStripSize(tiff_uint32 nrows)
{
    return tiffdir->vStripSize(nrows);
}


tsize_t TiffStripImage::stripSize()
{
    tiff_uint32 rps = tiffdir->rowsPerStrip();
    if (rps > tiffdir->imageLength())
        rps = tiffdir->imageLength();
    return (vStripSize(rps));
}


// Set state to appear as if a
// strip has just been read in.
int TiffStripImage::startStrip(tstrip_t strip)
{
    if (!bCoderSetup) 
    {
        if (!codec->setupDecode())
            return (0);
        bCoderSetup = true;
    }
    curstrip = strip;
    codec->tif_row = (strip % stripsperimage) * tiffdir->rowsPerStrip();
    codec->tif_rawcp = codec->tif_rawdata;
    vector<tiff_uint32> stripbytecount = tiffdir->stripByteCounts();
    codec->tif_rawcc = stripbytecount[strip];
    return (codec->preDecode((tsample_t)(strip / stripsperimage)));
}


/*
 * Read the specified strip and setup for decoding. 
 * The data buffer is expanded, as necessary, to
 * hold the strip's data.
 */
int TiffStripImage::fillStrip(tstrip_t strip)
{
    tsize_t bytecount;

    vector<tiff_uint32> stripbytecount = tiffdir->stripByteCounts();
    bytecount = stripbytecount[strip];
    if (bytecount <= 0) 
    {
        char m[256];
        sprintf(m, "%lu: Invalid strip byte count, strip %lu",
                (tiff_uint32) bytecount, (tiff_uint32) strip);
        throw TiffError(TIFF_INVALID_BYTECOUNTS, m, EKTIFF_WHERE);
    }
    if (tiffio->isMapped() &&
        (tiffdir->isFillOrder(tiffdir->fillOrder()) || (tiffio->flags() & TIFF_NOBITREV))) 
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
        vector<tiff_uint32> stripoffset = tiffdir->stripOffsets();
        if (stripoffset[strip] + bytecount > (tiff_uint32)tiffio->size()) 
        {
            /*
             * This error message might seem strange, but it's
             * what would happen if a read were done instead.
             */
            char m[256];
            sprintf(m, "%s: Read error on strip %lu", tiffio->name().c_str(), (tiff_uint32) strip);
            throw TiffError(TIFF_IO_READ, m, EKTIFF_WHERE);
//			tif_curstrip = NOSTRIP;
        }
        codec->tif_rawdatasize = bytecount;               
        codec->tif_rawdata = tiffio->mapBase() + stripoffset[strip];
    } 
    else 
    {
        /*
         * Expand raw data buffer, if needed, to
         * hold data strip coming from file
         * (perhaps should set upper bound on
         *  the size of a buffer we'll use?).
         */
        if (bytecount > codec->tif_rawdatasize) 
        {
            curstrip = NOSTRIP;
            if ((tiffio->flags() & TIFF_MYBUFFER) == 0) 
            {
                char m[256];
                sprintf(m, "%s: Data buffer too small to hold strip %lu",
                        tiffio->name().c_str(), (tiff_uint32) strip);
                throw TiffError(TIFF_ERROR, m, EKTIFF_WHERE);
                return (0);
            }
            if (!readBufferSetup(0, TIFFroundup(bytecount, 1024)))
                return (0);
        }
        if (readRawStrip1(strip, (uint8 *)codec->tif_rawdata, bytecount) != bytecount)
            return (0);
        if (!tiffdir->isFillOrder(tiffdir->fillOrder()) &&
            (tiffio->flags() & TIFF_NOBITREV) == 0)
            codec->reverseBits( bytecount );
    }
    return (startStrip(strip));
}


tsize_t TiffStripImage::readRawStrip1(tstrip_t strip, tdata_t buf, tsize_t size)
{
    vector<tiff_uint32> stripoffset = tiffdir->stripOffsets();
        
    if (tiffio->seek(stripoffset[strip], SEEK_SET) != stripoffset[strip])
    {
        char m[256];
        sprintf(m, "%s: Seek error at scanline %lu, strip %lu",
                tiffio->name().c_str(),(tiff_uint32)codec->tif_row, (tiff_uint32) strip);
        throw TiffError(TIFF_IO_SEEK, m, EKTIFF_WHERE);
    }
    if (tiffio->read(buf, size) != size)
    {
        char m[256];
        sprintf(m, "%s: Read error at scanline %lu",
                tiffio->name().c_str(), (tiff_uint32) codec->tif_row);
        throw TiffError(TIFF_IO_READ, m, EKTIFF_WHERE);
    }

    return (size);
}


int TiffStripImage::seek(tiff_uint32 row, tsample_t sample)
{
    tstrip_t strip;

    if (row >= tiffdir->imageLength()) 
    {	/* out of range */
        char m[256];
        sprintf(m, "%lu: Row out of range, max %lu", (tiff_uint32) row, (tiff_uint32) tiffdir->imageLength());
        throw TiffError(TIFF_ERROR, m, EKTIFF_WHERE);
    }
    if (tiffdir->planarConfig() == PLANARCONFIG_SEPARATE) 
    {
        if (sample >= tiffdir->samplesPerPixel()) 
        {
            char m[256];
            sprintf(m, "%lu: Sample out of range, max %lu",
                    (tiff_uint32) sample, (tiff_uint32) tiffdir->samplesPerPixel());
            throw TiffError(TIFF_ERROR, m, EKTIFF_WHERE);
        }
        strip = sample*stripsperimage + row/tiffdir->rowsPerStrip();
    } 
    else
        strip = row / tiffdir->rowsPerStrip();
    if (strip != curstrip) 
    { 	/* different strip, refill */
        if (!fillStrip(strip))
            return (0);
    } 
    else 
        if (row < codec->tif_row) 
        {
            /*
             * Moving backwards within the same strip: backup
             * to the start and then decode forward (below).
             *
             * NB: If you're planning on lots of random access within a
             * strip, it's better to just read and decode the entire
             * strip, and then access the decoded data in a random fashion.
             */
            if (!startStrip(strip))
                return (0);
	}
    if (row != codec->tif_row) 
    {
        /*
         * Seek forward to the desired row.
         */
        if (!codec->seek(row - codec->tif_row))
            return (0);
        codec->tif_row = row;
    }
    return (1);
}



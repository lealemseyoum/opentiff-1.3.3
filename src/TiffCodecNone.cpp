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


#include "TiffCodecNone.h"
#include "TiffImage.h"

int TiffCodecNone::DumpModeEncode(tidata_t pp, tsize_t cc, tsample_t s)
{
    while (cc > 0) 
    {
        tsize_t n;
        
        n = cc;
        if (tif_rawcc + n > tif_rawdatasize)
            n = tif_rawdatasize - tif_rawcc;
        /*
         * Avoid copy if client has setup raw
         * data buffer to avoid extra copy.
         */
        if (tif_rawcp != pp)
            memcpy(tif_rawcp, pp, n);
        tif_rawcp += n;
        tif_rawcc += n;
        pp += n;
        cc -= n;
        if (tif_rawcc >= tif_rawdatasize && !tiffimg->flushData())
            return (-1);
    }
    return (1);
}

int TiffCodecNone::DumpModeDecode(tidata_t buf, tsize_t cc, tsample_t s)
{
    if (tif_rawcc < cc) 
    {
        char m[256];
        sprintf(m, "DumpModeDecode: Not enough data for scanline %d", tif_row);
        throw TiffError(TIFF_ERROR, m, EKTIFF_WHERE);
    }
    /*
     * Avoid copy if client has setup raw
     * data buffer to avoid extra copy.
     */
    if (tif_rawcp != buf)
        memcpy(buf, tif_rawcp, cc);
    tif_rawcp += cc;
    tif_rawcc -= cc;
    return (1);
}

int TiffCodecNone::DumpModeSeek(tiff_uint32 nrows)
{
    tif_rawcp += nrows * scanlineSize();
    tif_rawcc -= nrows * scanlineSize();
    return (1);
}

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

#ifdef PACKBITS_SUPPORT

#include "TiffCodecPackBits.h"
#include "TiffIO.h"
#include "TiffImage.h"

/*
 * EkTiff Library
 *
 * PackBits Compression Algorithm Support
 */
#include <assert.h>
#include <stdio.h>


int
TiffCodecPackBits::preEncode(tsample_t s)
{
    /*
     * Calculate the scanline/tile-width size in bytes.
     */
    if (tiffdir->isTiled()) 
        mLineSize = tiffdir->tileRowSize();  // TIFFTileRowSize(tif);
    else
        mLineSize = tiffdir->scanlineSize(); // TIFFScanlineSize(tif);
    return (1);
}

/*
 * Encode a run of pixels.
 */

int
TiffCodecPackBits::PackBitsEncode(tidata_t buf, tsize_t cc, tsample_t s)
{
    uint8* bp = (uint8*) buf;
    tidata_t op, ep, lastliteral;
    long n, slop;
    int b;
    enum { BASE, LITERAL, RUN, LITERAL_RUN } state;

    (void) s;
    op = tif_rawcp;
    ep = tif_rawdata + tif_rawdatasize;
    state = BASE;
    lastliteral = 0;
    while (cc > 0) {
        /*
         * Find the longest string of identical bytes.
         */
        b = *bp++, cc--, n = 1;
        for (; cc > 0 && b == *bp; cc--, bp++)
            n++;
    again:
        if (op + 2 >= ep) {        /* insure space for new data */
            /*
             * Be careful about writing the last
             * literal.  Must write up to that point
             * and then copy the remainder to the
             * front of the buffer.
             */
            if (state == LITERAL || state == LITERAL_RUN) {
                slop = op - lastliteral;
                tif_rawcc += lastliteral - tif_rawcp;
                if(!tiffimg->flushData())
                // if (!TIFFFlushData1(tif))
                    return (-1);
                op = tif_rawcp;
                while (slop-- > 0)
                    *op++ = *lastliteral++;
                lastliteral = tif_rawcp;
            } else {
                tif_rawcc += op - tif_rawcp;
                if(!tiffimg->flushData())
            //    if (!TIFFFlushData1(tif))
                    return (-1);
                op = tif_rawcp;
            }
        }
        switch (state) {
        case BASE:        /* initial state, set run/literal */
            if (n > 1) {
                state = RUN;
                if (n > 128) {
                    *op++ = (tidataval_t) -127;
                    *op++ = b;
                    n -= 128;
                    goto again;
                }
                *op++ = (tidataval_t)(-(n-1));
                *op++ = b;
            } else {
                lastliteral = op;
                *op++ = 0;
                *op++ = b;
                state = LITERAL;
            }
            break;
        case LITERAL:        /* last object was literal string */
            if (n > 1) {
                state = LITERAL_RUN;
                if (n > 128) {
                    *op++ = (tidataval_t) -127;
                    *op++ = b;
                    n -= 128;
                    goto again;
                }
                *op++ = (tidataval_t)(-(n-1));    /* encode run */
                *op++ = b;
            } else {            /* extend literal */
                if (++(*lastliteral) == 127)
                    state = BASE;
                *op++ = b;
            }
            break;
        case RUN:        /* last object was run */
            if (n > 1) {
                if (n > 128) {
                    *op++ = (tidataval_t) -127;
                    *op++ = b;
                    n -= 128;
                    goto again;
                }
                *op++ = (tidataval_t)(-(n-1));
                *op++ = b;
            } else {
                lastliteral = op;
                *op++ = 0;
                *op++ = b;
                state = LITERAL;
            }
            break;
        case LITERAL_RUN:    /* literal followed by a run */
            /*
             * Check to see if previous run should
             * be converted to a literal, in which
             * case we convert literal-run-literal
             * to a single literal.
             */
            if (n == 1 && op[-2] == (tidataval_t) -1 &&
                *lastliteral < 126) {
                state = (((*lastliteral) += 2) == 127 ?
                    BASE : LITERAL);
                op[-2] = op[-1];    /* replicate */
            } else
                state = RUN;
            goto again;
        }
    }
    tif_rawcc += op - tif_rawcp;
    tif_rawcp = op;
    return (1);
}

/*
 * Encode a rectangular chunk of pixels.  We break it up
 * into row-sized pieces to insure that encoded runs do
 * not span rows.  Otherwise, there can be problems with
 * the decoder if data is read, for example, by scanlines
 * when it was encoded by strips.
 */
int
TiffCodecPackBits::PackBitsEncodeChunk(tidata_t bp, tsize_t cc, tsample_t s)
{
    tsize_t rowsize = mLineSize;

    assert(rowsize > 0);
    while ((long)cc > 0) {
        if (PackBitsEncode(bp, rowsize, s) < 0)
            return (-1);
        bp += rowsize;
        cc -= rowsize;
    }
    return (1);
}

int
TiffCodecPackBits::PackBitsDecode(tidata_t op, tsize_t occ, tsample_t s)
{
    char *bp;
    tsize_t cc;
    long n;
    int b;

    (void) s;
    bp = (char*) tif_rawcp;
    cc = tif_rawcc;
    while (cc > 0 && (long)occ > 0) {
        n = (long) *bp++, cc--;
        /*
         * Watch out for compilers that
         * don't sign extend chars...
         */
        if (n >= 128)
            n -= 256;
        if (n < 0) {        /* replicate next byte -n+1 times */
            if (n == -128)    /* nop */
                continue;
            n = -n + 1;
            occ -= n;
            b = *bp++, cc--;
            while (n-- > 0)
                *op++ = b;
        } else {        /* copy next n+1 bytes literally */
            memcpy(op, bp, ++n);
            op += n; occ -= n;
            bp += n; cc -= n;
        }
    }
    tif_rawcp = (tidata_t) bp;
    tif_rawcc = cc;
    if (occ > 0) {
        char m[256];
        sprintf(m, "PackBitsDecode: Not enough data for scanline %d", tif_row);
        throw TiffError(TIFF_ERROR, m, EKTIFF_WHERE);
        
    }
    /* check for buffer overruns? */
    return (1);
}


#endif /* PACKBITS_SUPPORT */

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
#include "TiffCodec.h"
#include "TiffIO.h"
#include "TiffMemMapIO.h"
#include "TiffUtil.h"



TiffCodec::~TiffCodec()
{
    if (tif_rawdata) 
    {
        if (tiffio->flags() & TIFF_MYBUFFER) 
        {
            free(tif_rawdata);
        }
        tif_rawdata = NULL;
    }
}

tiff_uint32 TiffCodec::defaultStripSize(tiff_uint32 s)
{
    if ((tiff_int32) s < 1) 
    {
        /*
         * If RowsPerStrip is unspecified, try to break the
         * image up into strips that are approximately 8Kbytes.
         */
        tsize_t scanline = scanlineSize();
        s = (tiff_uint32)(8*1024) / (scanline == 0 ? 1 : scanline);
        if (s == 0)		/* very wide images */
            s = 1;
    }
    return (s);	
}


void TiffCodec::defaultTileSize(tiff_uint32 *tw, tiff_uint32 *th)
{
    if (*(tiff_int32*) tw < 1)
        *tw = 256;
    if (*(tiff_int32*) th < 1)
        *th = 256;
    /* roundup to a multiple of 16 per the spec */
    if (*tw & 0xf)
        *tw = TIFFroundup(*tw, 16);
    if (*th & 0xf)
        *th = TIFFroundup(*th, 16);
}


// todo: add implementation to swabXXbitdata
void TiffCodec::postDecode(tidata_t buf, tsize_t cc)
{
	if (tiffio->doSwab())
    {
        switch ((tiffdir->bitsPerSample())[0])
        {
            case 16:
                TiffUtil::swabArrayOfShort((uint16*)buf, cc/2);
                break;
            case 32:
                TiffUtil::swabArrayOfLong((tiff_uint32*)buf, cc/4);
        }
    }
}

tsize_t TiffCodec::scanlineSize()
{
    return tiffdir->scanlineSize();
}


tsize_t TiffCodec::tileRowSize()
{
    return tiffdir->tileRowSize();
}


/*
 * Bit reversal tables.  TIFFBitRevTable[<byte>] gives
 * the bit reversed value of <byte>.  Used in various
 * places in the library when the FillOrder requires
 * bit reversal of byte values (e.g. CCITT Fax 3
 * encoding/decoding).
 */
static const unsigned char TIFFBitRevTable[256] = {
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
    0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};


void TiffCodec::reverseBits(void )
{
    reverseBits( tif_rawcc ) ;

    return ;
}


void TiffCodec::reverseBits( register tiff_uint32 nBytes )
{

    register uint8* cp = tif_rawdata ;
    register tiff_uint32 n = nBytes ;
    
    for ( ; n > 8; n -= 8) 
    {
        cp[0] = TIFFBitRevTable[cp[0]];
        cp[1] = TIFFBitRevTable[cp[1]];
        cp[2] = TIFFBitRevTable[cp[2]];
        cp[3] = TIFFBitRevTable[cp[3]];
        cp[4] = TIFFBitRevTable[cp[4]];
        cp[5] = TIFFBitRevTable[cp[5]];
        cp[6] = TIFFBitRevTable[cp[6]];
        cp[7] = TIFFBitRevTable[cp[7]];
        cp += 8;
    }
    while (n-- > 0)
        *cp = TIFFBitRevTable[*cp], cp++;

    return ;
}


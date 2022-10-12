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


#ifndef _TIFFCODECNONE_H
#define _TIFFCODECNONE_H

#include "TiffCodec.h"

class TiffCodecNone : public TiffCodec
{
    public:
	TiffCodecNone() { }
	~TiffCodecNone() { }

    protected:

	virtual int init(TiffDirectory* tdir, TiffIO* tio, TiffImage *img)
	{
            if (tiffdir)
            {
                assert(tiffio);
                return 1;
            }
            
            tiffdir = tdir;
            tiffio = tio;
            tiffimg = img;
            return 1;
	}

	virtual int setupDecode() { return 1; }
	virtual int setupEncode() { return 1; }
	virtual int preDecode(tsample_t s) { return 1; }
	virtual int preEncode(tsample_t s) { return 1; }
	virtual int postEncode() { return 1; }
	virtual int decodeRow(tidata_t buf, tsize_t cc, tsample_t s)
	{
            return DumpModeDecode(buf, cc, s);
	}
	virtual int encodeRow(tidata_t buf, tsize_t cc, tsample_t s)
	{
            return DumpModeEncode(buf, cc, s);
	}
	virtual int decodeStrip(tidata_t buf, tsize_t cc, tsample_t s)
        {
            return DumpModeDecode(buf, cc, s);
	}
	virtual int encodeStrip(tidata_t buf, tsize_t cc, tsample_t s)
	{
            return DumpModeEncode(buf, cc, s);
	}
	virtual int decodeTile(tidata_t buf, tsize_t cc, tsample_t s)
	{
            return DumpModeDecode(buf, cc, s);
	}
	virtual int encodeTile(tidata_t buf, tsize_t cc, tsample_t s)
	{
            return DumpModeEncode(buf, cc, s);
	}
	virtual int seek(tiff_uint32 nrows)
	{
            return DumpModeSeek(nrows);
	}
	virtual void close() { }
	virtual void cleanup() { }

	int DumpModeEncode(tidata_t pp, tsize_t cc, tsample_t s);
	int DumpModeDecode(tidata_t buf, tsize_t cc, tsample_t s);
	int DumpModeSeek(tiff_uint32 nrows);

};

#endif











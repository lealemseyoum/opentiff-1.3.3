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
#include "TiffMemoryIO.h"
#include "TiffError.h"

toff_t TiffMemoryIO::seek( toff_t off, int whence )
{
    switch(whence)
    {
	case SEEK_SET:
            curBufOffset = off;
            break;
        case SEEK_CUR:
            curBufOffset += off;
            break;
        case SEEK_END:
            curBufOffset = bytesWritten;
            break;
        default:
            curBufOffset = off;
            break;
    }

    return(curBufOffset);
}

tsize_t TiffMemoryIO::read( tdata_t buf, tsize_t size )
{
    if (bufferSize >= (tsize_t)curBufOffset + size)
    {
        memcpy( (char*)buf, imageBuffer + curBufOffset, size ) ;
        curBufOffset += size ;
        return size ;
    }
    else
    {
        char msg[256];
        sprintf(msg, "TiffMemoryIO::read - can't read beyond image buffer, current offset: %ld, size: %ld, buffer size: %ld", curBufOffset, size, bufferSize);
        throw TiffError(TIFF_IO_READ, msg, EKTIFF_WHERE);
    }
}


tsize_t TiffMemoryIO::write( tdata_t buf, tsize_t size )
{
    if (bufferSize >= (tsize_t)curBufOffset + size)
    {
        memcpy( imageBuffer + curBufOffset, (char*)buf, size ) ;
        curBufOffset += size;
        if (bytesWritten < (tsize_t)curBufOffset)
            bytesWritten = curBufOffset;
        return size ;
    }
    else
    {
        char msg[256];
        sprintf(msg, "TiffMemoryIO::write - can't write beyond image buffer, current offset: %ld, size: %ld, buffer size: %ld", curBufOffset, size, bufferSize);
        throw TiffError(TIFF_IO_WRITE, msg, EKTIFF_WHERE);
    }
}


toff_t TiffMemoryIO::size()
{
    if ((parent.mode() | O_RDWR )|| (parent.mode() | O_CREAT))
        return bytesWritten;
    else
        return bufferSize;	// may not be the image size;
}


void TiffMemoryIO::close()
{

}


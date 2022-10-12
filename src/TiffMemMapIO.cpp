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
 * George Sotak <george.sotak@kodak.com>
 * 
 * Contributor(s): 
 * Chris Lin <ti.lin@kodak.com> 
 */ 


#ifndef macintosh
#include <sys/types.h>
#else
#include <MacMemory.h>
#include <unistd.h>
#endif 

#include <fcntl.h>
#include <stdio.h>
#include "TiffConf.h"
#include "TiffMemMapIO.h"


// NOTE: The MacOS does not implement access to file through memory maps before OS 9.1,
// and the compiler interfaces are not out yet for OS 9.1. At this point, it is just best
// to fake out the TiffMemMapIO interface with traditional file IO

#ifdef __APPLE_CC__
#include "sys/types.h"
#include "sys/mman.h"
//#define unix
#endif


toff_t
TiffMemMapIO::seek( toff_t off, int whence )
{
    // KODAK - todo, check that requested offset is
    // not greater than the file size  => CANNOT DO THIS
    // SINCE IN THE CASE OF WRITING THE CURRENT OFFSET 
    // MAY NEED TO GET SET PAST THE END OF FILE.

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
    
#ifdef macintosh
  if (lseek((int)fileHandle, curBufOffset, SEEK_SET) == -1) return -1;
#endif

    return(curBufOffset);
}

tsize_t
TiffMemMapIO::read( tdata_t buf, tsize_t sizen )
{
#ifdef macintosh
  sizen = ::read((int)fileHandle, buf, sizen);
#else
    memCopy( buf, mapBase+curBufOffset, sizen ) ;
#endif
 
    curBufOffset += sizen ;
 
    return sizen ;
}


tsize_t
TiffMemMapIO::write( tdata_t buf, tsize_t size )
{
#ifdef macintosh
  size = ::write((int)fileHandle, buf, size);
#else
    memCopy( mapBase+curBufOffset, buf, size ) ;
#endif

    curBufOffset += size;
    if (curBufOffset > bytesWritten)
        bytesWritten = curBufOffset;

    return size ;
}


TiffMemMapIO*
TiffMemMapIO::memMapFile( TiffIO& theParent, ekthandle_t& fileHandle, toff_t mapSize )
{

#ifdef macintosh
  return new TiffMemMapIO( theParent, fileHandle, 0, NULL, mapSize);
#endif


    ekthandle_t tmpPvHandle ;

    if( mapSize <= 0 )
        if( (mapSize = theParent.size()) == static_cast<toff_t>(-1) )
            return static_cast<TiffMemMapIO*>(NULL) ;

#if (defined _MSC_VER) && (defined WIN32)     
    tiff_int32 mapAccessType = PAGE_READWRITE ;
    tiff_int32 viewAccessType = FILE_MAP_WRITE ;
    if( theParent.mode() == O_RDONLY )
    {
        mapAccessType = PAGE_READONLY ;
        viewAccessType = FILE_MAP_READ ;
    }


    if( (tmpPvHandle = static_cast<ekthandle_t>(CreateFileMapping(fileHandle, NULL, mapAccessType, 0, mapSize, NULL))) == NULL)
        return static_cast<TiffMemMapIO*>(NULL) ;

    tdata_t base ;
    if( (base = MapViewOfFile(tmpPvHandle, viewAccessType, 0, 0, mapSize)) == NULL)
    {
        CloseHandle(tmpPvHandle);
        return static_cast<TiffMemMapIO*>(NULL) ;
    }

#endif

#if defined(unix) || defined(__unix)

    int mapAccessType = PROT_READ|PROT_WRITE ;
  
    if( theParent.mode() == O_RDONLY )
        mapAccessType = PROT_READ ;
  
    tmpPvHandle = 0 ;
  
    void* base =  mmap(0, mapSize, mapAccessType, MAP_SHARED, (int)fileHandle, 0);
    // On Linux, when mapSize is 0, mmap return base = 0, rather than the expected
    // MAP_FAILED.
    if( (base == MAP_FAILED) || (base == 0) ) 
        return static_cast<TiffMemMapIO*>(NULL) ;

#endif

#ifndef macintosh
    return new TiffMemMapIO( theParent, fileHandle, tmpPvHandle, 
                             (tidata_t)(base), mapSize ) ;
#endif
}



void
TiffMemMapIO::memUnMapFile( void )
{
    if( fileMapped )
    {
#if (defined _MSC_VER) && (defined WIN32) 
        UnmapViewOfFile( mapBase );
        CloseHandle( pvMapHandle );
#endif

#if defined(unix) || defined(__unix)
        munmap((char*)mapBase, size);
#endif

// TODO - check into whether or not this is necessary...
//        if( bytesWritten != 0 )
//          {
//            DWORD dwCurrentFilePosition = 
//              SetFilePointer( fileHandle, bytesWritten, NULL, FILE_BEGIN); 
          
//            SetEndOfFile( fileHandle );
//          }

        fileMapped = false ;
    }
  
    return;
}


void
TiffMemMapIO::memSet(void* p, int v, tsize_t c)
{
#if (defined _MSC_VER) && (defined WIN32)
    FillMemory(p, c, (BYTE)v);
#endif

#if defined(unix) || defined(__unix) || defined(macintosh)
    memset( p, v, c ) ;
#endif
}

void
TiffMemMapIO::memCopy(void* d, const tdata_t s, tsize_t c)
{
#if (defined _MSC_VER) && (defined WIN32)
    CopyMemory(d, s, c);
#endif

#if defined(unix) || defined(__unix)
    memcpy( d, s, c ) ;
#endif

#if defined(macintosh)
  BlockMove(s, d, c);
#endif
}

int
TiffMemMapIO::memCmp(const tidata_t p1, const tidata_t p2, tsize_t c)
{
    const uint8 *pb1 = p1;
    const uint8 *pb2 = p2;
    tiff_int32 dwTmp = c;
    int iTmp;
    for (iTmp = 0; dwTmp-- && !iTmp; iTmp = (int)*pb1++ - (int)*pb2++)
        ;
    return (iTmp);
}

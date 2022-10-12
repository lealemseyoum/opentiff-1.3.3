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


#include "TiffConf.h"

#include "TiffError.h"
#include "TiffIO.h"
#include "TiffFileIO.h"
#if ENABLE_MEM_MAP
#include "TiffMemMapIO.h"
#endif
#ifdef WIN32
#include "TiffInternetIO.h"
#endif
#include "TiffMemoryIO.h"
#include "TiffUtil.h"

#define    HDROFF(f)    ((toff_t) &(((TiffHeader*) 0)->f))

TiffIO::TiffIO( ekthandle_t theFileHandle, const string& filename, tiff_uint32 imode )
    : _name(filename), _mode(imode), fileHandle( theFileHandle )
{
    handleIO = new TiffFileIO( *this, fileHandle ) ;

    _lastNextOffset = HDROFF(diroff);
}


TiffIO::TiffIO(void* imgBuf, tsize_t bufSize, tiff_uint32 imode )
    : _name(""), _mode(imode), fileHandle(0)
{
    handleIO = new TiffMemoryIO(*this, imgBuf, bufSize);
    _lastNextOffset = HDROFF(diroff);
}


#ifdef WIN32
TiffIO::TiffIO( HINTERNET hInet, const string& filename, tiff_uint32 imode)
    : _name(filename), fileHandle(0), _mode(imode)
{
    handleIO = new TiffInternetIO(*this, hInet);
    _lastNextOffset = HDROFF(diroff);
}
#endif
 
/*
 * Open a TIFF file for read/writing.
 */
TiffIO*
TiffIO::open(const char* filename, const char* mode)
{
#if (defined _MSC_VER) && (defined WIN32) 
    DWORD accessMode ;
    DWORD dwMode ;

    tiff_uint32 imode = convertMode( mode ) ;

    switch( imode )
    {
        case O_RDONLY:
            accessMode = GENERIC_READ ;
            dwMode = OPEN_EXISTING;
            break;
        case O_RDWR:
            accessMode = GENERIC_READ | GENERIC_WRITE ;
            dwMode = OPEN_ALWAYS;
            break;
        case O_RDWR|O_CREAT:
            accessMode = GENERIC_READ | GENERIC_WRITE ;
            dwMode = CREATE_NEW;
            break;
        case O_RDWR|O_TRUNC:
            accessMode = GENERIC_READ | GENERIC_WRITE ;
            dwMode = CREATE_ALWAYS;
            break;
        case O_RDWR|O_CREAT|O_TRUNC:
            accessMode = GENERIC_READ | GENERIC_WRITE ;
            dwMode = CREATE_ALWAYS;
            break;
        default:
            return static_cast<TiffIO*>(NULL) ;
    }

    ekthandle_t fd = static_cast<ekthandle_t>(
        CreateFile(filename, accessMode, FILE_SHARE_READ, NULL, dwMode,
                   (imode == O_RDONLY) ? FILE_ATTRIBUTE_READONLY :
                   FILE_ATTRIBUTE_NORMAL, NULL) ) ;

    if( fd == INVALID_HANDLE_VALUE) 
    {
        string errMsg("Could not open file: ") ;
        errMsg += filename ;
        throw TiffError(TIFF_IO, errMsg, EKTIFF_WHERE);
    }

    return new TiffIO( fd, filename, imode ) ;
#endif


#if defined(unix) || defined(__unix) || defined(macintosh)
    int fd;

    tiff_uint32 imode = convertMode( mode ) ;
// Following check doesn't make any sense
//    if (imode == -1)
//        return (0);
#if defined(_AM29K) || defined (macintosh)
    fd = ::open(filename, imode);
#else
    fd = ::open(filename, imode, 0666);
#endif
    if (fd < 0) {
        string errMsg("Could not open file: ") ;
        errMsg += filename ;
        throw TiffError(TIFF_IO, errMsg, EKTIFF_WHERE);
    }
    return new TiffIO( (ekthandle_t)fd, filename, imode ) ;
#endif

}
 

void TiffIO::close()
{
    if (handleIO)
    {
        handleIO->close();
        delete handleIO;
        handleIO = 0;
    }
}


// may need to include sys/stat.h on the unix side
toff_t
TiffIO::size()
{
    return handleIO->size();
}


void 
TiffIO::memMapFile( toff_t& mapSize )
{
#if ENABLE_MEM_MAP
    TiffHandleIO* tmp = TiffMemMapIO::memMapFile( *this, fileHandle, mapSize ) ;

    if( tmp != NULL )
    {
        delete handleIO ;
        handleIO = tmp ;
    }
    else if( _flags & TIFF_MAPPED )
    {
        _flags &= ~TIFF_MAPPED;
    }
#endif

    return ;
}


void 
TiffIO::memUnMapFile( void )
{
#if ENABLE_MEM_MAP
    TiffMemMapIO* memMapIO = dynamic_cast<TiffMemMapIO*>(handleIO) ;
  
    memMapIO->memUnMapFile( ) ;

    delete handleIO ;
  
    handleIO = new TiffFileIO( *this, fileHandle ) ;
#endif

    return ;
}

tiff_uint32
TiffIO::convertMode( const char* cmode )
{
    tiff_uint32 imode = 0 ;
  
    switch( cmode[0] ) 
    {
        case 'r':
            imode = O_RDONLY;
            if( cmode[1] == '+' )
                imode = O_RDWR;
            break;

        case 'w':
        case 'a':
            imode = O_RDWR|O_CREAT;
            if( cmode[0] == 'w' )
                imode |= O_TRUNC;
            break;

        default:
            string errMsg("Invalide file mode specification: ") ;
            errMsg += cmode[0] ;
            throw TiffError(TIFF_IO, errMsg, EKTIFF_WHERE);
            break;
    }

    return imode ;
}






void TiffIO::swabShort(uint16* wp)
{
    if ( doSwab() )
        TiffUtil::swabShort(wp); 
}


void TiffIO::swabLong(tiff_uint32* lp)
{
    if ( doSwab() )
        TiffUtil::swabLong(lp);
}

void TiffIO::swabArrayOfShort(uint16* wp, tiff_uint32 n)
{
    if( doSwab() )
        TiffUtil::swabArrayOfShort(wp, n);
}

void TiffIO::swabArrayOfLong(tiff_uint32* lp, tiff_uint32 n)
{
    if ( doSwab() )
        TiffUtil::swabArrayOfLong(lp, n);
}

void TiffIO::swabDouble(double *dp)
{
    if( doSwab() )
        TiffUtil::swabDouble(dp);
}


void TiffIO::swabArrayOfDouble(double* dp, tiff_uint32 n)
{
    if ( doSwab() )
        TiffUtil::swabArrayOfDouble(dp, n);
}

tidata_t TiffIO::mapBase()
{
#if ENABLE_MEM_MAP
  TiffMemMapIO* mapIO = dynamic_cast<TiffMemMapIO*>(handleIO);
  if (mapIO)
    return mapIO->base();
  else
#endif
    return NULL;
}

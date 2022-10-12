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
#include "TiffFileIO.h"
#include "TiffError.h"

#if defined(unix) || defined(__unix)
#include <sys/stat.h>
#elif defined(macintosh)
#include <stat.h>
#endif

tsize_t
TiffFileIO::read( tdata_t buf, tsize_t size)
{
#if (defined _MSC_VER) && (defined WIN32) 
    DWORD dwSizeRead;

    /* ReadFile is the Win32 API call to read from a file */
    if( !ReadFile(fileHandle, buf, size, &dwSizeRead, NULL) )
        return(0);

    return static_cast<tsize_t>(dwSizeRead);
#endif

#if defined(unix) || defined(__unix) || defined (macintosh)
    return ::read((int)fileHandle, buf, (int)size);
#endif
}



tsize_t
TiffFileIO::write( tdata_t buf, tsize_t size )
{
#if (defined _MSC_VER) && (defined WIN32) 
    DWORD dwSizeWritten = size;
    if( !WriteFile(fileHandle, buf, size, &dwSizeWritten, NULL) )
        return(0);
  
    return static_cast<tsize_t>(dwSizeWritten);
#endif


#if defined(unix) || defined(__unix) || defined (macintosh)
    return static_cast<tsize_t>(::write((int)fileHandle, buf, static_cast<size_t>(size)));
#endif
}


toff_t
TiffFileIO::seek( toff_t off, int whence)
{
#if (defined _MSC_VER) && (defined WIN32) 

//  Note that it is not an error to set the file pointer to a position beyond the
//  end of the file. The size of the file does not increase until you call the
//  SetEndOfFile, WriteFile, or WriteFileEx function. A write operation increases
//  the size of the file to the file pointer position plus the size of the buffer
//  written, leaving the intervening bytes uninitialized. 

    DWORD dwMoveMethod;
  
    switch(whence)
    {
        case SEEK_SET:  //0
            dwMoveMethod = FILE_BEGIN;
            break;
        case SEEK_CUR:  //1
            dwMoveMethod = FILE_CURRENT;
            break;
        case SEEK_END:  //2
            dwMoveMethod = FILE_END;
            break;
        default:
            dwMoveMethod = FILE_BEGIN;
            break;
    }
  
    return static_cast<toff_t>(SetFilePointer(fileHandle, off, NULL, dwMoveMethod) );
#endif


#if defined(unix) || defined(__unix) || defined(macintosh)
//  lseek() allows the file pointer to be set beyond the  exist-
//  ing  data  in  the  file.  If data are later written at this
//  point, subsequent reads in the gap between the previous  end
//  of  data  and  the  newly  written data will return bytes of
//  value 0 until data are written into the gap.

    return static_cast<toff_t>( lseek((int)fileHandle, 
                                      static_cast<off_t>(off), whence));
#endif
}


toff_t TiffFileIO::size()
{
#if (defined _MSC_VER) && (defined WIN32)
    return ((toff_t)GetFileSize( fileHandle, NULL));
#endif
#if defined(unix) || defined(__unix) || defined(macintosh)
    struct stat sb;
    return (toff_t) (fstat((int) fileHandle, &sb) < 0 ? 0 : sb.st_size);
#endif
}


void TiffFileIO::close()
{
#if (defined _MSC_VER) && (defined WIN32) 
    if (fileHandle)
    {
        if (!CloseHandle(fileHandle))
        {
            string errMsg("Could not close file: ") ;
            errMsg += parent.name() ;  
            throw TiffError(TIFF_IO, errMsg, EKTIFF_WHERE);
        }
        fileHandle = 0;
    }
#endif

#if defined(unix) || defined(__unix) || defined(macintosh)
    if (fileHandle)
    {
        if (::close((int)fileHandle))
        {
            string errMsg("Could not close file: ") ;
            errMsg += parent.name() ;  
            throw TiffError(TIFF_IO, errMsg, EKTIFF_WHERE);
        }
        fileHandle = 0;
    }
#endif
}


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


#ifndef TIFF_MEM_MAP_IO_H
#define TIFF_MEM_MAP_IO_H

#include "TiffTypeDefs.h"
#include "TiffFileIO.h"

class TiffMemMapIO : public TiffFileIO
{

        
    public:
        TiffMemMapIO( TiffIO& theParent, ekthandle_t& fhandle, ekthandle_t pvHandle, tidata_t base, toff_t theSize ) 
            : TiffFileIO(theParent, fhandle), pvMapHandle(pvHandle),
              mapBase(base), curBufOffset(0), size(theSize), bytesWritten(0),
              fileMapped(true)
            {}
        ~TiffMemMapIO( void ) 
            {
                memUnMapFile() ;
            }
        
        static TiffMemMapIO* memMapFile( TiffIO& parent, ekthandle_t& fileHandle, toff_t mapSize=0 ) ;
        void memUnMapFile( void ) ;
        
        virtual tsize_t read( tdata_t buf, tsize_t sizen ) ;
        virtual tsize_t write( tdata_t buf, tsize_t sizen ) ;
        virtual toff_t seek( toff_t off, int whence ) ;
	
	tidata_t base() { return mapBase; }

    private:
	    ekthandle_t  pvMapHandle;	
        tidata_t     mapBase ;         // was tif->tif_base
        toff_t       curBufOffset ;
	    toff_t       size;	          // size of mapped file region (bytes)
        toff_t       bytesWritten ;
        bool         fileMapped ;
        
        void memSet( void* p, int v, tsize_t c ) ;
        void memCopy( void* d, const tdata_t s, tsize_t c ) ;
        int  memCmp( const tidata_t p1, const tidata_t p2, tsize_t c ) ;
        
};


#endif // TIFF_MEM_MAP_IO_H

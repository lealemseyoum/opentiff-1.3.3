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
#include "TiffError.h"
#include "TiffIO.h"
#include "TiffTagEntry.h"
#include "TiffDirectory.h"
#include "TiffImageFile.h"

TiffImageFile::~TiffImageFile()
{
    if (tiffio)
        delete tiffio;
}

void TiffImageFile::readDirectories()
{  
    uint16 dircount ;
    toff_t dirOffset;
    toff_t nextOff = header.diroff;

    uint16 idx = 0;
    while (true)
    {
        // reach last dir
        if (nextOff == 0)
            return;
        else
            dirOffset = nextOff;

        tiffio->seek( dirOffset, SEEK_SET ) ;
      
        tiffio->read( &dircount, SIZEOF_UINT16 ) ;
      
        tiffio->swabShort( &dircount ) ;
      
        tiffio->seek( dircount*sizeof(TiffDirEntry), SEEK_CUR ) ;
      
        tiffio->read( &nextOff, 4 ) ; // must be 4 by the spec...

        tiffio->swabLong( reinterpret_cast<tiff_uint32*>(&nextOff) ) ;
      
        TiffTagEntry    *tagEntry = new TiffTagEntryT<tiff_uint32>( TIFF_MAINIFD, EKTIFF_USHORT );
        mainDirs.push_back(new TiffDirectory( tiffio, tagEntry->getTagNum(), dirOffset, idx )) ;
        delete tagEntry;
        mainDirs[idx]->readDirTree() ;

        idx++ ;
    }
}


void TiffImageFile::writeDirectories()
{
    for (unsigned int i=0; i<mainDirs.size(); i++)
        mainDirs[i]->writeDirTree();
}


TiffDirectory* TiffImageFile::getDirectory( uint16 index)
{
    if (index < mainDirs.size())
    {
        return mainDirs[index];
    }
    else 
        return 0;
}


TiffDirectory*
TiffImageFile::getDirectory( DirPath& dirPath )
{
    TiffDirectory* crntDir = 0;
    if (!dirPath.empty())
        crntDir = getDirectory(dirPath[0].second) ;

    for (unsigned int i=1; i<dirPath.size() && crntDir != 0; i++)
    {
        TiffDirectory::DirVec dirVec;
        if (crntDir->getSubDirVec(dirPath[i].first, dirVec) &&
            dirPath[i].second < dirVec.size())
        {
            crntDir = dirVec[dirPath[i].second];        
        }
        else
            crntDir = 0;
    }

    return crntDir ;
}

  
TiffTagEntry*
TiffImageFile::getGenericTag( ttag_t tag, DirPath& dirPath )
{
    const TiffDirectory* dir = (const TiffDirectory*)getDirectory( dirPath ) ;

    if( dir!=NULL )
        return dir->getGenericTag( tag ) ;
    else
        return static_cast<TiffTagEntry*>(NULL) ;
}

void 
TiffImageFile::setGenericTag( const TiffTagEntry& entry, DirPath& dirPath )
{
    TiffDirectory* dir = getDirectory(dirPath);
    if (!dir)
    {
        createDirectory(dirPath);
        dir = getDirectory(dirPath);
    }
    dir->setGenericTag(entry);
}

// Remove a tag entry from an IFD
void TiffImageFile::removeGenericTag(ttag_t tag, DirPath& dirPath )
{
    TiffDirectory* dir = getDirectory( dirPath ) ;

    if( dir!=NULL )
        dir->removeGenericTag( tag );
}

void TiffImageFile::open(const char* name, const char* cmode)
{
    toff_t mapSize = 0;
    open(name, cmode, mapSize);
}


void TiffImageFile::open(ekthandle_t fileHandle, const char* name, const char* cmode)
{
    toff_t mapSize = 0;
    open(fileHandle, name, cmode, mapSize);
}


void
TiffImageFile::open(const char* filename, const char* cmode, toff_t& mapSize )
{
    tiffio = TiffIO::open( filename, cmode ) ;
  
    if( tiffio )
    {
        fileName = filename ;
        initAfterOpen( cmode, mapSize ) ;
    }
    else
    {
        string errMsg("Problem opening file, ") ;
        errMsg += filename ;
        errMsg += ", TiffIO instance returned as NULL; possible memory allocation failure" ;
        throw TiffError(TIFF_IO, errMsg, EKTIFF_WHERE);
    }

    return ;
}

   
void
TiffImageFile::open(ekthandle_t fileHandle, const char* filename, const char* cmode, toff_t& mapSize )
{
    tiff_uint32 imode = TiffIO::convertMode( cmode ) ;

    tiffio = new TiffIO( fileHandle, filename, imode ) ;

    // What is going on here???
//  tiffio->mode() = tiffio->mode() & ~(O_CREAT|O_TRUNC);
  
    initAfterOpen( cmode, mapSize ) ;
  
    return ;
}


void TiffImageFile::open(void* imgBuf, tsize_t bufSize, const char* cmode)
{
    char tmode[20];
    strcpy(tmode, cmode);
    char* cp;
    // disable mem map
    for(cp=tmode; *cp; cp++)
    {
        if (*cp == 'M')
            *cp = 'm';
    }

    toff_t  mapSize = 0;
    tiff_uint32 imode = TiffIO::convertMode(tmode) ;

    tiffio = new TiffIO(imgBuf, bufSize, imode) ;

    initAfterOpen(tmode, mapSize) ;
}


#ifdef WIN32
void TiffImageFile::open(HINTERNET hInet, const char* name)
{

    char cmode[] = "rm";
    toff_t mapSize = 0;
    tiff_uint32 imode = TiffIO::convertMode(cmode) ;

    tiffio = new TiffIO(hInet, name, imode) ;

    initAfterOpen(cmode, mapSize) ;
  
    return ;
}
#endif // WIN32


void TiffImageFile::close(tsize_t* imgSize)
{

    if (tiffio && (tiffio->mode()&O_RDWR || tiffio->mode()&O_CREAT))
    {
        writeDirectories();
    }
    for (unsigned int i=0; i<mainDirs.size(); i++)
        delete mainDirs[i];
    mainDirs.clear();
    if (tiffio)
    {
        if (imgSize)
            *imgSize = tiffio->size();
        delete tiffio;
        tiffio = NULL;
    }
}


void
TiffImageFile::initAfterOpen( const char* cmode, toff_t& mapSize )
{
    /*
     * Default is to return data MSB2LSB and enable the
     * use of memory-mapped files and strip chopping when
     * a file is opened read-only.
   */
    tiffio->flags() = FILLORDER_MSB2LSB;
    if( tiffio->mode() == O_RDONLY)
#if ENABLE_MEM_MAP
        tiffio->flags() |= TIFF_MAPPED|TIFF_STRIPCHOP;
#else
        tiffio->flags() |= TIFF_STRIPCHOP;
#endif

    // Check the endianness of the current architechture
    bool bigendian = HOST_BIGENDIAN ;
//   union 
//   { 
//       tiff_int32 i; 
//       char c[4]; 
//   } u; 
//   u.i = 1; 
//   bigendian = u.c[0] == 0;

  /*
   * Process library-specific flags in the open mode string.
   * The following flags may be used to control intrinsic library
   * behaviour that may or may not be desirable (usually for
   * compatibility with some application that claims to support
   * TIFF but only supports some braindead idea of what the
   * vendor thinks TIFF is):
   *
   * 'l'        use little-endian byte order for creating a file
   * 'b'        use big-endian byte order for creating a file
   * 'L'        read/write information using LSB2MSB bit order
   * 'B'        read/write information using MSB2LSB bit order
   * 'H'        read/write information using host bit order
   * 'M'        enable use of memory-mapped files when supported
   * 'm'        disable use of memory-mapped files
   * 'C'        enable strip chopping support when reading
   * 'c'        disable strip chopping support
   *
   * The use of the 'l' and 'b' flags is strongly discouraged.
   * These flags are provided solely because numerous vendors,
   * typically on the PC, do not correctly support TIFF; they
   * only support the Intel little-endian byte order.  This
   * support is not configured by default because it supports
   * the violation of the TIFF spec that says that readers *MUST*
   * support both byte orders.  It is strongly recommended that
   * you not use this feature except to deal with busted apps
   * that write invalid TIFF.  And even in those cases you should
   * bang on the vendors to fix their software.
   *
   * The 'L', 'B', and 'H' flags are intended for applications
   * that can optimize operations on data by using a particular
   * bit order.  By default the library returns data in MSB2LSB
   * bit order for compatibiltiy with older versions of this
   * library.  Returning data in the bit order of the native cpu
   * makes the most sense but also requires applications to check
   * the value of the FillOrder tag; something they probabyl do
   * not do right now.
   *
   * The 'M' and 'm' flags are provided because some virtual memory
   * systems exhibit poor behaviour when large images are mapped.
   * These options permit clients to control the use of memory-mapped
   * files on a per-file basis.
   *
   * The 'C' and 'c' flags are provided because the library support
   * for chopping up large strips into multiple smaller strips is not
   * application-transparent and as such can cause problems.  The 'c'
   * option permits applications that only want to look at the tags,
   * for example, to get the unadulterated TIFF tag information.
   */
    const char* cp ;
    for(cp = cmode; *cp; cp++)
        switch (*cp) 
        {
            case 'b':
                if ((tiffio->mode()&O_CREAT) && !bigendian)
                    tiffio->flags() |= TIFF_SWAB;
                break;
            case 'l':
                if ((tiffio->mode()&O_CREAT) && bigendian)
                    tiffio->flags() |= TIFF_SWAB;
                break;
            case 'B':
                tiffio->flags() = (tiffio->flags() &~ TIFF_FILLORDER) |
                    FILLORDER_MSB2LSB;
                break;
            case 'L':
                tiffio->flags() = (tiffio->flags() &~ TIFF_FILLORDER) |
                    FILLORDER_LSB2MSB;
                break;
            case 'H':
                tiffio->flags() = (tiffio->flags() &~ TIFF_FILLORDER) |
                    HOST_FILLORDER;
                break;
            case 'M':
                //    if (m == O_RDONLY)
                tiffio->flags() |= TIFF_MAPPED;
                break;
            case 'm':
                //    if (m == O_RDONLY)
                tiffio->flags() &= ~TIFF_MAPPED;
                break;
/*
  case 'Z':
  tiffio->flags() |= TIFF_MYBUFFER;
  break;
  case 'z':
  tiffio->flags() &= ~TIFF_MYBUFFER;
  break;
*/
#ifdef STRIPCHOP_SUPPORT
            case 'C':
                if (tiffio->mode() == O_RDONLY)
                    tiffio->flags() |= TIFF_STRIPCHOP;
                break;
            case 'c':
                if (tiffio->mode() == O_RDONLY)
                    tiffio->flags() &= ~TIFF_STRIPCHOP;
                break;
#endif            
        }

    // Setup for writing either appending or new
    if( tiffio->mode() & (O_RDWR|O_CREAT) )
    {
        bool appending = false ;
      
        if( !(tiffio->mode() & O_TRUNC) )
        {
            try
            {
                readHeader() ;
                appending = true ;
            }
            catch( ... ) { appending = false ; }
        }
      
      
        if( !appending )
        {
            /* KODAK - so if we are opening a write only file in memory,
               mapSize will be non-zero; otherwise, we are opening write
               only file in the file-system and mapsize is zero. */
            if( tiffio->isMapped() )
                tiffio->memMapFile( mapSize );
          
            initHeader( bigendian ) ;
            initEndian( bigendian );
            writeHeader() ;
                 
            return ;
        }
        /* END OF Setting up a new file for writing */
    }
    else
    {
        readHeader() ;
    }

    initEndian( bigendian );


  /*
   * Setup the byte order handling.
   */
/*
  if( !checkEndian() )
    {
      string errMsg("Incorrect specification of Endianess in file: ") ;
      errMsg += fileName ;
      throw TiffError(TIFF_IMAGE_FILE, errMsg, EKTIFF_WHERE);
    }
*/

    verifyVersion() ;
  
    tiffio->flags() |= TIFF_MYBUFFER;

  /*
   * Setup initial directory.
   */
  
    if( tiffio->isMapped() )
        tiffio->memMapFile( mapSize );

    readDirectories();
  
    return ;
}


void
TiffImageFile::verifyVersion( void )
{
    // Note that this isn't actually a version number, it's a
    // magic number that doesn't change (stupid).

    if( header.version != TIFF_VERSION) 
    {
        string errMsg("Not a TIFF file: ") ;
        errMsg += fileName ;
        throw TiffError(TIFF_IMAGE_FILE, errMsg, EKTIFF_WHERE);
    }

    return ;
}


void
TiffImageFile::readHeader( void )
{
  
  if( ( tiffio->read( &header.magic, 2 ) != 2 ) ||
      ( tiffio->read( &header.version, 2 ) != 2 ) ||
      ( tiffio->read( &header.diroff, 4 ) != 4 ) )
    {
      string errMsg("Could not read TIFF header from file: ") ;
      errMsg += fileName ;
      throw TiffError(TIFF_IMAGE_FILE, errMsg, EKTIFF_WHERE);
    }
  return ;
}


void
TiffImageFile::initHeader( bool bigendian )
{
  header.magic = tiffio->flags() & TIFF_SWAB
    ? (bigendian ? TIFF_LITTLEENDIAN : TIFF_BIGENDIAN)
    : (bigendian ? TIFF_BIGENDIAN : TIFF_LITTLEENDIAN);

  header.version = TIFF_VERSION;

//  if( _flags & TIFF_SWAB )
//    TIFFSwabShort(&tiffio->header.version);

  header.diroff = 0;    /* filled in later */

  return ;
}


void
TiffImageFile::writeHeader( void )
{
  if( tiffio->write( static_cast<tdata_t>(&header), sizeof (TiffHeader) ) != sizeof(TiffHeader) )
    {
      string errMsg("Could not write TIFF header to file: ") ;
      errMsg += fileName ;
      throw TiffError(TIFF_IMAGE_FILE, errMsg, EKTIFF_WHERE);
      return ;
    }

  return ;
}


void
TiffImageFile::initEndian( bool bigendian )
{
  if(header.magic == TIFF_BIGENDIAN) 
    {
      tiffio->bigEndian( true ) ;
       if( !bigendian )
        tiffio->flags() |= TIFF_SWAB;
    } 
  else 
      if (header.magic == TIFF_LITTLEENDIAN)
      {
          tiffio->bigEndian( false ) ;
          if (bigendian)
              tiffio->flags() |= TIFF_SWAB;
      }
      else
      {
          string errMsg("Tiff magic number not found: ") ;
          errMsg += fileName ;
          throw TiffError(TIFF_IMAGE_FILE, errMsg, EKTIFF_WHERE);    
      }
  
  // Swap header
  tiffio->swabShort(&header.version);
  tiffio->swabLong(&header.diroff);

  return ;
}


bool
TiffImageFile::checkEndian( void ) const
{
  if(header.magic == TIFF_BIGENDIAN) 
      tiffio->bigEndian( true ) ;
  else
      if(header.magic == TIFF_LITTLEENDIAN) 
          tiffio->bigEndian( false ) ;
      else
          return false ;
   
  return true ;
}


TiffDirectory* TiffImageFile::createDirectory()
{
    int16 dirIndex = mainDirs.size();
    mainDirs.push_back(new TiffDirectory(tiffio, TIFF_MAINIFD, dirIndex));
    return mainDirs[dirIndex];
}


void TiffImageFile::createDirectory(DirPath dirPath)
{
    if (dirPath.empty())
        throw TiffError(TIFF_ERROR, "empty DirPath", EKTIFF_WHERE);

    int nextInd = mainDirs.size();
    int ind = dirPath[0].second;
    if (nextInd == ind)
        createDirectory();
    else 
        if (nextInd < ind)
    {
            char msg[100];
            sprintf(msg, "TiffImageFile::setGenericTag - the next main directory to create should be %d instead of %d", nextInd, ind);
            throw TiffError(TIFF_ERROR, msg, EKTIFF_WHERE);
    }

    TiffDirectory* crntDir = getDirectory(dirPath[0].second) ;
    for (unsigned int i=1; i<dirPath.size(); i++)
    {
        TiffDirectory::DirVec dirVec;
        crntDir->getSubDirVec(dirPath[i].first, dirVec);
        nextInd = dirVec.size();
        ind = dirPath[i].second;
        if (nextInd == ind)
            crntDir->addSubDir(dirPath[i].first);
        else 
            if (nextInd < ind)
            {
                char msg[100];
                sprintf(msg, "TiffImageFile::setGenericTag - the index of the next sub IFD(%d) to create should be %d instead of %d", 
                        dirPath[i].first, nextInd, ind);
                throw TiffError(TIFF_ERROR, msg, EKTIFF_WHERE);
            }
        crntDir = crntDir->getSubDir(dirPath[i].first, dirPath[i].second);
    }
}




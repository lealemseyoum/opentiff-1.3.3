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
 * Ricardo Rosario <ricardo.rosario@kodak.com>
 */ 

#include <stdlib.h>
#include <cmath>
#include <climits>

#include "TiffTypeDefs.h"
#include "TiffError.h"
#include "TiffImageFile.h"
#include "TiffIO.h"
#include "TiffDirectory.h"
#include "TiffTileImage.h"
#include "TiffStripImage.h"

#if ((defined _MSC_VER) && (defined WIN32)) | (defined macintosh)
#define TIFF_COPY std::copy
#endif

#if __GNUC__
#define TIFF_COPY    ::copy
#endif

TiffDirectory::SubDirVec    TiffDirectory::subDirTags;
EntryMapWrapper        TiffDirectory::defaultEntries;

void
TiffDirectory::setupDefaults( void )
{
    static bool alreadySetup = false;
    if (!alreadySetup)
    {
        alreadySetup = true;
  
    subDirTags.push_back(330);            // Sub IFD (0x8292)
    subDirTags.push_back(33426);            // Sound IFD (0x8292)
    subDirTags.push_back(34665);            // EXIF IFD (0x8769)
    subDirTags.push_back(40965);            // Interoperability IFD (0x8292)
    subDirTags.push_back(34853);            // GPS IFD
  
    // Bits per Sample
    TiffTagEntry *tagEntry = new TiffTagEntryT<uint16>( 258, EKTIFF_USHORT, 1, 1 ) ;
    defaultEntries.map.insert( EntryMap::value_type(tagEntry->getTagNum(), tagEntry ) ) ;
  
    // Compression Mode
    tagEntry = new TiffTagEntryT<uint16>( 259, EKTIFF_USHORT, 1, 1 ) ;
    defaultEntries.map.insert( EntryMap::value_type(tagEntry->getTagNum(), tagEntry ) ) ;

    // Fil Order
    tagEntry = new TiffTagEntryT<uint16>( 266, EKTIFF_USHORT, 1, 1 ) ;
    defaultEntries.map.insert( EntryMap::value_type(tagEntry->getTagNum(), tagEntry ) ) ;

    // Grey Response Unit
    tagEntry = new TiffTagEntryT<uint16>( 290, EKTIFF_USHORT, 1, 2 ) ;
    defaultEntries.map.insert( EntryMap::value_type(tagEntry->getTagNum(), tagEntry ) ) ;

    // New SubFile Type
    tagEntry = new TiffTagEntryT<tiff_uint32>( 254, EKTIFF_ULONG, 1, 0 ) ;
    defaultEntries.map.insert( EntryMap::value_type(tagEntry->getTagNum(), tagEntry ) ) ;

    // Orientation
    tagEntry = new TiffTagEntryT<uint16>( 274, EKTIFF_USHORT, 1, 1 ) ;
    defaultEntries.map.insert( EntryMap::value_type(tagEntry->getTagNum(), tagEntry ) ) ;

    // Planar Configuration
    tagEntry = new TiffTagEntryT<uint16>( 284, EKTIFF_USHORT, 1, 1 ) ;
    defaultEntries.map.insert( EntryMap::value_type(tagEntry->getTagNum(), tagEntry ) ) ;

    // Resolution Unit
    tagEntry = new TiffTagEntryT<uint16>( 296, EKTIFF_USHORT, 1, 2 ) ;
    defaultEntries.map.insert( EntryMap::value_type(tagEntry->getTagNum(), tagEntry ) ) ;

    // Rows per Strip
    tagEntry = new TiffTagEntryT<tiff_uint32>( 278, EKTIFF_ULONG, 1, LONG_MAX ) ;
    defaultEntries.map.insert( EntryMap::value_type(tagEntry->getTagNum(), tagEntry ) ) ;

    // Samples per Pixel
    tagEntry = new TiffTagEntryT<uint16>( 277, EKTIFF_USHORT, 1, 1 ) ;
    defaultEntries.map.insert( EntryMap::value_type(tagEntry->getTagNum(), tagEntry ) ) ;

    // Thresholding
    tagEntry = new TiffTagEntryT<uint16>( 263, EKTIFF_USHORT, 1, 1 ) ;
    defaultEntries.map.insert( EntryMap::value_type(tagEntry->getTagNum(), tagEntry ) ) ;

    // Tile depth
    tagEntry = new TiffTagEntryT<tiff_uint32>( 32998, EKTIFF_ULONG, 1, 1 ) ;
    defaultEntries.map.insert( EntryMap::value_type(tagEntry->getTagNum(), tagEntry ) ) ;

    // Image depth
    tagEntry = new TiffTagEntryT<tiff_uint32>( 32997, EKTIFF_ULONG, 1, 1 ) ;
    defaultEntries.map.insert( EntryMap::value_type(tagEntry->getTagNum(), tagEntry ) ) ;
 
    } // end if alreadySetup. 
}


void
TiffDirectory::writeDirTree( void )
{
    /* Walk down the directory tree and start writing
       at the leaf dirs and walk back up */

    SubDirIter subDir, endSubDir ;
    subDir = subDirs.begin() ;
    endSubDir = subDirs.end() ;
    while ( subDir != endSubDir )
    {
        DirVec dirVec = (*subDir).second;
        for (unsigned int i=0; i<dirVec.size(); i++)
        {
            dirVec[i]->writeDirTree();
        }
        subDir++  ;
    }

    writeDir() ;

    return ;
}


// When assigning multiple subdirs of the same type to the same directory,
// we need to make sure that only one TiffTagEntry is created. This tag
// is then assigned the multiple offsets and written to the file...
void
TiffDirectory::writeDir( void )
{
 
    uint16 nEntries = numEntries() ;
    // if dir empty
    if (nEntries == 0)
    {    
        offset = 0;
        return;
    }

    // offset is initialized to 0 for a dir that is ready for writing
    // if offset is not zero, then it might has benn written or it is 
    // a dir for reading
//  if (offset != 0)
//      return; 
    
    offset = entryOffset = tiffio->seek( 0, SEEK_END );

    dataOffset = offset + nEntries*sizeof(TiffDirEntry) + SIZEOF_UINT16 + SIZEOF_TOFF_T ;
  
    uint16 num = nEntries;
    tiffio->swabShort(&num);
    tiffio->write( &num, SIZEOF_UINT16 ) ;
    entryOffset += SIZEOF_UINT16 ;

    EntryIter crntEntry, endEntry ;
    crntEntry = tagEntries.begin() ;
    endEntry = tagEntries.end() ;

    while ( crntEntry != endEntry )
    {
        // set subIFD offsets if any
        DirVec dirVec;
        if (getSubDirVec((*crntEntry).second->getTagNum(), dirVec))
        {
            if ((*crntEntry).second->getCount() > 1)
            {
                vector<tiff_uint32>& offsetVec = dynamic_cast<TiffTagEntryT< vector<tiff_uint32> >*>((*crntEntry).second)->getValue() ;
                for (unsigned int i=0; i<dirVec.size(); i++)
                {
                    offsetVec[i] = dirVec[i]->getOffset();
                }
            }
            else
            {
                dynamic_cast<TiffTagEntryT<tiff_uint32>*>((*crntEntry).second)->setValue(dirVec[0]->getOffset());
            }    
        }
      
        writeNormalTag( (*crntEntry).second ) ;
        crntEntry++  ;
    }


    // link the cur dir to the tiff dir structure if it is a main dir
    toff_t linkOffset = offset + SIZEOF_UINT16 + nEntries*sizeof(TiffDirEntry);
    if (tag == TIFF_MAINIFD)
    {
    tiffio->seek(tiffio->lastNextOffset(), SEEK_SET);
    toff_t tmpoff = offset;
    tiffio->swabLong((tiff_uint32*)&tmpoff);
        tiffio->write(&tmpoff, sizeof(tiff_uint32));
    tiffio->lastNextOffset() = linkOffset;
//    tiffio->seek(0, SEEK_END);
    }
    // Write the place keeper for the offset to the next directory
    tiff_uint32 nulVal = 0 ;
    tiffio->seek(linkOffset, SEEK_SET);
    tiffio->write( &nulVal, sizeof(tiff_uint32) ) ;

    return ;
}

// Need to track two different offsets when writing:
// the offset to where the current directory entry should be
// written; second, offset to where the data (that does not
// fit in the offset field of the tag) is written. That is,
//
//  start dirEntries  -----------------------
//                      |                  |
//                      |                  |
//  dir Entry Offset -> |                  |
//                      |                  |
//                      |                  |
// end of dirEntries  -----------------------  start dir data
//                      |                  |
//                      |                  |
//  data offset ->      |                  |
//
// the start of the data segment is at the end of the
// entry segment. The size of the entry segment can be
// pre-computed and added to the offset of the start
// of the entry segment to get the offset to the start 
// of the data segment. Use the seek() method on TiffIO
// to set the respective offset for writing. The seek()
// method allows the offset to be set past the end of
// file... see the seek() implemetation in TiffIO.cpp.

void
TiffDirectory::writeNormalTag( TiffTagEntry* tagEntry )
{
    TiffDirEntry dEntry ;
    dEntry.tag = tagEntry->getTagNum() ;
    dEntry.type = tagEntry->getType() ;
    dEntry.count = tagEntry->getCount() ;
    dEntry.offset = 0 ;

    switch( dEntry.type ) 
    {
        case EKTIFF_UBYTE:
            {
                uint8* uint8Vec = new uint8[dEntry.count] ;
                if (dEntry.count == 1)
                    *uint8Vec = dynamic_cast<TiffTagEntryT<uint8>*>(tagEntry)->getValue();
                else
                {
                    vector<uint8>& tagVec =
                        dynamic_cast<TiffTagEntryT<vector<uint8> >*>(tagEntry)->getValue() ;
                    TIFF_COPY( tagVec.begin(), tagVec.end(), uint8Vec ) ;
                }

                writeByteArray(dEntry, reinterpret_cast<int8*>(uint8Vec));
                delete [] uint8Vec ;

                break ;
            }

        case EKTIFF_BYTE:
        case EKTIFF_UNDEFINED:
            {
                int8* int8Vec = new int8[dEntry.count] ;
                if (dEntry.count == 1)
                    *int8Vec = dynamic_cast<TiffTagEntryT<int8>*>(tagEntry)->getValue();
                else
                {
                    vector<int8>& tagVec =
                        dynamic_cast<TiffTagEntryT<vector<int8> >*>(tagEntry)->getValue() ;
                    TIFF_COPY( tagVec.begin(), tagVec.end(), int8Vec ) ;
                }

                writeByteArray(dEntry, int8Vec);
                delete [] int8Vec ;

                break ;
            }
        
        case EKTIFF_USHORT:
            {
                uint16* uint16Vec = new uint16[dEntry.count] ;
                if (dEntry.count == 1)
                    *uint16Vec = dynamic_cast<TiffTagEntryT<uint16>*>(tagEntry)->getValue();
                else
                {
                    vector<uint16>& tagVec =
                        dynamic_cast<TiffTagEntryT<vector<uint16> >*>(tagEntry)->getValue() ;
                    TIFF_COPY( tagVec.begin(), tagVec.end(), uint16Vec ) ;
                }

                writeShortArray(dEntry, reinterpret_cast<int16*>(uint16Vec));
                delete [] uint16Vec ;

                break ;
            }
        
        case EKTIFF_SHORT:
            {
                int16* int16Vec = new int16[dEntry.count] ;
                if (dEntry.count == 1)
                    *int16Vec = dynamic_cast<TiffTagEntryT<int16>*>(tagEntry)->getValue();
                else
                {
                    vector<int16>& tagVec =
                        dynamic_cast<TiffTagEntryT<vector<int16> >*>(tagEntry)->getValue() ;
                    TIFF_COPY( tagVec.begin(), tagVec.end(), int16Vec ) ;
                }
                writeShortArray(dEntry, int16Vec);
                delete [] int16Vec ;

                break ;
            }
        
        case EKTIFF_ULONG:
            {
                tiff_uint32* tiff_uint32Vec = new tiff_uint32[dEntry.count] ;
                if (dEntry.count == 1)
                    *tiff_uint32Vec = dynamic_cast<TiffTagEntryT<tiff_uint32>*>(tagEntry)->getValue();
                else
                {
                    vector<tiff_uint32>& tagVec =
                        dynamic_cast<TiffTagEntryT<vector<tiff_uint32> >*>(tagEntry)->getValue() ;
                    TIFF_COPY( tagVec.begin(), tagVec.end(), tiff_uint32Vec ) ;
                }

                writeLongArray(dEntry, reinterpret_cast<tiff_int32*>(tiff_uint32Vec));
                delete [] tiff_uint32Vec ;

                break ;
            }
        
        case EKTIFF_LONG:
            {
                tiff_int32* tiff_int32Vec = new tiff_int32[dEntry.count] ;
                if (dEntry.count == 1)
                    *tiff_int32Vec = dynamic_cast<TiffTagEntryT<tiff_int32>*>(tagEntry)->getValue();
                else
                {
                    vector<tiff_int32>& tagVec =
                        dynamic_cast<TiffTagEntryT<vector<tiff_int32> >*>(tagEntry)->getValue() ;
                    TIFF_COPY( tagVec.begin(), tagVec.end(), tiff_int32Vec ) ;
                }
                writeLongArray(dEntry, tiff_int32Vec);
            
                delete [] tiff_int32Vec ;

                break ;
            }
        
        case EKTIFF_URATIONAL:
            {
                tiff_uint32* rat = new tiff_uint32[2*dEntry.count] ;

                if ( dEntry.count == 1 )
                {
                    float fval = dynamic_cast<TiffTagEntryT<float>*>(tagEntry)->getValue() ;
                    rationalize( fval, *(rat), *(rat+1) ) ;
                }          
                else
                {
                    vector<float>& tagVec =
                        dynamic_cast<TiffTagEntryT<vector<float> >*>(tagEntry)->getValue() ;
                    vector<float>::iterator crnt, end ;
                    crnt = tagVec.begin() ;
                    end = tagVec.end() ;
                    int j = 0 ;
                    while( crnt != end )
                    {
                        rationalize( *crnt, *(rat+j), *(rat+j+1) ) ;
                        j += 2 ;
                        crnt++ ;
                    }
                }

                writeRationalArray(dEntry, reinterpret_cast<tiff_int32*>(rat));

                delete [] rat ;

                break ;
            }

        case EKTIFF_RATIONAL:
            {
                tiff_int32* rat = new tiff_int32[2*dEntry.count] ;

                if ( dEntry.count == 1 )
                {
                    float fval = dynamic_cast<TiffTagEntryT<float>*>(tagEntry)->getValue() ;
                    bool setSign;
                    if ( fval < 0.0 )
                    {
                        setSign = true ;
                        fval = -fval ;
                    }
                    else
                        setSign = false ;

                    rationalize( fval, *reinterpret_cast<tiff_uint32*>((rat)), 
                                 *reinterpret_cast<tiff_uint32*>((rat+1)) ) ;

                    if ( setSign )
                        *(rat) = -*(rat) ;
                }          
                else
                {
                    vector<float>& tagVec =
                        dynamic_cast<TiffTagEntryT<vector<float> >*>(tagEntry)->getValue() ;
                    vector<float>::iterator crnt, end ;
                    crnt = tagVec.begin() ;
                    end = tagVec.end() ;
                    int j = 0 ;
                    bool setSign ;
                    while( crnt != end )
                    {
                        if ( *crnt < 0.0 )
                        {
                            setSign = true ;
                            *crnt = -*crnt ;
                        }
                        else
                            setSign = false ;
                  
                        rationalize( *crnt, *reinterpret_cast<tiff_uint32*>((rat+j)), 
                                     *reinterpret_cast<tiff_uint32*>((rat+j+1)) ) ;

                        if ( setSign )
                            *(rat+j) = -*(rat+j) ;
                  
                        j += 2 ;
                        crnt++ ;
                    }
                }

                writeRationalArray(dEntry, rat);

                delete [] rat ;

                break ;
            }

        case EKTIFF_FLOAT:
            {
                float* floatVec = new float[dEntry.count] ;
                if (dEntry.count == 1)
                    *floatVec = dynamic_cast<TiffTagEntryT<float>*>(tagEntry)->getValue();
                else
                {
                    vector<float>& tagVec =
                        dynamic_cast<TiffTagEntryT<vector<float> >*>(tagEntry)->getValue() ;
                    TIFF_COPY( tagVec.begin(), tagVec.end(), floatVec ) ;
                }

                writeFloatArray(dEntry, floatVec);

                delete [] floatVec ;

                break ;
            }
        
        case EKTIFF_DOUBLE:
            {
                double* doubleVec = new double[dEntry.count] ;
          
                if ( dEntry.count == 1 )
                    *doubleVec = dynamic_cast<TiffTagEntryT<double>*>(tagEntry)->getValue() ;
                else
                {
                    vector<double>& tagVec =
                        dynamic_cast<TiffTagEntryT<vector<double> >*>(tagEntry)->getValue() ;
                    TIFF_COPY( tagVec.begin(), tagVec.end(), doubleVec ) ;
                }

                writeDoubleArray(dEntry, doubleVec);
                delete [] doubleVec ;

                break ;
            }
        
        case EKTIFF_ASCII:
            {
                int8* int8Vec = new int8[dEntry.count];
                string stng = dynamic_cast<TiffTagEntryT<string>*>(tagEntry)->getValue() ;
                TIFF_COPY( stng.begin(), stng.end(), int8Vec ) ;
                int8Vec[dEntry.count - 1] = '\0' ;
                writeByteArray(dEntry, int8Vec);
                delete [] int8Vec ;
          
                break ;
            }
    }

    // write dir entry
    tiffio->swabShort(&dEntry.tag);
    tiffio->swabShort(&dEntry.type);
    tiffio->swabLong(&dEntry.count);
    tiffio->swabLong(&dEntry.offset);
    tiffio->seek( entryOffset, SEEK_SET ) ;
    tiffio->write( &dEntry, sizeof(TiffDirEntry) ) ;
    entryOffset += sizeof(TiffDirEntry) ;

    return ;
}


void TiffDirectory::writeByteArray(TiffDirEntry& dirEntry, int8* dp)
{
    if (dirEntry.count > 4)
    {
        // Size of value is too large to fit in the 4 byte
        // lenght of the entry's offset. So, write it out
        // somewhere else, and store the offset to the value
        // in TiffDirEntry's offset element.
        dirEntry.offset = tiffio->seek( dataOffset, SEEK_SET ) ;
        tiffio->write(dp, dirEntry.count);
        dataOffset = tiffio->seek(0, SEEK_END);
    }
    else
    {
        // Copy directly into the offset element of
        // the TiffDirEntry structure since it will
        // fit.
        memcpy(&dirEntry.offset, dp, dirEntry.count);        
    }
}


void TiffDirectory::writeShortArray(TiffDirEntry& dirEntry, int16* dp)
{
    if (dirEntry.count > 2)
    {
        dirEntry.offset = tiffio->seek( dataOffset, SEEK_SET ) ;
        tiffio->swabArrayOfShort((uint16*)dp, dirEntry.count);
        tiffio->write(dp, dirEntry.count*sizeof(int16));
        dataOffset = tiffio->seek(0, SEEK_END);
    }
    else
    {
        if (tiffio->bigEndian())
        {
            dirEntry.offset = (tiff_uint32) ((long) dp[0] << 16);
            if (dirEntry.count == 2)
                dirEntry.offset |= dp[1] & 0xffff;
        }
        else
        {
            dirEntry.offset =  dp[0] & 0xffff;
            if (dirEntry.count == 2)
                dirEntry.offset |= (long) dp[1] << 16;
        }
    }
}


void TiffDirectory::writeLongArray(TiffDirEntry& dirEntry, tiff_int32* dp)
{
    if (dirEntry.count > 1)
    {
        dirEntry.offset = tiffio->seek( dataOffset, SEEK_SET ) ;
        tiffio->swabArrayOfLong((tiff_uint32*)dp, dirEntry.count);
        tiffio->write(dp, dirEntry.count*sizeof(tiff_int32));
        dataOffset = tiffio->seek(0, SEEK_END);
    }
    else
    {
        dirEntry.offset = dp[0];
    }
}


void TiffDirectory::writeRationalArray(TiffDirEntry& dirEntry, tiff_int32* dp)
{
    dirEntry.offset = tiffio->seek( dataOffset, SEEK_SET ) ;
    tiffio->swabArrayOfLong((tiff_uint32*)dp, 2*dirEntry.count);
    tiffio->write(dp, 2*dirEntry.count*sizeof(tiff_int32));
    dataOffset = tiffio->seek(0, SEEK_END);

}


void TiffDirectory::writeFloatArray(TiffDirEntry& dirEntry, float* dp)
{
    if (dirEntry.count > 1)
    {
        dirEntry.offset = tiffio->seek( dataOffset, SEEK_SET ) ;
        tiffio->swabArrayOfLong((tiff_uint32*)dp, dirEntry.count);
        tiffio->write(dp, dirEntry.count*sizeof(float));
        dataOffset = tiffio->seek(0, SEEK_END);
    }
    else
    {
        dirEntry.offset = *(tiff_uint32*)&dp[0];
    }
}


void TiffDirectory::writeDoubleArray(TiffDirEntry& dirEntry, double* dp)
{
    dirEntry.offset = tiffio->seek( dataOffset, SEEK_SET ) ;
//    TIFFCvtNativeToIEEEDouble(tif, n, v);
    tiffio->swabArrayOfDouble(dp, dirEntry.count);
    tiffio->write(dp, dirEntry.count*sizeof(double));
    dataOffset = tiffio->seek(0, SEEK_END);
}


void
TiffDirectory::rationalize( const double& fpNum, tiff_uint32& numer, tiff_uint32& denom, const double error )
{
    // This algorithm will not work with negative floating point
    // numbers - so record the sign and set the calculated numerator's
    // sign at end.
    if (fpNum != 0)
    {
        if ( fpNum < 0.0 || fpNum < 3.0e-10 || fpNum > 4.0e+10 || error < 0.0 )
        {
            char errMsg[256];
            sprintf(errMsg, "Cannot convert floating point number to rational: %e", fpNum);
            throw TiffError(TIFF_DIRECTORY_WRITE, errMsg, EKTIFF_WHERE);
        }
    }
  
    tiff_uint32 D, N ;
    double epsilon = 0.0 ;

    denom = D = 1;
    numer = static_cast<int>(fpNum) ;
    N = numer + 1;

    while( true )
    {
        double r = 0.0;
        if ( fpNum * (denom) != static_cast<double>(numer) )
        {
            r = (N - fpNum * D)/( (fpNum * denom) - numer);
            if (r <= 1.0)
            {
                unsigned long t = N;
                N = numer;
                numer = t;
                t = D;
                D = denom;
                denom = t;
            }
        }
        else
            break ;
      
        epsilon = fabs( 1.0 - ( numer / ( fpNum * denom )) );
        if (epsilon <= error)
            break ;

        double m = 10.0;
        while( m * epsilon < 1.0)
            m *= 10.0;

        epsilon = 1.0/m * ((int)(0.5 + m*epsilon));

        if (epsilon <= error)
            break ;
 
        if ( r < 1.0 )
            r = 1.0/r ;
      
        N += numer * static_cast<unsigned long>(r);
        D += denom * static_cast<unsigned long>(r);
        numer += N;
        denom += D;

    }

    return ;
}

int
TiffDirectory::readDir( toff_t offset, TiffDirEntry*& dirEntries )
{
    uint16 dircount ;

    tiffio->seek( offset, SEEK_SET ) ;
    tiffio->read( &dircount, SIZEOF_UINT16 ) ;
    tiffio->swabShort( &dircount ) ;
  
    if ( (dirEntries = new TiffDirEntry[dircount]) == NULL )
    {
        string errMsg("Failed to allocate memory for directory entries.") ;
        throw TiffError(TIFF_DIRECTORY_WRITE, errMsg, EKTIFF_WHERE);
    }

    if ( (unsigned)(tiffio->read( dirEntries, dircount*sizeof(TiffDirEntry))) !=
         dircount*sizeof(TiffDirEntry) )
    {
        delete [] dirEntries ;
        dirEntries = NULL ;
        string errMsg("Read of directory entries from file failed.") ;
        throw TiffError(TIFF_DIRECTORY_WRITE, errMsg, EKTIFF_WHERE);
    }
    for (int i=0; i<dircount; i++)
    {
    tiffio->swabShort(&(dirEntries[i].tag));
    tiffio->swabShort(&(dirEntries[i].type));
    tiffio->swabLong(&(dirEntries[i].count));
    tiffio->swabLong(&(dirEntries[i].offset));
    }

    // Read offset to next directory for sequential scans.
    tiffio->read( &nextDirOffset, sizeof(tiff_uint32) );
    tiffio->swabLong(reinterpret_cast<tiff_uint32*>(&nextDirOffset));

    return dircount;
}


/*
 * Read the next TIFF directory from a file
 * and convert it to the internal format.
 * We read directories sequentially.
 */
void
TiffDirectory::readDirTree( void )
{
    tiffio->flags() &= ~TIFF_BEENWRITING;    /* reset before new dir */

    if (offset == 0)
        return;

    TiffDirEntry* dirEntries;
    int dircount = readDir( offset, dirEntries ) ;
  
    try
    {
        register TiffDirEntry* dp;
        register int n ;
        TiffTagEntry* tagEntry ;
        for( dp = dirEntries, n = dircount; n > 0; n--, dp++ ) 
        {
            tagEntry = fetchNormalTag( dp ) ;
            if ( tagEntry != NULL )
            {
                setGenericTag( *tagEntry ) ;
                delete tagEntry;
            }
        }
#ifdef STRIPCHOP_SUPPORT
        if (getGenericTag(TIFFTAG_IMAGEWIDTH) && getGenericTag(TIFFTAG_IMAGELENGTH) &&
            (getGenericTag(TIFFTAG_STRIPOFFSETS) || getGenericTag(TIFFTAG_TILEOFFSETS)))
        {
            tstrip_t nstrip = isTiled()? numOfTiles(): numOfStrips();
            if (tiffio->flags() & TIFF_STRIPCHOP && nstrip == 1 &&
                compression() == COMPRESSION_NONE)
                chopupSingleUncompressedStrip();
        }
#endif
    }
    catch(...)
    {
        delete [] dirEntries ;
        throw ;
    }
  
    delete [] dirEntries ;
  
    SubDirIter crntSubDir, endSubDir ;
    crntSubDir = subDirs.begin() ;
    endSubDir = subDirs.end() ;
    while( crntSubDir != endSubDir )
    {
        DirVec dirVec = (*crntSubDir).second;
        for (unsigned int i=0; i<dirVec.size(); i++)
        {
            dirVec[i]->readDirTree();
        }

        crntSubDir++;
    }
  
    return ;
}

void 
TiffDirectory::deleteItems( void )
{
  
    EntryIter crntEntry, endEntry ;
    crntEntry = tagEntries.begin() ;
    endEntry = tagEntries.end() ;

    while( crntEntry != endEntry )
    {
        delete (*crntEntry).second ;
        crntEntry++  ;
    }
    tagEntries.clear();
  
    SubDirIter crntSubDir, endSubDir ;
    crntSubDir = subDirs.begin() ;
    endSubDir = subDirs.end() ;
    while( crntSubDir != endSubDir )
    {
        DirVec& dirVec = (*crntSubDir).second;
        for (unsigned int i=0; i<dirVec.size(); i++)
        {
            delete dirVec[i];
        }
        dirVec.clear();
        crntSubDir++;
    }
    subDirs.clear();
  
    if (tiffimg)
        delete tiffimg;

    return ;
}



void
TiffDirectory::addSubDir( TiffTagEntry* tagEntry )
{
    // set subIFDs
    if ( tagEntry->getCount() > 1 )
    {
        TiffTagEntryT<vector<tiff_uint32> >* offsetVec = 
            dynamic_cast<TiffTagEntryT<vector<tiff_uint32> >*>(tagEntry) ;
      
        vector<tiff_uint32>::iterator crnt, end ;
        crnt = offsetVec->getValue().begin() ;
        end = offsetVec->getValue().end() ;
        int16 idx = 0 ;
        while( crnt != end )
        {
            TiffDirectory* subDir = new TiffDirectory( tiffio, tagEntry->getTagNum(), *crnt, idx ) ;
            addSubDir( subDir ) ;
            crnt++ ;
            idx++ ;
        }
    }
    else if (tagEntry->getCount() == 1)
    {
        TiffTagEntryT<tiff_uint32>* offset = dynamic_cast<TiffTagEntryT<tiff_uint32>*>(tagEntry) ;
        TiffDirectory* subDir = new TiffDirectory( tiffio, tagEntry->getTagNum(),  offset->getValue(), 0 ) ;
        addSubDir( subDir ) ;
    }

    return ;
}

/*
 * Fetch a tag that is not handled by special case code.
 */
TiffTagEntry*
TiffDirectory::fetchNormalTag( TiffDirEntry* dp )
{

    TiffTagEntry *tagEntry = NULL ;

    switch( dp->type ) 
    {
        case EKTIFF_UBYTE:
            {
                uint8* uint8Vec = reinterpret_cast<uint8*>( fetchByteArray( dp ) ) ;
                if ( dp->count == 1 )
                    tagEntry = new TiffTagEntryT<uint8>(dp->tag, dp->type, dp->count, *uint8Vec) ;
                else
                    tagEntry = new TiffTagEntryT<vector<uint8> >(dp->tag, dp->type, dp->count, vector<uint8>(uint8Vec, (uint8Vec+dp->count))) ;
                delete [] uint8Vec ;
                break ;
            }
        

        case EKTIFF_BYTE:
            {
                int8* int8Vec = fetchByteArray( dp );
                if ( dp->count == 1 )
                    tagEntry = new TiffTagEntryT<int8>(dp->tag, dp->type, dp->count, *int8Vec) ;
                else
                    tagEntry = new TiffTagEntryT<vector<int8> >(dp->tag, dp->type, dp->count, vector<int8>(int8Vec, (int8Vec+dp->count))) ;
                delete [] int8Vec ;
                break ;
            }
        
        case EKTIFF_USHORT:
            {
                uint16* uint16Vec = reinterpret_cast<uint16*>( fetchShortArray( dp ) ) ;
                // Some tag can be USHORT or ULONG. For consistence reason,
                // we represent them as ULONG in TiffTagEntry.
                if (canBeUSHORTULONG(dp->tag))
                {
                    tiff_uint32* tiff_uint32Vec = new tiff_uint32[dp->count];
                    for (unsigned int i=0; i<dp->count; i++)
                        tiff_uint32Vec[i] = (tiff_uint32)(uint16Vec[i]);
                    if ( dp->count == 1 )
                        tagEntry = new TiffTagEntryT<tiff_uint32>(dp->tag, EKTIFF_ULONG, dp->count, *tiff_uint32Vec) ;
                    else
                        tagEntry = new TiffTagEntryT<vector<tiff_uint32> >(dp->tag, EKTIFF_ULONG, dp->count, vector<tiff_uint32>(tiff_uint32Vec, (tiff_uint32Vec+dp->count))) ;
                    delete[] tiff_uint32Vec;
                }
                else
                {
                    if ( dp->count == 1 )
                        tagEntry = new TiffTagEntryT<uint16>(dp->tag, dp->type, dp->count, *uint16Vec) ;
                    else
                        tagEntry = new TiffTagEntryT<vector<uint16> >(dp->tag, dp->type, dp->count, vector<uint16>(uint16Vec, (uint16Vec+dp->count))) ;
                }
                delete [] uint16Vec ;
                break ;
            }
        
        case EKTIFF_SHORT:
            {
                int16* int16Vec = fetchShortArray( dp ) ;
                if ( dp->count == 1 )
                    tagEntry = new TiffTagEntryT<int16>(dp->tag, dp->type, dp->count, *int16Vec) ;
                else
                    tagEntry = new TiffTagEntryT<vector<int16> >(dp->tag, dp->type, dp->count, vector<int16>(int16Vec, (int16Vec+dp->count))) ;
                delete [] int16Vec ;
                break ;
            }
        
        case EKTIFF_ULONG:
            {
                tiff_uint32* tiff_uint32Vec = reinterpret_cast<tiff_uint32*>( fetchLongArray( dp ) ) ;
                if ( dp->count == 1 )
                    tagEntry = new TiffTagEntryT<tiff_uint32>(dp->tag, dp->type, dp->count, *tiff_uint32Vec) ;
                else
                    tagEntry = new TiffTagEntryT<vector<tiff_uint32> >(dp->tag, dp->type, dp->count, vector<tiff_uint32>(tiff_uint32Vec, (tiff_uint32Vec+dp->count))) ;
                delete [] tiff_uint32Vec ;
                break ;
            }
        
        case EKTIFF_LONG:
            {
                tiff_int32* tiff_int32Vec = fetchLongArray( dp ) ;
                if ( dp->count == 1 )
                    tagEntry = new TiffTagEntryT<tiff_int32>(dp->tag, dp->type, dp->count, *tiff_int32Vec) ;
                else
                    tagEntry = new TiffTagEntryT<vector<tiff_int32> >(dp->tag, dp->type, dp->count, vector<tiff_int32>(tiff_int32Vec, (tiff_int32Vec+dp->count))) ;
                delete [] tiff_int32Vec ;
                break ;
            }
        
        case EKTIFF_URATIONAL:
        case EKTIFF_RATIONAL:
            {
                float* floatVec = fetchRationalArray( dp ) ;
                if ( dp->count == 1 )
                    tagEntry = new TiffTagEntryT<float>(dp->tag, dp->type, dp->count, *floatVec) ;
                else
                    tagEntry = new TiffTagEntryT<vector<float> >(dp->tag, dp->type, dp->count, vector<float>(floatVec, (floatVec+dp->count))) ;
                delete [] floatVec ;
                break ;
            }
        
        case EKTIFF_FLOAT:
            {
                float* floatVec = fetchFloatArray( dp ) ;
                if ( dp->count == 1 )
                    tagEntry = new TiffTagEntryT<float>(dp->tag, dp->type, dp->count, *floatVec) ;
                else
                    tagEntry = new TiffTagEntryT<vector<float> >(dp->tag, dp->type, dp->count, vector<float>(floatVec, (floatVec+dp->count))) ;
                delete [] floatVec ;
                break ;
            }
        
        case EKTIFF_DOUBLE:
            {
                double* doubleVec = fetchDoubleArray( dp ) ;
                if ( dp->count == 1 )
                    tagEntry = new TiffTagEntryT<double>(dp->tag, dp->type, dp->count, *doubleVec) ;
                else
                    tagEntry = new TiffTagEntryT<vector<double> >(dp->tag, dp->type, dp->count, vector<double>(doubleVec, (doubleVec+dp->count))) ;
                delete [] doubleVec ;
                break ;
            }
        
        case EKTIFF_ASCII:
            {
                int8* int8Vec = fetchByteArray( dp ) ;
                tagEntry = new TiffTagEntryT<string>(dp->tag, dp->type, dp->count, string(int8Vec, dp->count-1)) ;
                delete [] int8Vec ;
                break ;
            }
        
        case EKTIFF_UNDEFINED:
            {
                int8* int8Vec = static_cast<int8*>( fetchByteArray( dp ) ) ;
        if (dp->count == 1)
                    tagEntry = new TiffTagEntryT<int8>(dp->tag, dp->type, dp->count, *int8Vec) ;
        else
                    tagEntry = new TiffTagEntryT<vector<int8> >(dp->tag, dp->type, dp->count, vector<int8>(int8Vec, (int8Vec+dp->count))) ;
                delete [] int8Vec ;
                break ;
            }
    }

    return( tagEntry );
}



/*
 * Fetch a contiguous directory item.
 */
void
TiffDirectory::fetchData( toff_t offset, int8* dataPtr, tiff_uint32 byteSize )
{
    tiffio->seek( offset, SEEK_SET ) ;
    tiffio->read( dataPtr, byteSize ) ;
  
    return ;
}


/*
 * Fetch an array of UBYTE or BYTE values.
 */
int8*
TiffDirectory::fetchByteArray( TiffDirEntry* dir )
{
    int8* vec = new int8[ dir->count ] ;
  
    if ( dir->count < 5 ) 
    {
        // Extract data from offset field.
        if ( tiffio->bigEndian() ) 
        {
            switch( dir->count ) 
            {
                case 4: vec[3] = (int8)(dir->offset & 0xff);
                case 3: vec[2] = (int8)((dir->offset >> 8) & 0xff);
                case 2: vec[1] = (int8)((dir->offset >> 16) & 0xff);
                case 1: vec[0] = (int8)(dir->offset >> 24);
            }
        } 
        else 
        {
            switch( dir->count ) 
            {
                case 4: vec[3] = (int8)(dir->offset >> 24);
                case 3: vec[2] = (int8)((dir->offset >> 16) & 0xff);
                case 2: vec[1] = (int8)((dir->offset >> 8) & 0xff);
                case 1: vec[0] = (int8)(dir->offset & 0xff);
            }
        }
    } 
    else
        fetchData( dir->offset, static_cast<int8*>( vec ), dir->count ) ;

    return vec ;
}

/*
 * Fetch an array of USHORT or SHORT values.
 */
int16*
TiffDirectory::fetchShortArray( TiffDirEntry* dir )
{
    int16* vec = new int16[dir->count] ;
  
    if ( dir->count <= 2 ) 
    {
        if ( tiffio->bigEndian() ) 
        {
            switch (dir->count) 
            {
                case 2: vec[1] = (int16)(dir->offset & 0xffff);
                case 1: vec[0] = (int16)(dir->offset >> 16);
            }
        } 
        else 
        {
            switch (dir->count) 
            {
                case 2: vec[1] = (int16)(dir->offset >> 16);
                case 1: vec[0] = (int16)(dir->offset & 0xffff);
            }
        }
    } 
    else
    {
        fetchData( dir->offset, reinterpret_cast<int8*>(vec), dir->count*sizeof(int16) ) ;
        tiffio->swabArrayOfShort(  reinterpret_cast<uint16*>(vec), dir->count );
    }
  
    return vec ;
}

/*
 * Fetch an array of LONG or SLONG values.
 */
tiff_int32*
TiffDirectory::fetchLongArray( TiffDirEntry* dir )
{
    tiff_int32* vec = new tiff_int32[dir->count] ;
  
    if ( dir->count == 1 ) 
        vec[0] = dir->offset;
    else
    {
        fetchData( dir->offset, reinterpret_cast<int8*>(vec), dir->count*sizeof(tiff_int32) ) ;
    tiffio->swabArrayOfLong( reinterpret_cast<tiff_uint32*>(vec), dir->count);
    }

    return vec ;
}


/*
 * Convert numerator+denominator to float.
 */
void
TiffDirectory::convertRational( TiffDirEntry* dir, tiff_uint32 num, tiff_uint32 denom, float* rv)
{
    if (num == 0 || denom == 0) {
        *rv = (float)0.0;
        return;
    }

    if (denom == 0) 
    {
        char errMsg[100] ;
        sprintf(errMsg, "Rational with zero denominator in tag %d", dir->tag);      
        throw TiffError(TIFF_DIRECTORY_READ, errMsg, EKTIFF_WHERE);
    } 
    else 
    {
        if ( dir->type == EKTIFF_RATIONAL )
           // *rv = static_cast<float>(num) / static_cast<float>(denom) ;
           
        *rv = ((float)(tiff_int32)num / (float)(tiff_int32)denom);
        else
          //  *rv = static_cast<float>(static_cast<tiff_int32>(num)) /
           //     static_cast<float>(static_cast<tiff_int32>(denom)) ;
           *rv = ((float)num / (float)denom);
           
    }
    return  ;
}

/*
 * Fetch an array of RATIONAL or SRATIONAL values.
 */
float*
TiffDirectory::fetchRationalArray( TiffDirEntry* dir )
{
    float* vec = new float[dir->count] ;
    tiff_uint32* l = new tiff_uint32[2*dir->count] ;
    
    fetchData( dir->offset, reinterpret_cast<int8*>(l), 2*dir->count*sizeof(tiff_uint32) ) ;

    tiffio->swabArrayOfLong( l, 2*dir->count);

    tiff_uint32 i;
    try
    {
        for (i = 0; i < dir->count; i++)
            convertRational( dir, l[2*i+0], l[2*i+1], (vec+i));
    }
    catch(...)
    {
        delete [] vec ;
        delete [] l ;
        throw ;
    }
  

    delete [] l ;

    return vec ;
}

/*
 * Fetch an array of FLOAT values.
 */
float*
TiffDirectory::fetchFloatArray( TiffDirEntry* dir )
{
    float* vec = new float[dir->count] ;

    if (dir->count == 1) 
        vec[0] = *(float*) &dir->offset;
    else
        fetchData( dir->offset, reinterpret_cast<int8 *>(vec), dir->count*sizeof(float) ) ;

    tiffio->swabArrayOfLong( reinterpret_cast<tiff_uint32*>(vec), dir->count);

    // TIFFCvtIEEEFloatToNative( dir->count, vec );
  
    return vec ;
}

/*
 * Fetch an array of DOUBLE values.
 */
double*
TiffDirectory::fetchDoubleArray( TiffDirEntry* dir )
{
    double* vec = new double[dir->count] ;

    fetchData( dir->offset, reinterpret_cast<int8*>(vec), dir->count*sizeof(double) ) ;

    tiffio->swabArrayOfDouble( vec, dir->count );

    // TIFFCvtIEEEDoubleToNative( dir->count, vec );
  
    return vec ;
}


TiffImage* TiffDirectory::getImage()
{
    if (!tiffimg) 
    {
        if (isTiled())
            tiffimg = new TiffTileImage(this, tiffio);
        else 
            tiffimg = new TiffStripImage(this, tiffio);

        try
        {
            tiffimg->init();
        }
        catch (...)
        {
            delete tiffimg;
            tiffimg = NULL;
        }
    }

    return tiffimg;
}


bool TiffDirectory::canBeUSHORTULONG(uint16 tagNum) const
{
    if (tagNum == TIFFTAG_IMAGEWIDTH || tagNum == TIFFTAG_IMAGELENGTH ||
        tagNum == TIFFTAG_TILEWIDTH || tagNum == TIFFTAG_TILELENGTH ||
        tagNum == TIFFTAG_ROWSPERSTRIP || tagNum == TIFFTAG_STRIPOFFSETS ||
        tagNum == TIFFTAG_STRIPBYTECOUNTS || tagNum == TIFFTAG_TILEBYTECOUNTS)
        return true;
    else
        return false;
}


ostream& TiffDirectory::print(ostream& c, bool bPrintSubDirs) const
{
    c << "Dir Index:" << index << " Tag Num:" << tag;
    c << " Offset:" << offset << " Next:" << nextDirOffset << endl;
    c << "Num of Dir Entries:" << tagEntries.size() << endl;

    ConstEntryIter crntEntry, endEntry ;
    crntEntry = tagEntries.begin() ;
    endEntry = tagEntries.end() ;
    while (crntEntry != endEntry)
    {
        ((*crntEntry).second)->print(c);
        c << endl;
        crntEntry++;
    }

    printf("\n");

    if (bPrintSubDirs)
    {
        const SubDirMap subDirs = getSubDirs();
        ConstSubDirIter s = subDirs.begin();
        while (s != subDirs.end())
        {
            DirVec dirVec = (*s).second;
            for (unsigned int i=0; i<dirVec.size(); i++)
            {
                dirVec[i]->print(c);
            }
            s ++;
        }
    }

    return c;
}


const TiffTagEntry* TiffDirectory::getDefaultTag(ttag_t tagNum ) const
{
    ConstEntryIter entryIter ;
    if ( (entryIter = tagEntries.find( tagNum )) == tagEntries.end() )
    {
        if ( (entryIter = defaultEntries.map.find( tagNum )) == defaultEntries.map.end() )
            return static_cast<TiffTagEntry*>(NULL) ;
        else
            return (*entryIter).second;
    }

    return (*entryIter).second ;
}


void 
TiffDirectory::setGenericTag( const TiffTagEntry& tagEntry )
{ 
    // remove the pre-set tagEntry if any
    TiffTagEntry* entry = tagEntry.clone();

    removeGenericTag(entry->getTagNum());

    pair<EntryIter, bool> rtnVal =
        tagEntries.insert(EntryMap::value_type(entry->getTagNum(), entry));
    assert(rtnVal.second);

    // set tile flag
    if (tagEntry.getTagNum() == TIFFTAG_TILEWIDTH ||
        tagEntry.getTagNum() == TIFFTAG_TILELENGTH )
        tiffio->flags() |= TIFF_ISTILED;

    // if it is a subIFD tag
    SubDirVecIter subDirTag = find( subDirTags.begin(), subDirTags.end(), entry->getTagNum() ) ;
    if ( subDirTag != subDirTags.end() )
        addSubDir( entry ) ;
    
    return ; 
}

TiffTagEntry* 
TiffDirectory::getGenericTag(ttag_t tagNum ) const 
{
    ConstEntryIter entryIter ;
    if ( (entryIter = tagEntries.find( tagNum )) == tagEntries.end() )
    {
//      if ( (entryIter = defaultEntries.find( tagNum )) == defaultEntries.end() )
        return static_cast<TiffTagEntry*>(NULL) ;
//      return (*entryIter).second->clone() ;
    }

    return (*entryIter).second ;
}


bool TiffDirectory::removeGenericTag(ttag_t tag)
{
    EntryIter it = tagEntries.find(tag);
    if (it != tagEntries.end())
    {
        delete (*it).second;
        tagEntries.erase(it);

        DirVec dirVec;
        // if a subIFD tag
        if (getSubDirVec(tag, dirVec))
        {
            for (unsigned int i=0; i<dirVec.size(); i++)
            {
                delete dirVec[i];
            }
            subDirs.erase(tag);
        }
    
        return true;
    }

    return false;
}


bool TiffDirectory::getSubDirVec(ttag_t tag, DirVec& dirVec)
{
    dirVec.clear();
    SubDirIter it = subDirs.find(tag);
    if (it != subDirs.end())
    {
        dirVec = (*it).second;
        return true;
    }
    else
    {
        return false;
    }
}


void 
TiffDirectory::addSubDir( TiffDirectory* subDir )
{ 
    SubDirIter s = subDirs.find(subDir->getTagNum());
    if (s != subDirs.end())
    {
        (*s).second.push_back(subDir);
    }
    else
    {
        DirVec val;
        val.push_back(subDir);
        subDirs.insert(SubDirMap::value_type(subDir->getTagNum(), val));
    }
}

void TiffDirectory::addSubDir(ttag_t tag)
{
    SubDirVecIter subDirTag = find(subDirTags.begin(), subDirTags.end(), tag) ;
    if ( subDirTag == subDirTags.end())
    {
        char msg[100];
        sprintf(msg, "tag %d is not a sub IFD tag", tag);
        throw TiffError(TIFF_ERROR, msg, EKTIFF_WHERE);
    }

    // adding tag
    TiffTagEntry* tte = getGenericTag(tag);
    if (tte)
    {
        vector<tiff_uint32> offs;
        if (tte->getCount() == 1)
        {
            offs.push_back((dynamic_cast<TiffTagEntryT<tiff_uint32>*>(tte))->getValue());
        }
        else 
        {
            offs = (dynamic_cast<TiffTagEntryT<vector<tiff_uint32> >*>(tte))->getValue();
        }
        offs.push_back(0);
        
        tagEntries.erase(tag);
        delete tte;
        tte = new TiffTagEntryT<vector<tiff_uint32> >(tag, EKTIFF_ULONG, offs.size(), offs);
        tagEntries.insert(EntryMap::value_type(tag, tte));
    }
    else
    {
        tte = new TiffTagEntryT<tiff_uint32>(tag, EKTIFF_ULONG, 1, 0);
        tagEntries.insert(EntryMap::value_type(tag, tte));
    }

    // adding sub dir
    SubDirIter s = subDirs.find(tag);
    if (s != subDirs.end())
    {
        TiffDirectory* subDir = new TiffDirectory(tiffio, tag, (*s).second.size()) ;
        (*s).second.push_back(subDir);
    }
    else
    {
        TiffDirectory* subDir = new TiffDirectory(tiffio, tag, 0) ;
        DirVec val;
        val.push_back(subDir);
        subDirs.insert(SubDirMap::value_type(subDir->getTagNum(), val));
    }
}


TiffDirectory* TiffDirectory::getSubDir(ttag_t tag, tiff_uint32 idx)
{
    DirVec dirVec;
    if (getSubDirVec(tag, dirVec))
    {
        if (idx < dirVec.size())
            return (dirVec[idx]);
        else 
            return 0;
    }
    else
        return 0;
}


bool TiffDirectory::selfGenerated(ttag_t tag) const
{
    if (tag == TIFFTAG_STRIPOFFSETS || tag == TIFFTAG_STRIPBYTECOUNTS ||
        tag == TIFFTAG_TILEOFFSETS || tag == TIFFTAG_TILEBYTECOUNTS)
        return true;
    else 
        return false;
}

void TiffDirectory::copy(const TiffDirectory& tifdir)
{
    const EntryMap& entryMap = tifdir.getTagEntries();
    const SubDirMap& dirMap = tifdir.getSubDirs();

    // copy tag entries
    ConstEntryIter s = entryMap.begin();
    while (s != entryMap.end())
    {
        if (!selfGenerated((*s).first))
        {
            setGenericTag(*((*s).second));
        }

        s++;
    }

    // copy sub dirs
    ConstSubDirIter ss = dirMap.begin();
    while (ss != dirMap.end())
    {
        DirVec dirVec1 = (*ss).second;
        DirVec dirVec2;
        getSubDirVec((*ss).first, dirVec2);
        assert(dirVec1.size() == dirVec2.size());
        for (unsigned int i=0; i<dirVec1.size(); i++)
        {
            TiffDirectory* dir1 = dirVec1[i];
            TiffDirectory* dir2 = dirVec2[i];
            assert(dir1);
            assert(dir2);
            dir2->copy(*dir1);
        }
        ss ++;
    }
}


tsize_t TiffDirectory::readFile(tdata_t buf, toff_t off, tsize_t size) const
{
    toff_t orig_off = tiffio->seek(0, SEEK_CUR);
    tiffio->seek(off, SEEK_SET);
    tsize_t ret = tiffio->read(buf, size);
    if (ret == tsize_t(-1))
    {
        char msg[100];
        sprintf("Failed to read file: %s at offset %d for the amount %d", 
                tiffio->name().c_str(), off, size);
        throw TiffError(TIFF_IO_READ, msg, EKTIFF_WHERE);
    }
    tiffio->seek(orig_off, SEEK_SET);
    return ret;
}


tsize_t TiffDirectory::writeFile(tdata_t buf, tsize_t size, toff_t& off)
{
    off = tiffio->seek(0, SEEK_CUR);
    tsize_t ret = tiffio->write(buf, size);
    if (ret == tsize_t(-1))
    {
        char msg[100];
        sprintf("Failed to write file: %s, amount to write %d", 
                tiffio->name().c_str(), size);
        throw TiffError(TIFF_IO_READ, msg, EKTIFF_WHERE);
    }

    return ret;
}


tsize_t TiffDirectory::getAudioSize() const
{
    if (tag != TIFFTAG_SOUNDIFD)
    {
        throw TiffError(TIFF_ERROR, "Current IFD is not a sound IFD", EKTIFF_WHERE);
    }

    TiffTagEntry* entry = getGenericTag(259); // SoundByteCounts
    if (!entry)
        return 0;

    tsize_t size = 0;
    if (entry->getCount() > 1)
    {
        TiffTagEntryT<vector<tiff_uint32> >* p = dynamic_cast<TiffTagEntryT<vector<tiff_uint32> >*>(entry);
        if (!p)
            return 0;

        const vector<tiff_uint32>& cnts = p->getValue();

        for (unsigned int i=0; i<cnts.size(); i++)
            size += cnts[i];
    }
    else // count == 1
    {
        TiffTagEntryT<tiff_uint32>* p = dynamic_cast<TiffTagEntryT<tiff_uint32>*>(entry);
        if (!p)
            return 0;

        size = p->getValue();
    }

    return size;
}


tsize_t TiffDirectory::readAudio(tdata_t buf, tsize_t bufSize) const
{
    if (tag != TIFFTAG_SOUNDIFD)
    {
        throw TiffError(TIFF_ERROR, "Current IFD is not a sound IFD", EKTIFF_WHERE);
    }

    TiffTagEntry* entry = getGenericTag(259); // SoundByteCounts
    if (!entry)
        return 0;

    tsize_t size = 0;
    tsize_t amountRead = 0;
    if (entry->getCount() > 1)
    {
        TiffTagEntryT<vector<tiff_uint32> >* p = dynamic_cast<TiffTagEntryT<vector<tiff_uint32> >*>(entry);
        if (!p)
            return 0;

        const vector<tiff_uint32>& cnts = p->getValue();

        unsigned int i;
        for (i=0; i<cnts.size(); i++)
            size += cnts[i];

        if (size == 0)
            return 0;

        if (bufSize < size)
        {
            char msg[100];
            sprintf(msg, "Buffer size %d is not enough for audio size %d", bufSize, size);
            throw TiffError(TIFF_ERROR, msg, EKTIFF_WHERE);
        }

        entry = getGenericTag(260); // SoundOffset
        if (!entry)
            return 0;

        p = dynamic_cast<TiffTagEntryT<vector<tiff_uint32> >*>(entry);
        if (!p)
            return 0;

        const vector<tiff_uint32>& offsets = p->getValue();
        unsigned char *pBuf = (unsigned char*)buf;
        for (i=0; i<cnts.size(); i++)
        {
            amountRead += readFile(pBuf, offsets[i], cnts[i]);
            pBuf += cnts[i];
        }
    }
    else // count == 1
    {
        TiffTagEntryT<tiff_uint32>* p = dynamic_cast<TiffTagEntryT<tiff_uint32>*>(entry);
        if (!p)
            return 0;

        size = p->getValue();

        if (size == 0)
            return 0;

        if (bufSize < size)
        {
            char msg[100];
            sprintf(msg, "Buffer size %d is not enough for audio size %d", bufSize, size);
            throw TiffError(TIFF_ERROR, msg, EKTIFF_WHERE);
        }

        entry = getGenericTag(260); // SoundOffset
        if (!entry)
            return 0;

        p = dynamic_cast<TiffTagEntryT<tiff_uint32>*>(entry);
        if (!p)
            return 0;

        const tiff_uint32& offsets = p->getValue();
        amountRead = readFile(buf, offsets, size);
    }

    return amountRead;
}


tsize_t TiffDirectory::writeAudio(tdata_t buf, tsize_t size)
{
    if (tag != TIFFTAG_SOUNDIFD)
    {
        throw TiffError(TIFF_ERROR, "Current IFD is not a sound IFD", EKTIFF_WHERE);
    }

    toff_t off = 0;
    if (size == 0 || writeFile(buf, size, off) != size)
        return 0;

    // SoundByteCounts
    TiffTagEntry* entry = new TiffTagEntryT<tiff_uint32>(259, EKTIFF_ULONG, 1, size);
    setGenericTag(*entry);
    delete entry;
    
    // SoundOffsets
    entry = new TiffTagEntryT<tiff_uint32>(260, EKTIFF_ULONG, 1, off);
    setGenericTag(*entry);
    delete entry;

    return size;
}


tsize_t TiffDirectory::scanlineSize()
{
    if (isTiled())
        throw TiffError(TIFF_ERROR, "No scanline size for tile image", EKTIFF_WHERE);

    tsize_t scanline;
    
    scanline = bitsPerSample()[0] * imageWidth();
    if (planarConfig() == PLANARCONFIG_CONTIG)
        scanline *= samplesPerPixel();
    return ((tsize_t) TIFFhowmany(scanline, 8));
}


tsize_t TiffDirectory::tileRowSize()
{
    if (!isTiled())
        throw TiffError(TIFF_ERROR, "No tile row size for strip image", EKTIFF_WHERE);

    tsize_t rowsize;
    
    tiff_uint32 tilelength = tileLength();
    tiff_uint32 tilewidth = tileWidth();
    if (tilelength == 0 || tilewidth == 0)
        return ((tsize_t) 0);
    rowsize = (bitsPerSample())[0] * tilewidth;
    if (planarConfig() == PLANARCONFIG_CONTIG)
        rowsize *= samplesPerPixel();
    return ((tsize_t) TIFFhowmany(rowsize, 8));    
}


tsize_t TiffDirectory::vStripSize(tiff_uint32 nrows)
{
    if (isTiled())
        throw TiffError(TIFF_ERROR, "No vStripsize for tile image", EKTIFF_WHERE);

    if (nrows == (tiff_uint32) -1)
        nrows = imageLength();
#ifdef YCBCR_SUPPORT
    if (planarConfig() == PLANARCONFIG_CONTIG &&
        photoMetric() == PHOTOMETRIC_YCBCR &&
        !isUpSampled()) {
        /*
         * Packed YCbCr data contain one Cb+Cr for every
         * HorizontalSampling*VerticalSampling Y values.
         * Must also roundup width and height when calculating
         * since images that are not a multiple of the
         * horizontal/vertical subsampling area include
         * YCbCr data for the extended image.
         */
        vector<uint16> ycbcrsubsampling = yccSubSampling();

        tsize_t w =
            TIFFroundup(imageWidth(), ycbcrsubsampling[0]);
        tsize_t scanline = TIFFhowmany(w*(bitsPerSample())[0], 8);
        tsize_t samplingarea =
            ycbcrsubsampling[0]*ycbcrsubsampling[1];
        nrows = TIFFroundup(nrows, ycbcrsubsampling[1]);
        /* NB: don't need TIFFhowmany here 'cuz everything is rounded */
        return ((tsize_t)
                (nrows*scanline + 2*(nrows*scanline / samplingarea)));
    } else
#endif
        return ((tsize_t)(nrows * scanlineSize()));
}


tsize_t TiffDirectory::vTileSize(tiff_uint32 nrows)
{
    if (!isTiled())
        throw TiffError(TIFF_ERROR, "vTileSize for strip image", EKTIFF_WHERE);

    tsize_t tilesize = 0;

//    if (tiffdir->tileLength() == 0 || tiffdir->tileWidth() == 0 ||
//        tiffdir->tileDepth() == 0)
//        return ((tsize_t) 0);
#ifdef YCBCR_SUPPORT
    if (planarConfig() == PLANARCONFIG_CONTIG &&
        photoMetric() == PHOTOMETRIC_YCBCR &&
        !isUpSampled()) {
        /*
         * Packed YCbCr data contain one Cb+Cr for every
         * HorizontalSampling*VerticalSampling Y values.
         * Must also roundup width and height when calculating
         * since images that are not a multiple of the
         * horizontal/vertical subsampling area include
         * YCbCr data for the extended image.
         */
        vector<uint16> ycbcrsubsampling = yccSubSampling();
        tsize_t w =
            TIFFroundup(tileWidth(), ycbcrsubsampling[0]);
        tsize_t rowsize = TIFFhowmany(w*(bitsPerSample())[0], 8);
        tsize_t samplingarea =
            ycbcrsubsampling[0]*ycbcrsubsampling[1];
        nrows = TIFFroundup(nrows, ycbcrsubsampling[1]);
        /* NB: don't need TIFFhowmany here 'cuz everything is rounded */
        tilesize = nrows*rowsize + 2*(nrows*rowsize / samplingarea);
    } else
#endif
        tilesize = nrows * tileRowSize();
    return ((tsize_t)(tilesize * tileDepth()));
}


tstrip_t TiffDirectory::numOfStrips()
{
    if (isTiled())
        throw TiffError(TIFF_ERROR, "No number of strips for tile image", EKTIFF_WHERE);

    tstrip_t num;

    num = (rowsPerStrip() == (tiff_uint32) -1 ?
           (imageLength() != 0 ? 1 : 0) :
           TIFFhowmany(imageLength(), rowsPerStrip()));
    if (planarConfig() == PLANARCONFIG_SEPARATE)
        num *= samplesPerPixel();
    return (num);
}


ttile_t TiffDirectory::numOfTiles()
{
    if (!isTiled())
        throw TiffError(TIFF_ERROR, "No number of tiles for strip image", EKTIFF_WHERE);

    tiff_uint32 dx = tileWidth();
    tiff_uint32 dy = tileLength();
    tiff_uint32 dz = tileDepth();
    ttile_t ntiles;

//    if (dx == (tiff_uint32) -1)
//        dx = tiffdir->imageWidth();
//    if (dy == (tiff_uint32) -1)
//        dy = tiffdir->imageLength();
//    if (dz == (tiff_uint32) -1)
//        dz = tiffdir->imageDepth();
    ntiles = (dx == 0 || dy == 0 || dz == 0) ? 0 :
        (TIFFhowmany(imageWidth(), dx) *
         TIFFhowmany(imageLength(), dy) *
         TIFFhowmany(imageDepth(), dz));
    if (planarConfig() == PLANARCONFIG_SEPARATE)
        ntiles *= samplesPerPixel();
    return (ntiles);
}


#ifdef STRIPCHOP_SUPPORT
void TiffDirectory::chopupSingleUncompressedStrip()
{
    tiff_uint32 bytecount;
    tiff_uint32 offset;
    tiff_uint32 rowbytes, stripbytes;
    TiffTagEntryT<tiff_uint32>* off;
    TiffTagEntryT<tiff_uint32>* cnt;

    if ( isTiled() )
    {
        off = (TiffTagEntryT<tiff_uint32>*)getGenericTag(TIFFTAG_TILEOFFSETS);
        cnt = (TiffTagEntryT<tiff_uint32>*)getGenericTag(TIFFTAG_TILEBYTECOUNTS);
        rowbytes = vTileSize(1);
    }
    else
    {
        off = (TiffTagEntryT<tiff_uint32>*)getGenericTag(TIFFTAG_STRIPOFFSETS);
        cnt = (TiffTagEntryT<tiff_uint32>*)getGenericTag(TIFFTAG_STRIPBYTECOUNTS);
        rowbytes = vStripSize(1);
    }
        
    if (off == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_TILEOFFSETS);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    if (cnt == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_TILEBYTECOUNTS);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    bytecount = cnt->getValue();
    offset = off->getValue();

    tstrip_t strip, nstrips, rowsperstrip;


    // Make the rows hold at least one
    // scanline, but fill 8k if possible.
    if (rowbytes > 8192) {
        stripbytes = rowbytes;
        rowsperstrip = 1;
    } else {
        rowsperstrip = 8192 / rowbytes;
        stripbytes = rowbytes * rowsperstrip;
    }
    // never increase the number of strips in an image 
    if (rowsperstrip >= rowsPerStrip())
        return;
    nstrips = (tstrip_t) TIFFhowmany(bytecount, stripbytes);

    vector<tiff_uint32> offsets;
    vector<tiff_uint32> bytecounts;
    offsets.clear();
    bytecounts.clear();

    // fill in new strip/tile info
    for (strip = 0; strip < nstrips; strip++) {
        if (stripbytes > bytecount)
            stripbytes = bytecount;
        bytecounts.push_back(stripbytes);
        offsets.push_back(offset);
        offset += stripbytes;
        bytecount -= stripbytes;
    }

    TiffTagEntry* newOff = isTiled()?
        new TiffTagEntryT<vector<tiff_uint32> >(TIFFTAG_TILEOFFSETS, EKTIFF_ULONG, nstrips, offsets ):
        new TiffTagEntryT<vector<tiff_uint32> >(TIFFTAG_STRIPOFFSETS, EKTIFF_ULONG, nstrips, offsets );
    setGenericTag(*newOff);
    delete newOff;
    TiffTagEntry* newCnt = isTiled()?
        new TiffTagEntryT<vector<tiff_uint32> >(TIFFTAG_TILEBYTECOUNTS, EKTIFF_ULONG, nstrips, bytecounts ):
        new TiffTagEntryT<vector<tiff_uint32> >(TIFFTAG_STRIPBYTECOUNTS, EKTIFF_ULONG, nstrips, bytecounts );
    setGenericTag(*newCnt);
    delete newCnt;

    TiffTagEntryT<tiff_uint32>* p = 
        (TiffTagEntryT<tiff_uint32>*)(getGenericTag(TIFFTAG_ROWSPERSTRIP));
    if (p == NULL)
    {
        p = new TiffTagEntryT<tiff_uint32>(TIFFTAG_ROWSPERSTRIP,EKTIFF_ULONG,1,rowsperstrip);
        setGenericTag(*p);
        delete p;
    }
    else
    {
        p->setValue(rowsperstrip);
    }
}
#endif


//////////////////////////////////////////////////////////////
// --------- helper functions -----------
tiff_uint32& TiffDirectory::subfileType()
{
    TiffTagEntryT<tiff_uint32>* p =
        dynamic_cast<TiffTagEntryT<tiff_uint32>*>(getGenericTag(TIFFTAG_SUBFILETYPE));
    if (p == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_SUBFILETYPE);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    return (p->getValue());
}

float& TiffDirectory::xResolution()
{
    TiffTagEntryT<float>* p =
        dynamic_cast<TiffTagEntryT<float>*>(getGenericTag(TIFFTAG_XRESOLUTION));
    if (p == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_XRESOLUTION);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    return (p->getValue());
}

float& TiffDirectory::yResolution()
{
    TiffTagEntryT<float>* p =
        dynamic_cast<TiffTagEntryT<float>*>(getGenericTag(TIFFTAG_YRESOLUTION));
    if (p == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_YRESOLUTION);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    return (p->getValue());
}

uint16& TiffDirectory::resolutionUnit()
{
    TiffTagEntryT<uint16>* p =
        dynamic_cast<TiffTagEntryT<uint16>*>(getGenericTag(TIFFTAG_RESOLUTIONUNIT));
    if (p == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_RESOLUTIONUNIT);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    return (p->getValue());
}

tiff_uint32& TiffDirectory::imageWidth()
{
    TiffTagEntryT<tiff_uint32>* p =
        dynamic_cast<TiffTagEntryT<tiff_uint32>*>(getGenericTag(TIFFTAG_IMAGEWIDTH));
    if (p == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_IMAGEWIDTH);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    return (p->getValue());
}

tiff_uint32& TiffDirectory::imageLength()
{
    TiffTagEntryT<tiff_uint32>* p = 
        dynamic_cast<TiffTagEntryT<tiff_uint32>*>(getGenericTag(TIFFTAG_IMAGELENGTH));
    if (p == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_IMAGELENGTH);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    return (p->getValue());
}

tiff_uint32 TiffDirectory::imageDepth()
{
    const TiffTagEntryT<tiff_uint32>* p = 
        dynamic_cast<const TiffTagEntryT<tiff_uint32>*>(getDefaultTag(TIFFTAG_IMAGEDEPTH));
    if (p == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_IMAGEDEPTH);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    return (p->getValue());
}


tiff_uint32& TiffDirectory::tileWidth()
{
    TiffTagEntryT<tiff_uint32>* p = 
        dynamic_cast<TiffTagEntryT<tiff_uint32>*>(getGenericTag(TIFFTAG_TILEWIDTH));
    if (p == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_TILEWIDTH);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    return (p->getValue());
}

tiff_uint32& TiffDirectory::tileLength()
{
    TiffTagEntryT<tiff_uint32>* p = 
        dynamic_cast<TiffTagEntryT<tiff_uint32>*>(getGenericTag(TIFFTAG_TILELENGTH));
    if (p == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_TILELENGTH);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    return (p->getValue());
}


tiff_uint32 TiffDirectory::tileDepth()
{
    const TiffTagEntryT<tiff_uint32>* p = 
        dynamic_cast<const TiffTagEntryT<tiff_uint32>*>(getDefaultTag(TIFFTAG_TILEDEPTH));
    if (p == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_TILEDEPTH);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    return (p->getValue());
}

uint16& TiffDirectory::photoMetric()
{
    TiffTagEntryT<uint16>* p = 
        dynamic_cast<TiffTagEntryT<uint16>*>(getGenericTag(TIFFTAG_PHOTOMETRIC));
    if (p == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_PHOTOMETRIC);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    return (p->getValue());
}

uint16 TiffDirectory::planarConfig()
{
    const TiffTagEntryT<uint16>* p = 
        dynamic_cast<const TiffTagEntryT<uint16>*>(getDefaultTag(TIFFTAG_PLANARCONFIG));
    if (p == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_PLANARCONFIG);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    return (p->getValue());
}

uint16 TiffDirectory::samplesPerPixel()
{
    const TiffTagEntryT<uint16>* p = 
        dynamic_cast<const TiffTagEntryT<uint16>*>(getDefaultTag(TIFFTAG_SAMPLESPERPIXEL));
    if (p == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_SAMPLESPERPIXEL);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    return (p->getValue());
}

tiff_uint32 TiffDirectory::rowsPerStrip()
{
    const TiffTagEntry * tag = getDefaultTag(TIFFTAG_ROWSPERSTRIP);
    const TiffTagEntryT<tiff_uint32>* p = 
        dynamic_cast<const TiffTagEntryT<tiff_uint32>*>(tag);
    if (p == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_ROWSPERSTRIP);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    return (p->getValue());
}

uint16 TiffDirectory::fillOrder()
{
    const TiffTagEntryT<uint16>* p = 
        dynamic_cast<const TiffTagEntryT<uint16>*>(getDefaultTag(TIFFTAG_FILLORDER));
    if (p == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_FILLORDER);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    return (p->getValue());
}

uint16 TiffDirectory::compression()
{
    const TiffTagEntryT<uint16>* p = 
        dynamic_cast<const TiffTagEntryT<uint16>*>(getDefaultTag(TIFFTAG_COMPRESSION));
    if (p == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_COMPRESSION);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    return (p->getValue());
}



vector<uint16>& TiffDirectory::yccSubSampling()
{
    TiffTagEntryT<vector<uint16> >* p = 
        dynamic_cast<TiffTagEntryT<vector<uint16> >*>(getGenericTag(TIFFTAG_YCBCRSUBSAMPLING));
    if (p == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_YCBCRSUBSAMPLING);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    return (p->getValue());
}

vector<float>& TiffDirectory::refBlackWhite()
{
    TiffTagEntryT<vector<float> >* p = 
        dynamic_cast<TiffTagEntryT<vector<float> >*>(getGenericTag(TIFFTAG_REFERENCEBLACKWHITE));
    if (p == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_REFERENCEBLACKWHITE);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    return (p->getValue());
}

vector<uint16> TiffDirectory::bitsPerSample()
{
    TiffTagEntryT<vector<uint16> >* p = 
        dynamic_cast<TiffTagEntryT<vector<uint16> >*>(getGenericTag(TIFFTAG_BITSPERSAMPLE));
    if (p)
        return (p->getValue());
    else
    {
        TiffTagEntryT<uint16>* pp = dynamic_cast<TiffTagEntryT<uint16>*>(getGenericTag(TIFFTAG_BITSPERSAMPLE));
        if (pp)
        {
            vector<uint16> val;
            val.push_back(pp->getValue());
            return val;
        }
        else
        {
            char msg[100];
            sprintf(msg, "Tag %d not found.", TIFFTAG_BITSPERSAMPLE);
            throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
        }
    }
}

vector<tiff_uint32> TiffDirectory::stripOffsets()
{
    vector<tiff_uint32> stripoffsets ;
    TiffTagEntry* off = getGenericTag( TIFFTAG_STRIPOFFSETS );
    if (off == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_STRIPOFFSETS );
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    if( off->getCount() > 1 )
    {
        TiffTagEntryT< vector<tiff_uint32> >* temp = (TiffTagEntryT< vector<tiff_uint32> >*)off;
        stripoffsets = temp->getValue();
    }
    else
    {
        TiffTagEntryT<tiff_uint32>* temp = (TiffTagEntryT<tiff_uint32>*)off;
        stripoffsets.push_back( temp->getValue() );
    }
    return stripoffsets;
}

vector<tiff_uint32> TiffDirectory::stripByteCounts()
{
    vector<tiff_uint32> stripbytecount ;
    TiffTagEntry* cnt = getGenericTag( TIFFTAG_STRIPBYTECOUNTS );
    if (cnt == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_STRIPBYTECOUNTS);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    if( cnt->getCount() > 1 )
    {
        TiffTagEntryT< vector<tiff_uint32> >* temp = (TiffTagEntryT< vector<tiff_uint32> >*)cnt;
        stripbytecount = temp->getValue();
    }
    else
    {
        TiffTagEntryT<tiff_uint32>* temp = (TiffTagEntryT<tiff_uint32>*)cnt;
        stripbytecount.push_back( temp->getValue() );
    }
    return stripbytecount;
}

vector<tiff_uint32> TiffDirectory::tileOffsets()
{
    vector<tiff_uint32> tileoffsets ;
    TiffTagEntry* off = getGenericTag( TIFFTAG_TILEOFFSETS );
    if (off == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_TILEOFFSETS );
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    if( off->getCount() > 1 )
    {
        TiffTagEntryT< vector<tiff_uint32> >* temp = (TiffTagEntryT< vector<tiff_uint32> >*)off;
        tileoffsets = temp->getValue();
    }
    else
    {
        TiffTagEntryT<tiff_uint32>* temp = (TiffTagEntryT<tiff_uint32>*)off;
        tileoffsets.push_back( temp->getValue() );
    }
    return tileoffsets;
}

vector<tiff_uint32> TiffDirectory::tileByteCounts()
{
    vector<tiff_uint32> tilebytecount ;
    TiffTagEntry* cnt = getGenericTag( TIFFTAG_TILEBYTECOUNTS );
    if (cnt == NULL)
    {
        char msg[100];
        sprintf(msg, "Tag %d not found.", TIFFTAG_TILEBYTECOUNTS);
        throw TiffError(TIFF_TAG_NOTFOUND, msg, EKTIFF_WHERE);
    }
    if( cnt->getCount() > 1 )
    {
        TiffTagEntryT< vector<tiff_uint32> >* temp = (TiffTagEntryT< vector<tiff_uint32> >*)cnt;
        tilebytecount = temp->getValue();
    }
    else
    {
        TiffTagEntryT<tiff_uint32>* temp = (TiffTagEntryT<tiff_uint32>*)cnt;
        tilebytecount.push_back( temp->getValue() );
    }
    return tilebytecount;
}


EntryMapWrapper::~EntryMapWrapper()
{
    for (TiffDirectory::EntryIter iter = map.begin(); iter != map.end(); iter++)
        delete (*iter).second;
}

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

#ifndef TIFF_DIRECTORY_H
#define TIFF_DIRECTORY_H

#if (defined _MSC_VER)
#pragma warning( disable : 4786 )
#endif

#include "TiffTagDefs.h"

#include <algorithm>
#include <iterator>
#include <assert.h>
#include <map>
#include <vector>

#include "TiffTagEntry.h"
#include "TiffIO.h"
#include "TiffConf.h"

// Notify the compiler that we're using the standard namespace here
using namespace std;

//! header="TiffImageFile.h"

class TiffImage;
class EntryMapWrapper;


/*
 * TIFF Image File Directories are comprised of
 * a table of field descriptors of the form shown
 * below.  The table is sorted in ascending order
 * by tag.  The values associated with each entry
 * are disjoint and may appear anywhere in the file
 * (so long as they are placed on a word boundary).
 *
 * If the value is 4 bytes or less, then it is placed
 * in the offset field to save space.  If the value
 * is less than 4 bytes, it is left-justified in the
 * offset field.
 */

//:
// The TiffDirectory class describes an IFD with a tree-like structure. It provides the
// interface to:
// <UL>
//    <LI> read/write tag entries, including defined and private/undefined tag.
//    <LI> traverse/create sub IFDs, including defined and private/undefined sub IFDs.
//    <LI> retrieve/create TiffImage object for handling image data
//    <LI> register private sub IFDs.
// </UL>
// The support of the private/undefined sub IFDs is provided through a registry mechanism.
// To handle a private/undefined sub IFD, the sub IFD tag must be added to the registry.
// Some sub IFD tags, including TIFFTAG_SUBIFD, TIFFTAG_SOUNDIFD, TIFFTAG_EXIFIFD, 
// TIFFTAG_EXIFIFD, TIFFTAG_INTERIFD, are added to the registry automatically by the 
// library.
// <p>
// Each TiffDirectory object is identified by its tag number and index. In the case of
// a main IFD, the tag number will be TIFF_MAINIFD and the index will be the main IFD 
// index.
// <p>
// Example:
// <pre>
//  #include "TiffImageFile.h"
//  #include "TiffDirectory.h"
// 
//  int main()
//  {
//    // To support a private sub IFD (tag number: 16899), add it to the registry
//    TiffDirectory::registerDirTag(16899);
//
//    TiffImageFile imgFile;
//    imgFile.open("test.tif", "r");
//    TiffDirectory* dir = imgFile.getDirectory(0);
//
//    // retrieve defined tag
//    TiffTagEntry* entry = dir->getGenericTag(TIFFTAG_IMAGEWIDTH);
//
//    // retrieve private/undefined tag 12345
//    TiffTagEntry* entry2 = dir->getGenericTag(12345)
//
//    // retrieve the private sub IFD
//    TiffDirectory* subdir = dir->getSubDir(16899);
//
//    // retrieve the TiffImage object for handling image data
//    TiffImage* img = dir->getImage();
//    
//    // operations on image data
//    ....
//
//    imgFile.close();
//
//  }
// </pre>
//
class EKTIFF_DECL TiffDirectory
{
    friend class TiffImageFile;
        
    public:
        typedef map<ttag_t, TiffTagEntry*> EntryMap ;
	    typedef EntryMap::iterator EntryIter ;
        typedef EntryMap::const_iterator ConstEntryIter ;
	    typedef vector<TiffDirectory*> DirVec;
        typedef map<ttag_t, DirVec> SubDirMap ;
        typedef SubDirMap::iterator SubDirIter ;
	    typedef SubDirMap::const_iterator ConstSubDirIter;
        typedef vector<ttag_t> SubDirVec ;
        typedef SubDirVec::iterator SubDirVecIter ;

    
	    // Destructor
        ~TiffDirectory()
            {	
                deleteItems();
            }

        // Get the TiffImage object of this IFD, which is used to read/write image data.
        //!return: Pointer to the image object if it exists or can be created; otherwise NULL.
        TiffImage* getImage();

	    // Get the tag number of this IFD.
	    //!return: Tag number of the IFD.
        ttag_t getTagNum( void ) const
            { return tag ; }

        // Get the index of this IFD.
        // return: Index of the IFD.
        int16  getIndex( void ) const
            { return index ; }

        // Return the number of entries in this IFD.
        //!return: Number of entries.
        EntryMap::size_type numEntries( void ) const 
            { return tagEntries.size(); }

        // Set the tag entry to this IFD. If the tag entry is a sub IFD tag, a TiffDirectory
        // object for the sub IFD will be created internally and can be retrieved with the 
        // getSubDir(...) method. This method will overide any existing tag that matches 
        // the tag entry to set.
        //!param: tagEntry - Tag entry to set.
        void setGenericTag( const TiffTagEntry& tagEntry ) ;

        // Get the tag entry with the given tag number. If the given tag number is a sub IFD
        // tag, the returned tag entry contains the offset information of the sub IFD.
        //!param: tagNum - Tag number
        //!return: Pointer to the tag entry if the tag exists; otherwise NULL.
        TiffTagEntry* getGenericTag(ttag_t tagNum ) const ;

	    // Remove the given tag from the IFD. If the tag is a sub IFD tag, all the content of 
	    // the sub IFD will be removed as well.
	    //!param: tag - Tag number.
	    //!return: True if the tag exists and is removed successfully; otherwise false.
        bool removeGenericTag(ttag_t tag);

        // Perform a "deep" copy from the given IFD to this IFD. The whole IFD tree will be
        // duplicated.
        //!param: tifdir - Source to copy from.
        void copy(const TiffDirectory& tifdir);

        // Add a sub IFD. The index of the sub IFD is the next available index.
        //!param: tag - Sub IFD tag
        void addSubDir(ttag_t tag);
		
	    // Get the sub IFD with the tag and index.
	    //!param: tag - Tag number.
	    //!param: idx - Index.
	    //!return: Pointer to the sub IFD object if the sub IFD exists; otherwise NULL.
        TiffDirectory* getSubDir(ttag_t tag, tiff_uint32 idx = 0);

        // Get a list of sub IFDs with the same tag number.
        //!param: tag - Tag number
        //!param: dirVec - Reference to the vector of the sub IFDs returned.
        //!return: True if the sub IFDs exist; otherwise false.
        bool getSubDirVec(ttag_t tag, DirVec& dirVec);

        // Internally the sub IFDs are stored in a STL map. This method returns the 
        // reference to the map.
        //!return: Reference to the sub IFD map.
        const SubDirMap& getSubDirs() const { return subDirs; }

        // Internally the tag entries are stored in a STL map. This method returns the 
        // reference to the map.
        //!return: Reference to the tag entry map.
        const EntryMap& getTagEntries() const { return tagEntries; }

        // Register a sub IFD tag. The sub IFD are handled with a registry mechanism. 
        // In order to support a sub IFD, its tag number needs to be added to the registry.
        // Some sub IFD tags, including TIFFTAG_SUBIFD, TIFFTAG_SOUNDIFD, TIFFTAG_EXIFIFD, 
        // TIFFTAG_EXIFIFD, TIFFTAG_INTERIFD, are added to the registry automatically by the 
        // library.
        //!param: dirTag - IFD tag number.
        static void registerDirTag( ttag_t dirTag )
            { subDirTags.push_back( dirTag ) ; return ; }

        // Return the offset value of this IFD object.
        //!return: Offset
        toff_t getOffset() const { return offset; }

        // Read the specified amount of data that start from the given offset.
        //!param: buf - Buffer for data returned.
        //!param: off - Offset of the data to read.
        //!param: size - Amount of data to read in byte.
        //!return: Amount read in byte if successful; otherwise zero.
        tsize_t readFile(tdata_t buf, toff_t off, tsize_t size) const;

        // Write the specified amount of data to the file.
        //!param: buf - User-supplied data
        //!param: size - Amount of data to write in byte.
        //!param: off - When the function returns, this parameter is set to the offset of
        //!param:	the data wrote in the file.
        //!return: Amount of data wrote if successful; otherwise zero.
        tsize_t writeFile(tdata_t buf, tsize_t size, toff_t& off);

	    // Get the length of audio stream in byte. This is a helper function for Sound IFD. If
	    // the IFD object is not a sound IFD an error will be thrown.
	    //!return: Length of audio stream in byte if the audio stream exists; otherwise zero.
        tsize_t getAudioSize() const ;

        // Read the audio stream to the supplied buffer. This is a helper function for Sound IFD. If
        // the IFD object is not a sound IFD an error will be thrown.
        //!param: buf - Buffer for audio data.
        //!param: bufSize - Buffer size.
        //!return: Amount of data read in byte if the audio exists; otherwise zero.
        tsize_t readAudio(tdata_t buf, tsize_t bufSize) const ;

        // Write the specified amount of data to audio stream. This is a helper function for Sound IFD. If
        // the IFD object is not a sound IFD an error will be thrown. Note that this
        // should be called before writing the sound IFD.
        //!param: buf - User-supplied data
        //!param: size - Amount of data in byte
        //!return: Amount of data wrote in byte if successful; otherwise zero.
        tsize_t writeAudio(tdata_t buf, tsize_t size);

	    // Check if the IFD object is a main IFD
	    //!return: True if it is a main IFD; otherwise false.
        bool isMainIFD() const 
            { return (tag == TIFF_MAINIFD); }

        // Check if the image in this IFD is a tiled image.
        //!return: True if it is a tiled image; otherwise false.
        bool isTiled() const 
            { return ((tiffio->flags() & TIFF_ISTILED) != 0); }

        // Check if the library is doning data up-sampling.
        //!return: True if doing up-sampling; otherwise false
        bool isUpSampled() const 
            { return ((tiffio->flags() & TIFF_UPSAMPLED) != 0); }

        // Check if the fill order flag is set in the given parameter.
        //!param: fillOrder - Value to check.
        //!return: True if the fill order flag is set; otherwise false.
        bool isFillOrder(uint16 fillOrder) const 
            { return ((tiffio->flags() & (fillOrder)) != 0); }

        // Check if containing the tag entry.
        //!param: tagNum - Tag number.
        //!return: True if the tag exists; otherwise false.
        bool hasTag(ttag_t tagNum) const
            {
                TiffTagEntry* p = getGenericTag(tagNum);
                return (p ? true : false);
            }


        // Compute the size of scanline in byte.
        //!return: The size of scanline in byte.
        tsize_t scanlineSize();

	    // Compute the size of a row for a tile in byte
	    //!return: Size in byte
        tsize_t tileRowSize();	

        // Compute the number of bytes in a variable height, row-aligned strip.
        //!return: Size in bytes
        tsize_t vStripSize(tiff_uint32 nrows);

	    // Compute the number of bytes in a variable length, row-aligned tile. 
	    //!return: size in byte
        tsize_t vTileSize(tiff_uint32 nrows);

        // Compute how many strips in an image.
        //!return: The number of strips.
        tstrip_t numOfStrips();

	    // Compute the number of tiles in the image
	    //!return: The number of tiles
        ttile_t numOfTiles();

        // Print the IFD information. This method is for debugging purpose.
        //!param: c - Output stream for printing.
        //!param: bPrintSubDirs - Whether to print sub IFD inforamtion recursively.
        //!return: Output stream for printing.
        ostream& print(ostream& c = cout, bool bPrintSubDirs = true) const;
	
        //!i: --- helper functions ---------------------------------

        tiff_uint32& subfileType();
        float& xResolution();
        float& yResolution();
        uint16& resolutionUnit();
        tiff_uint32& imageWidth();
        tiff_uint32& imageLength();
        tiff_uint32  imageDepth();
        tiff_uint32& tileWidth();
        tiff_uint32& tileLength();
        tiff_uint32  tileDepth();
        uint16& photoMetric();
        uint16 planarConfig();
        uint16 samplesPerPixel();
        tiff_uint32 rowsPerStrip();
        uint16 fillOrder();
        uint16 compression();
        vector<uint16>& yccSubSampling();
        vector<float>& refBlackWhite();
        vector<uint16> bitsPerSample();
        
        vector<tiff_uint32> stripOffsets();
        vector<tiff_uint32> stripByteCounts();
        vector<tiff_uint32> tileOffsets();
        vector<tiff_uint32> tileByteCounts();
        


    protected:
	
	    // Constructor that is used in the library internally. 
	    //!param: _tiffio - Pointer to the TiffIO object.
	    //!param: tagNum - Tag number of the IFD.
	    //!param: _offset - Offset to the IFD.
	    //!param: idx - Index of the IFD
	    TiffDirectory( TiffIO* _tiffio, ttag_t tagNum, toff_t _offset, int16 idx ) 
            : tiffimg(0), tiffio(_tiffio), offset(_offset), nextDirOffset( 0 ),
              tag( tagNum ), index(idx)
            {
                setupDefaults() ;
            }

	    // Constructor that is used in the library internally. 
	    //!param: _tiffio - Pointer to the TiffIO object.
	    //!param: tagNum - Tag number of the IFD.
	    //!param: idx - Index of the IFD
        TiffDirectory( TiffIO* _tiffio, ttag_t tagNum, int16 idx ) 
            : tiffimg(0), tiffio(_tiffio), offset(0), nextDirOffset( 0 ), tag( tagNum ),
              index(idx)
            {
                setupDefaults() ;
            }

        int readDir( toff_t offset, TiffDirEntry*& dirEntries ) ;
    	void readDirTree( void ) ;
        void writeDir( void ) ;
    	void writeDirTree( void );
      
        void deleteItems( void ) ;
    
        void convertRational( TiffDirEntry* dir, tiff_uint32 num, tiff_uint32 denom, float* rv) ;
        void rationalize( const double& fpNum, tiff_uint32& numer, tiff_uint32& denom, const double error=1.0e-10 ) ;
    
        TiffTagEntry* fetchNormalTag( TiffDirEntry* dp ) ;
        void fetchData( toff_t offset, int8* dataPtr, tiff_uint32 byteSize ) ;
        int8* fetchByteArray( TiffDirEntry* dir ) ;
        int16* fetchShortArray( TiffDirEntry* dir ) ;
        tiff_int32* fetchLongArray( TiffDirEntry* dir ) ;
        float* fetchRationalArray( TiffDirEntry* dir ) ;
        float* fetchFloatArray( TiffDirEntry* dir ) ;
        double* fetchDoubleArray( TiffDirEntry* dir ) ;
    
        void writeNormalTag( TiffTagEntry* tagEntry ) ;
	    void writeByteArray(TiffDirEntry& dirEntry, int8* dp);
	    void writeShortArray(TiffDirEntry& dirEntry, int16* dp);
	    void writeLongArray(TiffDirEntry& dirEntry, tiff_int32* dp);
	    void writeRationalArray(TiffDirEntry& dirEntry, tiff_int32* dp);
	    void writeFloatArray(TiffDirEntry& dirEntry, float* dp);
	    void writeDoubleArray(TiffDirEntry& dirEntry, double* dp);

        void addSubDir( TiffDirectory* subDir ) ; 
	// This function will replace the existing sub IFDs
        void addSubDir( TiffTagEntry* tagEntry ) ;

    	bool canBeUSHORTULONG(uint16 tagNum) const;
    	bool selfGenerated(ttag_t tag) const;
        const TiffTagEntry* getDefaultTag(ttag_t tagNum ) const;

#ifdef STRIPCHOP_SUPPORT
    	void chopupSingleUncompressedStrip();
#endif

        
    private:
        TiffImage*  tiffimg;
        TiffIO*     tiffio ;
        
        static  EntryMapWrapper  defaultEntries ;
        toff_t entryOffset ;
        toff_t dataOffset ;

        toff_t offset ;					// was tif_diroff
        toff_t nextDirOffset ;			// was tif_nextdiroff

        ttag_t tag ;
        int16  index ;
        EntryMap tagEntries ;
        SubDirMap subDirs ;

        static SubDirVec subDirTags ;	// known sub-directory tags
    	static void setupDefaults( void ) ;

    
} ;


// wrapper of EntryMap so that the entries can be free once the program ends
class EntryMapWrapper {
    public:
        EntryMapWrapper() {}
        ~EntryMapWrapper();

        TiffDirectory::EntryMap map;
};
   
#endif // TIFF_DIRECTORY_H


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


#ifndef _TIFF_IMAGE_H
#define	_TIFF_IMAGE_H

//! header="TiffImageFile.h"

#include "TiffConf.h"
#include "TiffTypeDefs.h"
#include <vector>
#include <string>

//Notify the compiler that we're using the standard namespace here
using namespace std;

class TiffTagEntry ;
class TiffDirectory ;
class TiffIO ;

//:
// This class is used to open a TIFF image file and get/create an IFD. 
// <p>
// The TIFF image file can be opened with various open modes, as follows:
// <pre>
// "r"    open for reading.
// "w"    open for writing.
// "l"	  use little-endian byte order for creating a file.
// "b"    use big-endian byte order for creating a file
// "L"    read/write information using LSB2MSB bit order
// "B"    read/write information using MSB2LSB bit order
// "H"    read/write information using host bit order
// "M"    enable use of memory-mapped files when supported
// "m"    disable use of memory-mapped files
// "C"    enable strip chopping support when reading
// "c"    disable strip chopping support
// </pre>
// Currently open for both reading and writing is not supported.
// <p>
// The IFDs of a TIFF image file have a tree-like structure where the root node represents
// the main IFD and sub nodes represent the sub IFDs. To travase a node in the tree you need 
// to describe a path from the root to this node, which is expressed with the type DirPath
// DirPath is specifiec as a vector of a pair of values. For each pair, the first value
// is the IFD's TIFF tag, the second value is the index of the IFD in the case where
// multiple instances of the same IFD tag exist at the same level. The index of the
// vector implies the containing relationship of the IFDs, i.e., DirPath[0] contains
// DirPath[1]; DirPath[1] contains DirPath[2], etc...
// <p>
// Example:
// <pre>
//  #include "TiffImageFile.h"
//
//  int main()
//  {
//    TiffImageFile imgFile;
//    DirPath dirPath;
//    
//    // open a TIFF file    
//    imgFile.open("test.tif", "r");
//    
//    // specify a dir path: MainIFD, index 0 -> SubIFD 330, index 0
//    dirPath.push_back(TiffImageFile::DirPath::value_type(TIFF_MAINIFD, 0));
//    dirPath.push_back(TiffImageFile::DirPath::value_type(330, 0));
//    
//    // get the IFD specified by dirPath
//    TiffDirectory* dir = imgFile.getDirectory(dirPath);
//    
//    // get tag entry for tag TIFFTAG_COMPRESSION under IFD dirPath
//    TiffTagEntry* entry = imgFile.getGenericTag(TIFFTAG_COMPRESSION, dirPath);
//
//    imgFile.close();
//  }
// </pre>
class EKTIFF_DECL TiffImageFile
{
    public:
    //!i: Dir path is specified as a pair of values. The
    //!i: first value is the directory's TIFF Tag, the
    //!i: second value is the index of the directory
    //!i: in the case where multiple instances of the
    //!i: same directory tag exist at the same level. The
    //!i: relationship is implied through the index of the
    //!i: vector, i.e., DirPath[0] contains DirPath[1] contains
    //!i: DirPath[2], etc.
    //typedef pair<ttag_t, uint16> DirPair;
    class DirPair
    {
        public:
            DirPair(ttag_t f, uint16 s):first(f),second(s){}
            ~DirPair(){};
            ttag_t first;
            uint16 second;
    };
    typedef vector<DirPair> DirPath ;
    typedef DirPath::iterator DirPathIter ;
    typedef DirPath::value_type DirPathValType ;
    typedef vector<TiffDirectory*> DirVec;
    

	// Default Constructor.
    TiffImageFile() : tiffio(NULL) {}
    virtual ~TiffImageFile();
        
        //!i: KODAK - is mapsize needed at this level??
        
	// Open an image file.
	//!param: name - Name of the image file.
	//!param: cmode - Open mode.
	void open(const char* name, const char* cmode);
        
	// Open an image file with the given file handle.
	//!param: fileHandle - File handle.
	//!param: name - Name of the image file. It's not actually used to open the file, 
	//!param:	just for the information purpose.
	//!param: cmode - Open mode.
	void open(ekthandle_t fileHandle, const char* name, const char* cmode);

	// Open an image file.
	//!param: name - Name of the image file.
	//!param: cmode - Open mode.
	//!param: mapSize - Specify the memory size used for mapping if the file is opened with the
	//!param:	memory map mode. If this parameter is given a value of 0, the library
	//!param:	will determine the memory size for mapping. The returned value of this
	//!param:	parameter is the real memory size allocated for mapping.
    void open( const char* name, const char* cmode, toff_t& mapSize ) ;

	// Open an image file.
	//!param: fileHandle - File handle.
	//!param: name - Name of the image file. It's not actually used to open the file, 
	//!param:	just for the information purpose.
	//!param: cmode - Open mode.
	//!param: mapSize - Specify the memory size used for mapping if the file is opened with the
	//!param:	memory map mode. If this parameter is given a value of 0, the library
	//!param:	will determine the memory size for mapping. The returned value of this
	//!param:	parameter is the real memory size allocated for mapping.
    void open( ekthandle_t fileHandle, const char* name, const char* cmode, toff_t& mapSize ) ;

	// Open an in-memory image file.
	//!param: imgBuf - Image buffer.
	//!param: bufSize - Size of image buffer.
	//!param: cmode - Open mode.
	void open(void* imgBuf, tsize_t bufSize, const char* cmode);

#ifdef WIN32
	// Open an image file with the supplied internet file handle.
	//!param: hInternet - Internet file handle.
	//!param: name - File name, just for information purpose.
	void open(HINTERNET hInet, const char* name);
#endif
    
	// Close the image file.
	//!param: imgSize - Return the size of the image file.
    void close(tsize_t* imgSize = NULL);
    
	// Get an existing main IFD with the correct index.
	//!param: index - The index of main IFD.
	//!return: A pointer to the TiffDirectory object if the IFD found; otherwise NULL.
	TiffDirectory* getDirectory( uint16 index);
	
	// Get an existing main or sub IFD.
	//!param: dirPath - Directory path of the IFD to retrieve. 
	//!return: A pointer to the TiffDirectory object if the IFD found; otherwise NULL.
    TiffDirectory* getDirectory( DirPath& dirPath ) ;

	// Create a main IFD. The index of main IFD starts from 0 and increments by 1 with
	// each call to this function.
	//!return: A pointer to the TiffDirectory object created.
	TiffDirectory* createDirectory();

	// Create main/sub IFDs specified in the DirPath. If the IFDs already exists 
	// then nothing happens. Note that each of the IFD indexes in the DirPath should
	// be less or equal to the next available index for the corresponding IFD tag;
	// otherwise, a error will be thrown.
	void createDirectory(DirPath dirPath);

	// Return the number of main directories.
	//!return: Number of main directories.
	int numMainDirs() { return mainDirs.size(); }

	// Search a tag in an IFD and return the tag entry found.
	//!param: tag - the tag to retrieve.
	//!param: dirPath - the IFD to search.
	//!return: A pointer to the tag entry if found; otherwise NULL.
    TiffTagEntry* getGenericTag( ttag_t tag, DirPath& dirPath );

	// Set a tag entry in an IFD.
	//!param: entry - The tag entry to set.
	//!param: dirPath - The IFD where the tag entry is set to.
    void setGenericTag( const TiffTagEntry& entry, DirPath& dirPath  ) ;

    /*! Remove a tag entry from an IFD

        If the tag is a sub IFD tag, all the content of 
	    the sub IFD will be removed as well.
     */
    void removeGenericTag( ttag_t tag, DirPath& dirPath );

    private:
    TiffHeader header ;
    TiffIO*    tiffio ;
	DirVec     mainDirs;
    string     fileName ;

    void initAfterOpen( const char* cmode, toff_t& mapSize ) ;
    void verifyVersion( void ) ;
    void readHeader( void ) ;
    void writeHeader( void ) ;
    void initHeader( bool bigendian ) ;
    void initEndian( bool bigendian ) ;
    bool checkEndian( void ) const ;
      
    void readDirectories();
    void writeDirectories();

};


#endif // _TIFF_IMAGE_H

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


#ifndef _TIFF_ERROR_H
#define _TIFF_ERROR_H


// Error Codes
const int TIFF_ERROR                                = 1;


// some generic errors
const int TIFF_MEM_ALLOC_FAIL						= 1001;
const int TIFF_MISSING_IMAGEWIDTH					= 1002;
const int TIFF_MISSING_PLANARCFG					= 1003;
const int TIFF_INVALID_BYTECOUNTS					= 1004;
const int TIFF_IMPROPER_BITSPERSAMPLE				= 1005;
const int TIFF_IMPROPER_PHOTOMETRIC_INTERPRETATION	= 1006;


// Codec Errors
const int TIFF_JPEG_ERROR                           = 2001;
const int TIFF_BOGUS_JPEGTABLES_FIELD               = 2002;
const int TIFF_IMPROPER_JPEG_STRIPTILE_SIZE         = 2003;
const int TIFF_IMPROPER_JPEG_COMPONENT_COUNT        = 2004;
const int TIFF_IMPROPER_JPEG_DATA_PRECISION	    = 2005;
const int TIFF_IMPROPER_JPEG_SAMPLING_FACTORS       = 2006;

// Directory errors
const int TIFF_DIRECTORY_READ                       = 3001;
const int TIFF_DIRECTORY_WRITE                      = 3002;
const int TIFF_TAG_NOTFOUND                         = 3003;

// IO Errors
const int TIFF_IO                                   = 4001 ;
const int TIFF_IO_READ                              = 4002 ;
const int TIFF_IO_WRITE                             = 4003 ;
const int TIFF_IO_SEEK                              = 4004 ;

// General Image File errors
const int TIFF_IMAGE_FILE                           = 5001 ;


#include "TiffTypeDefs.h"
#include <string>
#include <iostream>
#include <exception>

//Notify the compiler that we're using the standard namespace here
using namespace std;

#define EKTIFF_WHERE __FILE__,__LINE__

class EKTIFF_DECL TiffError : public exception
{
    public:
        TiffError(): code(0), line(0), file(""), msg("Default TiffError object") {}
        TiffError(int ecode, const char *emsg, const char *efile, int eline)
            : code(ecode), line(eline), file(efile), msg(emsg) {}
        TiffError(int ecode, const string& emsg, const char *efile, int eline )
            : code(ecode), line(eline), file(efile), msg(emsg) {}
        TiffError(const TiffError& e)
            : code(e.code), line(e.line), file(e.file), msg(e.msg) {}

	~TiffError() throw() {}
        
        // Accessor functions to get information about an error 
        int getErrCode() const { return code; }
        int getErrLine() const { return line; }
        const char* getFilename() const { return file.c_str(); }
        
        void setMsg(const string& m) { msg = m; }
        void setMsg(const char* m) { msg = m; }
        virtual const char* getMsg() const { return msg.c_str(); }
        
        virtual const char* what() const throw() { return getMsg(); }
        void print (ostream &os) const { os << "TiffError: " << msg << endl; }
        
    protected:
        int     code;
        int     line;
        string	file;
        string	msg;
};

// Write error message (from getMsg) to an ostream
inline ostream & operator << (ostream &os, const TiffError& e)
{
    e.print(os);
    return os;
}

#endif











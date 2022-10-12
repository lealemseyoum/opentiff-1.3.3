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


#ifndef TIFF_TAG_ENTRY_H
#define TIFF_TAG_ENTRY_H

//! header="TiffImageFile.h"

#include "TiffTypeDefs.h"
#include <iostream>
#include <vector>
#include <string>

// Notify the compiler that we're using the standard namespace
using namespace std;

//:
// This abstract class is the base class for representing the tag entry of a TIFF image.
// It contains three pieces of inforamtion: tag that identified the field, field type
// and the count of number of values.
//
class EKTIFF_DECL TiffTagEntry
{
    public:

	// Destructor
        virtual ~TiffTagEntry() {}

	// Create a copy of the TiffTagEntry object.
	//!return: Pointer to the copy.
        virtual TiffTagEntry* clone( void ) const = 0 ;
    
	// Get the tag number.
	//!return: Tag number
        ttag_t getTagNum( void ) const
            { return tagNum ; }

	// Get the field type.
	//!return: Field type.
        ttype_t getType( void ) const
            { return type ; }

	// Get the number of values.
	//!return: Reference to the number of values.
        tiff_int32& getCount( void )
            { return count ; }

	// Get the number of values.
	//!return: Number of values.
        tiff_int32 getCount( void ) const
            { return count ; }

	// Set the number of values.
	//!param: cnt - Number of values
        void setCount( tiff_int32 cnt )
            { count = cnt; return ; }


	// Print the tag entry. For debugging.
	//!param: c - Output stream.
	//!return: Output stream.
	virtual ostream& print(ostream& c = cout) const = 0;

    protected:
        TiffTagEntry(ttag_t num, ttype_t typ )
            : tagNum(num), type(typ)
            {}
        TiffTagEntry(ttag_t num, ttype_t typ, tiff_int32 cnt )
            : tagNum(num), type(typ), count(cnt)
            {}


	ttag_t          tagNum;
	ttype_t		    type;
        tiff_int32           count ;
} ;


inline ostream& TiffPrintValue(ostream& c, const unsigned char& v) { return c << (int)v; }
inline ostream& TiffPrintValue(ostream& c, const char& v) { return c << (int)v; }
inline ostream& TiffPrintValue(ostream& c, const unsigned short& v) { return c << v; }
inline ostream& TiffPrintValue(ostream& c, const short& v) { return c << v; }
inline ostream& TiffPrintValue(ostream& c, const unsigned long& v) { return c << v; }
inline ostream& TiffPrintValue(ostream& c, const long& v) { return c << v; }
inline ostream& TiffPrintValue(ostream& c, const unsigned int& v) { return c << v; }
inline ostream& TiffPrintValue(ostream& c, const int& v) { return c << v; }
inline ostream& TiffPrintValue(ostream& c, const float& v) { return c << v; }
inline ostream& TiffPrintValue(ostream& c, const double& v) { return c << v; }
inline ostream& TiffPrintValue(ostream& c, const string& v) { return c << v ; }

// ostream implementation for vector. we want to avoid using operator "<<"
// that will conflict the one in ekc
template <class TYPE>
inline ostream& TiffPrintValue(ostream& c, const vector<TYPE>& v)
{
    int i = v.size();
    c << "[" ;
    for (int j=0; j<i; j++) 
    {
        c << (j==0?"":",");
        TiffPrintValue(c, v[j]);
    }
    c << "]";	
    return c;
}


//:
// This template class derives from TiffTagEntry to represent the tag entry of the
// specified type. 
//
template<class Type>
class EKTIFF_DECL TiffTagEntryT : public TiffTagEntry
{
    public:

	// Create a TiffTagEntry object with the specified tag number and type.
	//!param: num - Tag number.
	//!param: typ - Field type.
        TiffTagEntryT( ttag_t num, ttype_t typ )
            : TiffTagEntry( num, typ )
            {}

	// Create a TiffTagEntry object with the specified tag number, type, count and value.
	//!param: num - Tag number.
	//!param: typ - Field type.
	//!param: cnt - Number of values
	//!param: value - Field value
        TiffTagEntryT( ttag_t num, ttype_t typ, tiff_int32 cnt, Type value )
            : TiffTagEntry( num, typ, cnt ), _value(value)
            {}

	// Copy constructor
	//!param: entry - Source to copy
	TiffTagEntryT(const TiffTagEntryT<Type>& entry)
            : TiffTagEntry(entry.tagNum, entry.type, entry.count), _value(entry.getValue())
            {}

	// Destructor
        virtual ~TiffTagEntryT() {}

	// Create a copy of the TiffTagEntry object.
	//!return: Pointer to the copy.
        virtual TiffTagEntry* clone( void ) const
            { return new TiffTagEntryT<Type>( *this ) ; }

	// Get value.
	//!return: Reference to the value.
        Type& getValue() 
            { return _value ; }
 
	// Get value.
	//!return: Reference to the value.
        const Type& getValue( void ) const
            { return _value ; }

	// Set value.
	//!param: sv - Field value.
        void setValue( const Type& sv )
            { _value = sv; return ; }

	// Print the tag entry. For debugging.
	//!param: c - Output stream.
	//!return: Output stream.
	virtual ostream& print(ostream& c = cout) const
            {
		c << '{' << tagNum << ',' << type << ',' << count << ',';
		c << '(';
		TiffPrintValue(c, _value);
		c << ')' << '}';
		return c;
            }

    private:
	Type _value;

} ;


#endif // TIFF_TAG_ENTRY_H


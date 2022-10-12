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
 * Creation Date: 6/6/2001
 *
 * Original Author: 
 * Chris Lin <ti.lin@kodak.com> 
 * 
 * Contributor(s): 
 * George Sotak <george.sotak@kodak.com>
 */ 

 #include "TiffUtil.h"

void TiffUtil::swabShort(uint16* wp)
{
    uint8* cp = (uint8*) wp;
    int t;
        
    t = cp[1]; cp[1] = cp[0]; cp[0] = t;
}


void TiffUtil::swabLong(tiff_uint32* lp)
{
    uint8* cp = (uint8*) lp;
    int t;
        
    t = cp[3]; cp[3] = cp[0]; cp[0] = t;
    t = cp[2]; cp[2] = cp[1]; cp[1] = t;
}

void TiffUtil::swabArrayOfShort(uint16* wp, tiff_uint32 n)
{
    uint8* cp;
    int t;
    
    /* XXX unroll loop some */
    while (n-- > 0) 
    {
        cp = (uint8*) wp;
        t = cp[1]; cp[1] = cp[0]; cp[0] = t;
        wp++;
    }
}

void
TiffUtil::swabArrayOfLong(register tiff_uint32* lp, tiff_uint32 n)
{
    uint8 *cp;
    int t;
    
    /*         XXX unroll loop some */
    while (n-- > 0) 
    {
        cp = (uint8 *)lp;
        t = cp[3]; cp[3] = cp[0]; cp[0] = t;
        t = cp[2]; cp[2] = cp[1]; cp[1] = t;
        lp++;
    }
}

void
TiffUtil::swabDouble(double *dp)
{
    tiff_uint32* lp = (tiff_uint32*) dp;
    tiff_uint32 t;
    
    swabArrayOfLong(lp, 2);
    t = lp[0]; lp[0] = lp[1]; lp[1] = t;
}


void
TiffUtil::swabArrayOfDouble(double* dp, tiff_uint32 n)
{
    tiff_uint32* lp = (tiff_uint32*) dp;
    tiff_uint32 t;
        
    swabArrayOfLong(lp, n + n);
    while (n-- > 0) 
    {
        t = lp[0]; lp[0] = lp[1]; lp[1] = t;
        lp += 2;
    }
}

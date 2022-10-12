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


#ifndef _TIFFCODECJPEG_H
#define _TIFFCODECJPEG_H

#ifdef JPEG_SUPPORT

#include "TiffCodec.h"
#include "jpeglib.h"

class TiffCodecJpeg;

/*
 * State block for each open TIFF file using
 * libjpeg to do JPEG compression/decompression.
 *
 * libjpeg's visible state is either a jpeg_compress_struct
 * or jpeg_decompress_struct depending on which way we
 * are going.  comm can be used to refer to the fields
 * which are common to both.
 *
 * NB: cinfo is required to be the first member of JPEGState,
 *     so we can safely cast JPEGState* -> jpeg_xxx_struct*
 *     and vice versa!
 */

typedef	struct 
{
	union 
        {
                struct jpeg_compress_struct c;
                struct jpeg_decompress_struct d;		// NB: must be first
                struct jpeg_common_struct comm;
        } cinfo;
        
	jpeg_error_mgr	err;			// libjpeg error manager
        
	// The following two members could be a union, but
	// they're small enough that it's not worth the effort.
	jpeg_destination_mgr dest;		// data dest for compression
	jpeg_source_mgr	src;			// data source for decompression
        
	// private state
	uint16		photometric;		// copy of PhotometricInterpretation
	uint16		h_sampling;			// luminance sampling factors
	uint16		v_sampling;
	tsize_t		bytesperline;		// decompressed bytes per scanline
        
	// pointers to intermediate buffers when processing downsampled data
	JSAMPARRAY	ds_buffer[MAX_COMPONENTS];
	int			scancount;			// number of "scanlines" accumulated    
	int			samplesperclump;
        
	// pseudo-tag fields
	void*		jpegtables;			// JPEGTables tag value, or NULL
	tiff_uint32		jpegtables_length;	// number of bytes in same
	int			jpegquality;		// Compression quality level
	int			jpegcolormode;		// Auto RGB<=>YCbCr convert?
	int			jpegtablesmode;		// What to put in JPEGTables
        
	TiffCodecJpeg*	codec;
        
} JPEGState;

extern void std_init_destination(j_compress_ptr cinfo);
extern boolean std_empty_output_buffer(j_compress_ptr cinfo);
extern void std_term_destination(j_compress_ptr cinfo);
extern void std_init_source(j_decompress_ptr cinfo);

class TiffCodecJpeg : public TiffCodec
{
    friend void std_init_destination(j_compress_ptr cinfo);
    friend boolean std_empty_output_buffer(j_compress_ptr cinfo);
    friend void std_term_destination(j_compress_ptr cinfo);
    friend void std_init_source(j_decompress_ptr cinfo);
        
    public:
	TiffCodecJpeg() { }
	~TiffCodecJpeg() { cleanup(); }
        
    protected:
        
	virtual int init(TiffDirectory* tdir, TiffIO* tio, TiffImage* img);
        
	virtual int setupDecode();
	virtual int setupEncode();
	virtual int preDecode(tsample_t s);
	virtual int preEncode(tsample_t s);
	virtual int postEncode();
    // override byte swapping
	virtual void postDecode(tidata_t buf, tsize_t cc) { }
	virtual int decodeRow(tidata_t buf, tsize_t cc, tsample_t s); 
	virtual int encodeRow(tidata_t buf, tsize_t cc, tsample_t s); 
	virtual int decodeStrip(tidata_t buf, tsize_t cc, tsample_t s); 
	virtual int encodeStrip(tidata_t buf, tsize_t cc, tsample_t s); 
	virtual int decodeTile(tidata_t buf, tsize_t cc, tsample_t s); 
	virtual int encodeTile(tidata_t buf, tsize_t cc, tsample_t s); 
	virtual void cleanup();
	virtual tiff_uint32 defStripSize(tiff_uint32 s);
	virtual void defTileSize(tiff_uint32* tw, tiff_uint32* th);

protected:
	int decode(tidata_t buf, tsize_t cc, tsample_t s);
	int decodeRaw(tidata_t buf, tsize_t cc, tsample_t s);
	int prepare_JPEGTables();
	int encode(tidata_t buf, tsize_t cc, tsample_t s);
	int encodeRaw(tidata_t buf, tsize_t cc, tsample_t s);

//	void TIFFjpeg_error_exit(j_common_ptr cinfo);
//	void TIFFjpeg_output_message(j_common_ptr cinfo);
	int TIFFjpeg_create_compress(JPEGState* sp);
	int TIFFjpeg_create_decompress(JPEGState* sp);
	int TIFFjpeg_set_defaults(JPEGState* sp);
	int TIFFjpeg_set_colorspace(JPEGState* sp, J_COLOR_SPACE colorspace);
	int TIFFjpeg_set_quality(JPEGState* sp, int quality, boolean force_baseline);
	int TIFFjpeg_suppress_tables(JPEGState* sp, boolean suppress);
	int TIFFjpeg_start_compress(JPEGState* sp, boolean write_all_tables);
	int TIFFjpeg_write_scanlines(JPEGState* sp, JSAMPARRAY scanlines, int num_lines);
	int TIFFjpeg_write_raw_data(JPEGState* sp, JSAMPIMAGE data, int num_lines);
	int TIFFjpeg_finish_compress(JPEGState* sp);
	int TIFFjpeg_write_tables(JPEGState* sp);
	int TIFFjpeg_read_header(JPEGState* sp, boolean require_image);
	int TIFFjpeg_start_decompress(JPEGState* sp);
	int TIFFjpeg_read_scanlines(JPEGState* sp, JSAMPARRAY scanlines, int max_lines);
	int TIFFjpeg_read_raw_data(JPEGState* sp, JSAMPIMAGE data, int max_lines);
	int TIFFjpeg_finish_decompress(JPEGState* sp);
	int TIFFjpeg_abort(JPEGState* sp);
	int TIFFjpeg_destroy(JPEGState* sp);
	JSAMPARRAY TIFFjpeg_alloc_sarray(JPEGState* sp, int pool_id,
		      JDIMENSION samplesperrow, JDIMENSION numrows);
	void TIFFjpeg_data_dest(JPEGState* sp);
	int TIFFjpeg_tables_dest(JPEGState* sp);
	void TIFFjpeg_data_src(JPEGState* sp);
	void TIFFjpeg_tables_src(JPEGState* sp);
	int alloc_downsampled_buffers(jpeg_component_info* comp_info, int num_components);

	void getJpegInfo();
	void setJpegInfo();

    private:
};

#endif

#endif

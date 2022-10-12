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

//#define JPEG_SUPPORT
#ifdef JPEG_SUPPORT

#include "TiffConf.h"
#include "TiffCodecJpeg.h"
#include "TiffIO.h"
#include "TiffImage.h"

#include <assert.h>

#include "jerror.h"

typedef struct jpeg_destination_mgr jpeg_destination_mgr;
typedef struct jpeg_source_mgr jpeg_source_mgr;
typedef	struct jpeg_error_mgr jpeg_error_mgr;



#define	N(a)	(sizeof (a) / sizeof (a[0]))

/*
 * Error handling routines (these replace corresponding
 * IJG routines from jerror.c).  These are used for both
 * compression and decompression.
 */
static void
TIFFjpeg_error_exit(j_common_ptr cinfo)
{
    JPEGState *sp = (JPEGState *) cinfo;	/* NB: cinfo assumed first */
    char buffer[JMSG_LENGTH_MAX];

    (*cinfo->err->format_message) (cinfo, buffer);
    jpeg_abort(cinfo);			/* clean up libjpeg state */

    // throw error
    throw TiffError(TIFF_JPEG_ERROR, buffer, EKTIFF_WHERE);
}

/*
 * This routine is invoked only for warning messages,
 * since error_exit does its own thing and trace_level
 * is never set > 0.
 */
static void
TIFFjpeg_output_message(j_common_ptr cinfo)
//void TiffCodecJpeg::TIFFjpeg_output_message(j_common_ptr cinfo)
{
    char buffer[JMSG_LENGTH_MAX];
    
    (*cinfo->err->format_message) (cinfo, buffer);
    
    cout << "Warning: JPEGLib - ";
    cout << buffer << endl;
}


/*
 * Interface routines.  This layer of routines exists
 * primarily to limit side-effects from using setjmp.
 * Also, normal/error returns are converted into return
 * values per libtiff practice.
 */
//#define	CALLJPEG(sp, fail, op)	(SETJMP((sp)->exit_jmpbuf) ? (fail) : (op))
//#define	CALLVJPEG(sp, op)	CALLJPEG(sp, 0, ((op),1))
#define CALLJPEG(fail, op)				\
	try									\
	{									\
		return op;						\
	}									\
	catch (TiffError e)					\
	{									\
		cout << e;					\
		return fail;					\
	}									\

#define CALLVJPEG(op)	CALLJPEG(0, ((op),1))

int TiffCodecJpeg::TIFFjpeg_create_compress(JPEGState* sp)
{
    /* initialize JPEG error handling */
    sp->cinfo.c.err = jpeg_std_error(&sp->err);
    sp->err.error_exit = TIFFjpeg_error_exit;
    sp->err.output_message = TIFFjpeg_output_message;
    
    CALLVJPEG(jpeg_create_compress(&sp->cinfo.c));
}

int TiffCodecJpeg::TIFFjpeg_create_decompress(JPEGState* sp)
{
    /* initialize JPEG error handling */
    sp->cinfo.d.err = jpeg_std_error(&sp->err);
    sp->err.error_exit = TIFFjpeg_error_exit;
    sp->err.output_message = TIFFjpeg_output_message;
    
    CALLVJPEG(jpeg_create_decompress(&sp->cinfo.d));
}

int TiffCodecJpeg::TIFFjpeg_set_defaults(JPEGState* sp)
{
    CALLVJPEG(jpeg_set_defaults(&sp->cinfo.c));
}

int TiffCodecJpeg::TIFFjpeg_set_colorspace(JPEGState* sp, J_COLOR_SPACE colorspace)
{
    CALLVJPEG(jpeg_set_colorspace(&sp->cinfo.c, colorspace));
}

int TiffCodecJpeg::TIFFjpeg_set_quality(JPEGState* sp, int quality, boolean force_baseline)
{
    CALLVJPEG(jpeg_set_quality(&sp->cinfo.c, quality, force_baseline));
}

int TiffCodecJpeg::TIFFjpeg_suppress_tables(JPEGState* sp, boolean suppress)
{
    CALLVJPEG(jpeg_suppress_tables(&sp->cinfo.c, suppress));
}

int TiffCodecJpeg::TIFFjpeg_start_compress(JPEGState* sp, boolean write_all_tables)
{
    CALLVJPEG(jpeg_start_compress(&sp->cinfo.c, write_all_tables));
}

int TiffCodecJpeg::TIFFjpeg_write_scanlines(JPEGState* sp, JSAMPARRAY scanlines, int num_lines)
{
    CALLJPEG(-1, (int) jpeg_write_scanlines(&sp->cinfo.c,
                                            scanlines, (JDIMENSION) num_lines));
}

int TiffCodecJpeg::TIFFjpeg_write_raw_data(JPEGState* sp, JSAMPIMAGE data, int num_lines)
{
    CALLJPEG(-1, (int) jpeg_write_raw_data(&sp->cinfo.c,
                                           data, (JDIMENSION) num_lines));
}

int TiffCodecJpeg::TIFFjpeg_finish_compress(JPEGState* sp)
{
    CALLVJPEG(jpeg_finish_compress(&sp->cinfo.c));
}

int TiffCodecJpeg::TIFFjpeg_write_tables(JPEGState* sp)
{
    CALLVJPEG(jpeg_write_tables(&sp->cinfo.c));
}

int TiffCodecJpeg::TIFFjpeg_read_header(JPEGState* sp, boolean require_image)
{
    CALLJPEG(-1, jpeg_read_header(&sp->cinfo.d, require_image));
}

int TiffCodecJpeg::TIFFjpeg_start_decompress(JPEGState* sp)
{
    CALLVJPEG(jpeg_start_decompress(&sp->cinfo.d));
}

int TiffCodecJpeg::TIFFjpeg_read_scanlines(JPEGState* sp, JSAMPARRAY scanlines, int max_lines)
{
    CALLJPEG(-1, (int) jpeg_read_scanlines(&sp->cinfo.d,
                                           scanlines, (JDIMENSION) max_lines));
}

int TiffCodecJpeg::TIFFjpeg_read_raw_data(JPEGState* sp, JSAMPIMAGE data, int max_lines)
{
    CALLJPEG(-1, (int) jpeg_read_raw_data(&sp->cinfo.d,
                                          data, (JDIMENSION) max_lines));
}

int TiffCodecJpeg::TIFFjpeg_finish_decompress(JPEGState* sp)
{
    CALLJPEG(-1, (int) jpeg_finish_decompress(&sp->cinfo.d));
}

int TiffCodecJpeg::TIFFjpeg_abort(JPEGState* sp)
{
    CALLVJPEG(jpeg_abort(&sp->cinfo.comm));
}

int TiffCodecJpeg::TIFFjpeg_destroy(JPEGState* sp)
{
    CALLVJPEG(jpeg_destroy(&sp->cinfo.comm));
}

JSAMPARRAY TiffCodecJpeg::TIFFjpeg_alloc_sarray(JPEGState* sp, int pool_id,
		      JDIMENSION samplesperrow, JDIMENSION numrows)
{
    CALLJPEG((JSAMPARRAY) NULL,
             (*sp->cinfo.comm.mem->alloc_sarray)
             (&sp->cinfo.comm, pool_id, samplesperrow, numrows));
}

/*
 * JPEG library destination data manager.
 * These routines direct compressed data from libjpeg into the
 * libtiff output buffer.
 */

void
std_init_destination(j_compress_ptr cinfo)
{
    JPEGState* sp = (JPEGState*) cinfo;
    
    sp->dest.next_output_byte = (JOCTET*) sp->codec->tif_rawdata;
    sp->dest.free_in_buffer = (size_t) sp->codec->tif_rawdatasize;
}

boolean
std_empty_output_buffer(j_compress_ptr cinfo)
{
    JPEGState* sp = (JPEGState*) cinfo;

    /* the entire buffer has been filled */
    sp->codec->tif_rawcc = sp->codec->tif_rawdatasize;
    sp->codec->tiffimg->flushData1();
    sp->dest.next_output_byte = (JOCTET*) sp->codec->tif_rawdata;
    sp->dest.free_in_buffer = (size_t) sp->codec->tif_rawdatasize;

    return (TRUE);
}

void
std_term_destination(j_compress_ptr cinfo)
{
    JPEGState* sp = (JPEGState*) cinfo;
    
    sp->codec->tif_rawcp = (tidata_t) sp->dest.next_output_byte;
    sp->codec->tif_rawcc =
        sp->codec->tif_rawdatasize - (tsize_t) sp->dest.free_in_buffer;
}

void TiffCodecJpeg::TIFFjpeg_data_dest(JPEGState* sp)
{
    sp->cinfo.c.dest = &sp->dest;
    sp->dest.init_destination = std_init_destination;
    sp->dest.empty_output_buffer = std_empty_output_buffer;
    sp->dest.term_destination = std_term_destination;
}

/*
 * Alternate destination manager for outputting to JPEGTables field.
 */

static void
tables_init_destination(j_compress_ptr cinfo)
{
    JPEGState* sp = (JPEGState*) cinfo;
    
    /* while building, jpegtables_length is allocated buffer size */
    sp->dest.next_output_byte = (JOCTET*) sp->jpegtables;
    sp->dest.free_in_buffer = (size_t) sp->jpegtables_length;
}

static boolean
tables_empty_output_buffer(j_compress_ptr cinfo)
{
    JPEGState* sp = (JPEGState*) cinfo;
    void* newbuf;
    
    /* the entire buffer has been filled; enlarge it by 1000 bytes */
    newbuf = realloc((tdata_t) sp->jpegtables,
                     (tsize_t) (sp->jpegtables_length + 1000));
    if (newbuf == NULL)
        ERREXIT1(cinfo, JERR_OUT_OF_MEMORY, 100);
    
    sp->dest.next_output_byte = (JOCTET*) newbuf + sp->jpegtables_length;
    sp->dest.free_in_buffer = (size_t) 1000;
    sp->jpegtables = newbuf;
    sp->jpegtables_length += 1000;
    return (TRUE);
}

static void
tables_term_destination(j_compress_ptr cinfo)
{
    JPEGState* sp = (JPEGState*) cinfo;
    
    /* set tables length to number of bytes actually emitted */
    sp->jpegtables_length -= sp->dest.free_in_buffer;
}

int TiffCodecJpeg::TIFFjpeg_tables_dest(JPEGState* sp)
{
    /*
     * Allocate a working buffer for building tables.
     * Initial size is 1000 bytes, which is usually adequate.
     */
    if (sp->jpegtables)
        free(sp->jpegtables);
    sp->jpegtables_length = 1000;
    sp->jpegtables = (void*) malloc((tsize_t) sp->jpegtables_length);
    if (sp->jpegtables == NULL) 
    {
        sp->jpegtables_length = 0;
        string m = "No space for JPEGTables";
        throw TiffError(TIFF_MEM_ALLOC_FAIL, m, EKTIFF_WHERE);
    }
    sp->cinfo.c.dest = &sp->dest;
    sp->dest.init_destination = tables_init_destination;
    sp->dest.empty_output_buffer = tables_empty_output_buffer;
    sp->dest.term_destination = tables_term_destination;

    return (1);
}

/*
 * JPEG library source data manager.
 * These routines supply compressed data to libjpeg.
 */

void
std_init_source(j_decompress_ptr cinfo)
{
    JPEGState* sp = (JPEGState*) cinfo;
    
    sp->src.next_input_byte = (const JOCTET*) sp->codec->tif_rawdata;
    sp->src.bytes_in_buffer = (size_t) sp->codec->tif_rawcc;
}

static boolean
std_fill_input_buffer(j_decompress_ptr cinfo)
{
    JPEGState* sp = (JPEGState* ) cinfo;
    static const JOCTET dummy_EOI[2] = { 0xFF, JPEG_EOI };
    
    /*
     * Should never get here since entire strip/tile is
     * read into memory before the decompressor is called,
     * and thus was supplied by init_source.
     */
    WARNMS(cinfo, JWRN_JPEG_EOF);
    /* insert a fake EOI marker */
    sp->src.next_input_byte = dummy_EOI;
    sp->src.bytes_in_buffer = 2;
    return (TRUE);
}

static void
std_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
    JPEGState* sp = (JPEGState*) cinfo;
    
    if (num_bytes > 0) 
    {
        if (num_bytes > (long) sp->src.bytes_in_buffer) 
        {
            /* oops, buffer overrun */
            (void) std_fill_input_buffer(cinfo);
        } 
        else 
        {
            sp->src.next_input_byte += (size_t) num_bytes;
            sp->src.bytes_in_buffer -= (size_t) num_bytes;
        }
    }

    return ;
}

static void
std_term_source(j_decompress_ptr cinfo)
{
    /* No work necessary here */
    /* Or must we update tif->tif_rawcp, tif->tif_rawcc ??? */
    /* (if so, need empty tables_term_source!) */
    (void) cinfo;
}

void TiffCodecJpeg::TIFFjpeg_data_src(JPEGState* sp)
{
    sp->cinfo.d.src = &sp->src;
    sp->src.init_source = std_init_source;
    sp->src.fill_input_buffer = std_fill_input_buffer;
    sp->src.skip_input_data = std_skip_input_data;
    sp->src.resync_to_restart = jpeg_resync_to_restart;
    sp->src.term_source = std_term_source;
    sp->src.bytes_in_buffer = 0;		/* for safety */
    sp->src.next_input_byte = NULL;
}

/*
 * Alternate source manager for reading from JPEGTables.
 * We can share all the code except for the init routine.
 */

static void
tables_init_source(j_decompress_ptr cinfo)
{
    JPEGState* sp = (JPEGState*) cinfo;
    
    sp->src.next_input_byte = (const JOCTET*) sp->jpegtables;
    sp->src.bytes_in_buffer = (size_t) sp->jpegtables_length;
}

void TiffCodecJpeg::TIFFjpeg_tables_src(JPEGState* sp)
{
    TIFFjpeg_data_src(sp);
    sp->src.init_source = tables_init_source;
}

/*
 * Allocate downsampled-data buffers needed for downsampled I/O.
 * We use values computed in jpeg_start_compress or jpeg_start_decompress.
 * We use libjpeg's allocator so that buffers will be released automatically
 * when done with strip/tile.
 * This is also a handy place to compute samplesperclump, bytesperline.
 */
//static int
//alloc_downsampled_buffers(jpeg_component_info* comp_info,
//			  int num_components)
int TiffCodecJpeg::alloc_downsampled_buffers(jpeg_component_info* comp_info,
                                             int num_components)
{
    JPEGState* sp = (JPEGState*)tif_data;
    int ci;
    jpeg_component_info* compptr;
    JSAMPARRAY buf;
    int samples_per_clump = 0;
    
    for (ci = 0, compptr = comp_info; ci < num_components;
         ci++, compptr++) 
    {
        samples_per_clump += compptr->h_samp_factor *
            compptr->v_samp_factor;
        buf = TIFFjpeg_alloc_sarray(sp, JPOOL_IMAGE,
                                    compptr->width_in_blocks * DCTSIZE,
                                    (JDIMENSION) (compptr->v_samp_factor*DCTSIZE));
        if (buf == NULL)
            return (0);
        sp->ds_buffer[ci] = buf;
    }
    sp->samplesperclump = samples_per_clump;
    /* Cb,Cr both have sampling factors 1 */
    /* so downsampled width of Cb is # of clumps per line */
    sp->bytesperline = sizeof(JSAMPLE) * samples_per_clump *
        comp_info[1].downsampled_width;
	
    return (1);
}


/*
 * JPEG Decoding.
 */

//static int
//JPEGSetupDecode()
int TiffCodecJpeg::setupDecode()
{
    JPEGState* sp = (JPEGState*)tif_data;

    assert(sp != NULL);
    assert(sp->cinfo.comm.is_decompressor);
    
    // read in jpeg tables, mode, etc...
    getJpegInfo();
    
    TIFFjpeg_tables_src(sp);
    if(TIFFjpeg_read_header(sp,FALSE) != JPEG_HEADER_TABLES_ONLY) 
    {
        throw TiffError(TIFF_BOGUS_JPEGTABLES_FIELD, "Bogus JPEGTables field", EKTIFF_WHERE);
    }
    
    /* Grab parameters that are same for all strips/tiles */
    sp->photometric = tiffdir->photoMetric();
    switch (sp->photometric) 
    {
	case PHOTOMETRIC_YCBCR:
            {
		vector<uint16> vals = tiffdir->yccSubSampling();
		sp->h_sampling = vals[0];
		sp->v_sampling = vals[1];
		break;
            }
	default:
            /* TIFF 6.0 forbids subsampling of all other color spaces */
            sp->h_sampling = 1;
            sp->v_sampling = 1;
            break;
    }
    
    /* Set up for reading normal data */
    TIFFjpeg_data_src(sp);
    // tif->tif_postdecode = _TIFFNoPostDecode; /* override byte swapping */
    return (1);
}

/*
 * Set up for decoding a strip or tile.
 */
int TiffCodecJpeg::preDecode(tsample_t s)
{
    JPEGState *sp = (JPEGState *)tif_data;
    tiff_uint32 segment_width, segment_height;
    int downsampled_output;
    int ci;
    
    assert(sp != NULL);
    assert(sp->cinfo.comm.is_decompressor);
    /*
     * Reset decoder state from any previous strip/tile,
     * in case application didn't read the whole strip.
     */
    if (!TIFFjpeg_abort(sp))
        return (0);
    /*
     * Read the header for this strip/tile.
     */
    if (TIFFjpeg_read_header(sp, TRUE) != JPEG_HEADER_OK)
        return (0);
    /*
     * Check image parameters and set decompression parameters.
     */
    if (tiffdir->isTiled()) 
    {
        segment_width = tiffdir->tileWidth();
        segment_height = tiffdir->tileLength();
        sp->bytesperline = tileRowSize();
    }
    else
    {
        segment_width = tiffdir->imageWidth();
        segment_height = tiffdir->imageLength() - tif_row;
        
        tiff_uint32 rowsperstrip = tiffdir->rowsPerStrip();
        if (segment_height > rowsperstrip)
            segment_height = rowsperstrip;
        sp->bytesperline = scanlineSize();
    }
    
    uint16 planarconfig = tiffdir->planarConfig();
    if (planarconfig == PLANARCONFIG_SEPARATE && s > 0) 
    {
        /*
         * For PC 2, scale down the expected strip/tile size
         * to match a downsampled component
         */
        segment_width = TIFFhowmany(segment_width, sp->h_sampling);
        segment_height = TIFFhowmany(segment_height, sp->v_sampling);
    }
    if (sp->cinfo.d.image_width != segment_width ||
        sp->cinfo.d.image_height != segment_height) 
    {
        string m = "Improper JPEG strip/tile size";
        throw TiffError(TIFF_IMPROPER_JPEG_STRIPTILE_SIZE, m, EKTIFF_WHERE);
    }
    
    uint16 samplesperpixel = tiffdir->samplesPerPixel();
    if (sp->cinfo.d.num_components !=
        (planarconfig == PLANARCONFIG_CONTIG ?
         samplesperpixel : 1)) 
    {
        string m = "Improper JPEG component count";
        throw TiffError(TIFF_IMPROPER_JPEG_COMPONENT_COUNT, m, EKTIFF_WHERE);
    }
    
    uint16 bitspersample = (tiffdir->bitsPerSample())[0];
    if (sp->cinfo.d.data_precision != bitspersample) 
    {
        string m = "Improper JPEG data precision";
        throw TiffError(TIFF_IMPROPER_JPEG_DATA_PRECISION, m, EKTIFF_WHERE);
    }
    
    if (planarconfig == PLANARCONFIG_CONTIG) 
    {
        /* Component 0 should have expected sampling factors */
        if (sp->cinfo.d.comp_info[0].h_samp_factor != sp->h_sampling ||
            sp->cinfo.d.comp_info[0].v_samp_factor != sp->v_sampling) 
        {
            string m = "Improper JPEG sampling factors";
            throw TiffError(TIFF_IMPROPER_JPEG_SAMPLING_FACTORS, m, EKTIFF_WHERE);
        }
        /* Rest should have sampling factors 1,1 */
        for (ci = 1; ci < sp->cinfo.d.num_components; ci++) 
        {
            if (sp->cinfo.d.comp_info[ci].h_samp_factor != 1 ||
                sp->cinfo.d.comp_info[ci].v_samp_factor != 1) 
            {
                string m = "Improper JPEG sampling factors";
                throw TiffError(TIFF_IMPROPER_JPEG_SAMPLING_FACTORS, m, EKTIFF_WHERE);
            }
        }
    }
    else 
    {
        /* PC 2's single component should have sampling factors 1,1 */
        if (sp->cinfo.d.comp_info[0].h_samp_factor != 1 ||
            sp->cinfo.d.comp_info[0].v_samp_factor != 1) 
        {
            string m = "Improper JPEG sampling factors";
            throw TiffError(TIFF_IMPROPER_JPEG_SAMPLING_FACTORS, m, EKTIFF_WHERE);
        }
    }
    downsampled_output = FALSE;
    if (planarconfig == PLANARCONFIG_CONTIG &&
        sp->photometric == PHOTOMETRIC_YCBCR &&
        sp->jpegcolormode == JPEGCOLORMODE_RGB) 
    {
        /* Convert YCbCr to RGB */
        sp->cinfo.d.jpeg_color_space = JCS_YCbCr;
        sp->cinfo.d.out_color_space = JCS_RGB;
    }
    else
    {
        /* Suppress colorspace handling */
        sp->cinfo.d.jpeg_color_space = JCS_UNKNOWN;
        sp->cinfo.d.out_color_space = JCS_UNKNOWN;
        if (planarconfig == PLANARCONFIG_CONTIG &&
            (sp->h_sampling != 1 || sp->v_sampling != 1))
            downsampled_output = TRUE;
        /* XXX what about up-sampling? */
    }
    
    if (downsampled_output) 
    {
        /* Need to use raw-data interface to libjpeg */
        sp->cinfo.d.raw_data_out = TRUE;
        // tif->tif_decoderow = JPEGDecodeRaw;
        // tif->tif_decodestrip = JPEGDecodeRaw;
        // tif->tif        _decodetile = JPEGDecodeRaw;
    } 
    else 
    {
        /*      Use normal interface to libjpeg */
        sp->cinfo.d.raw_data_out = FALSE;
        // tif->tif_decoderow = JPEGDecode;
        // tif->tif_decodestrip = JPEGDecode;
        // tif->tif_decodetile = JPEGDecode;
	}
    /* Start JPEG decompressor */
    if (!TIFFjpeg_start_decompress(sp))
        return (0);
    /* Allocate downsampled-data buffers if needed */
    if (downsampled_output) 
    {
        if (!alloc_downsampled_buffers(sp->cinfo.d.comp_info,
                                       sp->cinfo.d.num_components))
            return (0);
        sp->scancount = DCTSIZE;	/* mark buffer empty */
    }
    return (1);
}

/*
 * Decode a chunk of pixels.
 * "Standard" case: returned data is not downsampled.
 */
int TiffCodecJpeg::decode(tidata_t buf, tsize_t cc, tsample_t s)
{
    JPEGState *sp = (JPEGState *)tif_data;
    tsize_t nrows;
    JSAMPROW bufptr[1];
    
    (void) s;
    assert(sp != NULL);
    /* data is expected to be read in multiples of a scanline */
    nrows = cc / sp->bytesperline;
    
    if (cc % sp->bytesperline)
        cout << "Warning: JPEGDecode -- fractional scanline not read";
    
    while (nrows-- > 0) 
    {
        bufptr[0] = (JSAMPROW) buf;
        if (TIFFjpeg_read_scanlines(sp, bufptr, 1) != 1)
            return (0);
        if (nrows > 0)
            tif_row++;
        buf += sp->bytesperline;
    }
    /* Close down the decompressor if we've finished the strip or tile. */
    if (sp->cinfo.d.output_scanline == sp->cinfo.d.output_height) 
    {
        if (TIFFjpeg_finish_decompress(sp) != TRUE)
            return (0);
    }
    return (1);
}

/*
 * Decode a chunk of pixels.
 * Returned data is downsampled per sampling factors.
 */
//static int
//JPEGDecodeRaw(tidata_t buf, tsize_t cc, tsample_t s)
int TiffCodecJpeg::decodeRaw(tidata_t buf, tsize_t cc, tsample_t s)
{
    JPEGState *sp = (JPEGState *)tif_data;
    JSAMPLE* inptr;
    JSAMPLE* outptr;
    tsize_t nrows;
    JDIMENSION clumps_per_line, nclump;
    int clumpoffset, ci, xpos, ypos;
    jpeg_component_info* compptr;
    int samples_per_clump = sp->samplesperclump;

    (void) s;
    assert(sp != NULL);
    /* data is expected to be read in multiples of a scanline */
    nrows = cc / sp->bytesperline;

    if (cc % sp->bytesperline)
        cout << "Warning: JPEGDecodeRaw -- fractional scanline not read";

    /* Cb,Cr both have sampling factors 1, so this is correct */
    clumps_per_line = sp->cinfo.d.comp_info[1].downsampled_width;

    while (nrows-- > 0) 
    {
        /* Reload downsampled-data buffer if needed */
        if (sp->scancount >= DCTSIZE) 
        {
            int n = sp->cinfo.d.max_v_samp_factor * DCTSIZE;
            if (TIFFjpeg_read_raw_data(sp, sp->ds_buffer, n) != n)
                return (0);
            sp->scancount = 0;
            /* Close down the decompressor if done. */
            if (sp->cinfo.d.output_scanline >=
                sp->cinfo.d.output_height) 
            {
                if (TIFFjpeg_finish_decompress(sp) != TRUE)
                    return (0);
            }
        }
        /*
         * Fastest way to unseparate the data is to make one pass
         * over the scanline for each row of each component.
         */
        clumpoffset = 0;		/* first sample in clump */
        for (ci = 0, compptr = sp->cinfo.d.comp_info;
             ci < sp->cinfo.d.num_components;
             ci++, compptr++) 
        {
            int hsamp = compptr->h_samp_factor;
            int vsamp = compptr->v_samp_factor;
            for (ypos = 0; ypos < vsamp; ypos++) 
            {
                inptr = sp->ds_buffer[ci][sp->scancount*vsamp + ypos];
                outptr = ((JSAMPLE*) buf) + clumpoffset;
                if (hsamp == 1) 
                {
                    /* fast path for at least Cb and Cr */
                    for (nclump = clumps_per_line; nclump-- > 0; ) 
                    {
                        outptr[0] = *inptr++;
                        outptr += samples_per_clump;
                    }
                } 
                else 
                {
                    /* general case */
                    for (nclump = clumps_per_line; nclump-- > 0; ) 
                    {
                        for (xpos = 0; xpos < hsamp; xpos++)
                            outptr[xpos] = *inptr++;
                        outptr += samples_per_clump;
                    }
                }
                clumpoffset += hsamp;
            }
        }
        sp->scancount++;
        if (nrows > 0)
            tif_row++;
        buf += sp->bytesperline;
    }
    return (1);
}


/*
 * JPEG Encoding.
 */

static void
unsuppress_quant_table (JPEGState* sp, int tblno)
{
    JQUANT_TBL* qtbl;
    
    if ((qtbl = sp->cinfo.c.quant_tbl_ptrs[tblno]) != NULL)
        qtbl->sent_table = FALSE;
}

static void
unsuppress_huff_table (JPEGState* sp, int tblno)
{
    JHUFF_TBL* htbl;
    
    if ((htbl = sp->cinfo.c.dc_huff_tbl_ptrs[tblno]) != NULL)
        htbl->sent_table = FALSE;
    if ((htbl = sp->cinfo.c.ac_huff_tbl_ptrs[tblno]) != NULL)
        htbl->sent_table = FALSE;
}

//static int
//prepare_JPEGTables()
int TiffCodecJpeg::prepare_JPEGTables()
{
    JPEGState* sp = (JPEGState*)tif_data;
    
    /* Initialize quant tables for current quality setting */
    if (!TIFFjpeg_set_quality(sp, sp->jpegquality, FALSE))
        return (0);
    /* Mark only the tables we want for output */
    /* NB: chrominance tables are currently used only with YCbCr */
    if (!TIFFjpeg_suppress_tables(sp, TRUE))
        return (0);
    if (sp->jpegtablesmode & JPEGTABLESMODE_QUANT) 
    {
        unsuppress_quant_table(sp, 0);
        if (sp->photometric == PHOTOMETRIC_YCBCR)
            unsuppress_quant_table(sp, 1);
    }
    if (sp->jpegtablesmode & JPEGTABLESMODE_HUFF) 
    {
        unsuppress_huff_table(sp, 0);
        if (sp->photometric == PHOTOMETRIC_YCBCR)
            unsuppress_huff_table(sp, 1);
    }
    /* Direct libjpeg output into jpegtables */
    if (!TIFFjpeg_tables_dest(sp))
        return (0);
    /* Emit tables-only datastream */
    if (!TIFFjpeg_write_tables(sp))
        return (0);
    
    return (1);
}

//static int
//JPEGSetupEncode()
int TiffCodecJpeg::setupEncode()
{
    JPEGState* sp = (JPEGState*)tif_data;

    assert(sp != NULL);
    assert(!sp->cinfo.comm.is_decompressor);

    /*
     * Initialize all JPEG parameters to default values.
     * Note that jpeg_set_defaults needs legal values for
     * in_color_space and input_components.
     */
    sp->cinfo.c.in_color_space = JCS_UNKNOWN;
    sp->cinfo.c.input_components = 1;
    if (!TIFFjpeg_set_defaults(sp))
        return (0);

    /* Set per-file parameters */
    sp->photometric = tiffdir->photoMetric();
    switch (sp->photometric) 
    {
	case PHOTOMETRIC_YCBCR:
            {
		vector<uint16> vals = tiffdir->yccSubSampling();
		sp->h_sampling = vals[0];
		sp->v_sampling = vals[1];
		/*
		 * A ReferenceBlackWhite field *must* be present since the
		 * default value is inappropriate for YCbCr.  Fill in the
		 * proper value if application didn't set it.
		 */
#ifdef COLORIMETRY_SUPPORT
		TiffTagEntryT<vector<float> >* ref = 
                    dynamic_cast<TiffTagEntryT<vector<float> >*>(tiffdir->getGenericTag(TIFFTAG_REFERENCEBLACKWHITE));
		if (ref) 
                {
                    vector<float> refbw;
                    uint16 bitspersample = (tiffdir->bitsPerSample())[0];

                    long top = 1L << bitspersample;
                    refbw[0] = 0;
                    refbw[1] = (float)(top-1L);
                    refbw[2] = (float)(top>>1);
                    refbw[3] = refbw[1];
                    refbw[4] = refbw[2];
                    refbw[5] = refbw[1];
                    TiffTagEntryT<vector<float>> entry(TIFFTAG_REFERENCEBLACKWHITE, EKTIFF_FLOAT, refbw.size(), refbw); 
                    tiffdir->setGenericTag(entry);
		}
#endif
		break;
            }
	case PHOTOMETRIC_PALETTE:		/* disallowed by Tech Note */
	case PHOTOMETRIC_MASK:
            {
		char m[256];
		sprintf(m, "Error: JPEGSetupEncode -- PhotometricInterpretation %d not allowed for JPEG", sp->photometric);
		throw TiffError(TIFF_IMPROPER_PHOTOMETRIC_INTERPRETATION, m, EKTIFF_WHERE);
            }
	default:
            /* TIFF 6.0 forbids subsampling of all other color spaces */
            sp->h_sampling = 1;
            sp->v_sampling = 1;
            break;
    }
	
    /* Verify miscellaneous parameters */

    /*
     * This would need work if libtiff ever supports different
     * depths for different components, or if libjpeg ever supports
     * run-time selection of depth.  Neither is imminent.
     */
    uint16 bitspersample = (tiffdir->bitsPerSample())[0];
    if (bitspersample != BITS_IN_JSAMPLE) 
    {
        char m[256];
        sprintf(m, "BitsPerSample %d not allowed for JPEG", bitspersample);
        throw TiffError(TIFF_IMPROPER_BITSPERSAMPLE, m, EKTIFF_WHERE);
    }
    sp->cinfo.c.data_precision = bitspersample;
    if (tiffdir->isTiled()) 
    {
        tiff_uint32 tilelength = tiffdir->tileLength();
        if ((tilelength % (sp->v_sampling * DCTSIZE)) != 0) 
        {
            char m[256];
            sprintf(m, "JPEG tile height must be multiple of %d", sp->v_sampling * DCTSIZE);
            throw TiffError(TIFF_JPEG_ERROR, m, EKTIFF_WHERE);
        }

        tiff_uint32 tilewidth = tiffdir->tileWidth();
        if ((tilewidth % (sp->h_sampling * DCTSIZE)) != 0) 
        {
            char m[256];
            sprintf(m, "JPEG tile width must be multiple of %d", sp->h_sampling * DCTSIZE);
            throw TiffError(TIFF_JPEG_ERROR, m, EKTIFF_WHERE);
        }
    } 
    else 
    {
        tiff_uint32 rowsperstrip = tiffdir->rowsPerStrip();
        tiff_uint32 imagelength = tiffdir->imageLength();

        if (rowsperstrip < imagelength &&
            (rowsperstrip % (sp->v_sampling * DCTSIZE)) != 0) 
        {
            char m[256];
            sprintf(m, "RowsPerStrip must be multiple of %d for JPEG", sp->v_sampling * DCTSIZE);
            throw TiffError(TIFF_JPEG_ERROR, m, EKTIFF_WHERE);
        }
    }

    /* Create a JPEGTables field if appropriate */
    if (sp->jpegtablesmode & (JPEGTABLESMODE_QUANT|JPEGTABLESMODE_HUFF)) 
    {
        if (!prepare_JPEGTables())
            return (0);
        /* Mark the field present */
        setJpegInfo();
//        tiffio->flags() |= TIFF_DIRTYDIRECT;
    } 
    else 
    {
        /* We do not support application-supplied JPEGTables, */
        /* so mark the field not present */
        // todo: need something here??
//		TIFFClrFieldBit(tif, FIELD_JPEGTABLES);
    }

    /* Direct libjpeg output to libtiff's output buffer */
    TIFFjpeg_data_dest(sp);

    return (1);
}

/*
 * Set encoding state at the start of a strip or tile.
 */
int TiffCodecJpeg::preEncode(tsample_t s)
{
    JPEGState *sp = (JPEGState *)tif_data;
    tiff_uint32 segment_width, segment_height;
    int downsampled_input;

    assert(sp != NULL);
    assert(!sp->cinfo.comm.is_decompressor);
    /*
     * Set encoding parameters for this strip/tile.
     */
    if (tiffdir->isTiled()) 
    {
        segment_width = tiffdir->tileWidth();
        segment_height = tiffdir->tileLength();
        sp->bytesperline = tileRowSize();
    } 
    else
    {
        segment_width = tiffdir->imageWidth();
        segment_height = tiffdir->imageLength() - tif_row;

        tiff_uint32 rowsperstrip = tiffdir->rowsPerStrip();
        if (segment_height > rowsperstrip)
            segment_height = rowsperstrip;
        sp->bytesperline = scanlineSize();
    }

    uint16 planarconfig = tiffdir->planarConfig();
    if (planarconfig == PLANARCONFIG_SEPARATE && s > 0) 
    {
        /* for PC 2, scale down the strip/tile size
         * to match a downsampled component
         */
        segment_width = TIFFhowmany(segment_width, sp->h_sampling);
        segment_height = TIFFhowmany(segment_height, sp->v_sampling);
    }
    if (segment_width > 65535 || segment_height > 65535) 
    {
        string m = "Strip/tile too large for JPEG";
        throw TiffError(TIFF_JPEG_ERROR, m, EKTIFF_WHERE);
    }
    sp->cinfo.c.image_width = segment_width;
    sp->cinfo.c.image_height = segment_height;
    downsampled_input = FALSE;
    if (planarconfig == PLANARCONFIG_CONTIG) 
    {
        sp->cinfo.c.input_components = tiffdir->samplesPerPixel();
        if (sp->photometric == PHOTOMETRIC_YCBCR) {
            if (sp->jpegcolormode == JPEGCOLORMODE_RGB) 
            {
                sp->cinfo.c.in_color_space = JCS_RGB;
            } 
            else 
            {
                sp->cinfo.c.in_color_space = JCS_YCbCr;
                if (sp->h_sampling != 1 || sp->v_sampling != 1)
                    downsampled_input = TRUE;
            }
            if (!TIFFjpeg_set_colorspace(sp, JCS_YCbCr))
                return (0);
            /*
             * Set Y sampling factors;
             * we assume jpeg_set_colorspace() set the rest to 1
             */
            sp->cinfo.c.comp_info[0].h_samp_factor = sp->h_sampling;
            sp->cinfo.c.comp_info[0].v_samp_factor = sp->v_sampling;
        } 
        else 
        {
            sp->cinfo.c.in_color_space = JCS_UNKNOWN;
            if (!TIFFjpeg_set_colorspace(sp, JCS_UNKNOWN))
                return (0);
            /* jpeg_set_colorspace set all sampling factors to 1 */
        }
    } 
    else
    {
        sp->cinfo.c.input_components = 1;
        sp->cinfo.c.in_color_space = JCS_UNKNOWN;
        if (!TIFFjpeg_set_colorspace(sp, JCS_UNKNOWN))
            return (0);
        sp->cinfo.c.comp_info[0].component_id = s;
        /* jpeg_set_colorspace() set sampling factors to 1 */
        if (sp->photometric == PHOTOMETRIC_YCBCR && s > 0) 
        {
            sp->cinfo.c.comp_info[0].quant_tbl_no = 1;
            sp->cinfo.c.comp_info[0].dc_tbl_no = 1;
            sp->cinfo.c.comp_info[0].ac_tbl_no = 1;
        }
    }
    /* ensure libjpeg won't write any extraneous markers */
    sp->cinfo.c.write_JFIF_header = FALSE;
    sp->cinfo.c.write_Adobe_marker = FALSE;
    /* set up table handling correctly */
    if (! (sp->jpegtablesmode & JPEGTABLESMODE_QUANT)) 
    {
        if (!TIFFjpeg_set_quality(sp, sp->jpegquality, FALSE))
            return (0);
        unsuppress_quant_table(sp, 0);
        unsuppress_quant_table(sp, 1);
    }
    if (sp->jpegtablesmode & JPEGTABLESMODE_HUFF)
        sp->cinfo.c.optimize_coding = FALSE;
    else
        sp->cinfo.c.optimize_coding = TRUE;
	
    if (downsampled_input) 
    {
        /* Need to use raw-data interface to libjpeg */
        sp->cinfo.c.raw_data_in = TRUE;
    } 
    else 
    {
        /* Use normal interface to libjpeg */
        sp->cinfo.c.raw_data_in = FALSE;
    }
    /* Start JPEG compressor */
    if (!TIFFjpeg_start_compress(sp, FALSE))
        return (0);
    /* Allocate downsampled-data buffers if needed */
    if (downsampled_input) 
    {
        if (!alloc_downsampled_buffers(sp->cinfo.c.comp_info,
                                       sp->cinfo.c.num_components))
            return (0);
    }
    sp->scancount = 0;

    return (1);
}

/*
 * Encode a chunk of pixels.
 * "Standard" case: incoming data is not downsampled.
 */
int TiffCodecJpeg::encode(tidata_t buf, tsize_t cc, tsample_t s)
{
    JPEGState *sp = (JPEGState *)tif_data;
    tsize_t nrows;
    JSAMPROW bufptr[1];

    (void) s;
    assert(sp != NULL);
    /* data is expected to be supplied in multiples of a scanline */
    nrows = cc / sp->bytesperline;
    if (cc % sp->bytesperline)
        cout << "Warning: JPEGEncode -- fractional scanline discarded" << endl;

    while (nrows-- > 0) 
    {
        bufptr[0] = (JSAMPROW) buf;
        if (TIFFjpeg_write_scanlines(sp, bufptr, 1) != 1)
            return (0);
        if (nrows > 0)
            tif_row++;
        buf += sp->bytesperline;
    }
    return (1);
}

/*
 * Encode a chunk of pixels.
 * Incoming data is expected to be downsampled per sampling factors.
 */
//static int
//JPEGEncodeRaw(tidata_t buf, tsize_t cc, tsample_t s)
int TiffCodecJpeg::encodeRaw(tidata_t buf, tsize_t cc, tsample_t s)
{
    JPEGState *sp = (JPEGState *)tif_data;
    JSAMPLE* inptr;
    JSAMPLE* outptr;
    tsize_t nrows;
    JDIMENSION clumps_per_line, nclump;
    int clumpoffset, ci, xpos, ypos;
    jpeg_component_info* compptr;
    int samples_per_clump = sp->samplesperclump;

    (void) s;
    assert(sp != NULL);
    /* data is expected to be supplied in multiples of a scanline */
    nrows = cc / sp->bytesperline;
    if (cc % sp->bytesperline)
        cout << "Warning: JPEGEncodeRaw -- fractional scanline discarded" << endl;

    /* Cb,Cr both have sampling factors 1, so this is correct */
    clumps_per_line = sp->cinfo.c.comp_info[1].downsampled_width;

    while (nrows-- > 0) 
    {
        /*
         * Fastest way to separate the data is to make one pass
         * over the scanline for each row of each component.
         */
        clumpoffset = 0;		/* first sample in clump */
        for (ci = 0, compptr = sp->cinfo.c.comp_info;
             ci < sp->cinfo.c.num_components;
             ci++, compptr++) 
        {
            int hsamp = compptr->h_samp_factor;
            int vsamp = compptr->v_samp_factor;
            int padding = (int) (compptr->width_in_blocks * DCTSIZE -
                                 clumps_per_line * hsamp);
            for (ypos = 0; ypos < vsamp; ypos++) 
            {
                inptr = ((JSAMPLE*) buf) + clumpoffset;
                outptr = sp->ds_buffer[ci][sp->scancount*vsamp + ypos];
                if (hsamp == 1) 
                {
                    /* fast path for at least Cb and Cr */
                    for (nclump = clumps_per_line; nclump-- > 0; ) 
                    {
                        *outptr++ = inptr[0];
                        inptr += samples_per_clump;
                    }
                } 
                else 
                {
                    /* general case */
                    for (nclump = clumps_per_line; nclump-- > 0; ) 
                    {
                        for (xpos = 0; xpos < hsamp; xpos++)
                            *outptr++ = inptr[xpos];
                        inptr += samples_per_clump;
                    }
                }
                /* pad each scanline as needed */
                for (xpos = 0; xpos < padding; xpos++) 
                {
                    *outptr = outptr[-1];
                    outptr++;
                }
                clumpoffset += hsamp;
            }
        }
        sp->scancount++;
        if (sp->scancount >= DCTSIZE) 
        {
            int n = sp->cinfo.c.max_v_samp_factor * DCTSIZE;
            if (TIFFjpeg_write_raw_data(sp, sp->ds_buffer, n) != n)
                return (0);
            sp->scancount = 0;
        }
        if (nrows > 0)
            tif_row++;
        buf += sp->bytesperline;
    }
    return (1);
}

/*
 * Finish up at the end of a strip or tile.
 */
int TiffCodecJpeg::postEncode()
{
    JPEGState *sp = (JPEGState *)tif_data;
    
    if (sp->scancount > 0) 
    {
        /*
         * Need to emit a partial bufferload of downsampled data.
         * Pad the data vertically.
         */
        int ci, ypos, n;
        jpeg_component_info* compptr;

        for (ci = 0, compptr = sp->cinfo.c.comp_info;
             ci < sp->cinfo.c.num_components;
             ci++, compptr++) 
        {
            int vsamp = compptr->v_samp_factor;
            tsize_t row_width = compptr->width_in_blocks * DCTSIZE
                * sizeof(JSAMPLE);
            for (ypos = sp->scancount * vsamp;
                 ypos < DCTSIZE * vsamp; ypos++) 
            {
                memcpy((tdata_t)sp->ds_buffer[ci][ypos],
                       (tdata_t)sp->ds_buffer[ci][ypos-1],
                       row_width);

            }
        }
        n = sp->cinfo.c.max_v_samp_factor * DCTSIZE;
        if (TIFFjpeg_write_raw_data(sp, sp->ds_buffer, n) != n)
            return (0);
    }

    return (TIFFjpeg_finish_compress((JPEGState *)tif_data));
}

void TiffCodecJpeg::cleanup()
{
    if (tif_data) 
    {
        JPEGState *sp = (JPEGState *)tif_data;
        TIFFjpeg_destroy(sp);		/* release libjpeg resources */
        if (sp->jpegtables)		/* tag value */
            free(sp->jpegtables);
        free(tif_data);	/* release local state */
        tif_data = NULL;
    }
}

tiff_uint32 TiffCodecJpeg::defStripSize(tiff_uint32 s)
{
    JPEGState* sp = (JPEGState*)tif_data;

    s = defaultStripSize(s);
    if (s < tiffdir->imageLength())
    {
        vector<uint16> ycbcrsubsampling = tiffdir->yccSubSampling();
        s = TIFFroundup(s, ycbcrsubsampling[1] * DCTSIZE);
    }
    return (s);
}

void TiffCodecJpeg::defTileSize(tiff_uint32* tw, tiff_uint32* th)
{
    JPEGState* sp = (JPEGState*)tif_data;
    
    defaultTileSize(tw, th);
    vector<uint16> ycbcrsubsampling = tiffdir->yccSubSampling();
    *tw = TIFFroundup(*tw, ycbcrsubsampling[0] * DCTSIZE);
    *th = TIFFroundup(*th, ycbcrsubsampling[1] * DCTSIZE);
}

int TiffCodecJpeg::init(TiffDirectory* tdir, TiffIO* tio, TiffImage* img)
{
    if (tiffdir)
    {
        assert(tiffio);
        return 1;
    }
    tiffdir = tdir;
    tiffio = tio;
    tiffimg = img;
    
    JPEGState* sp;
    
    /*
     * Allocate state block so tag methods have storage to record values.
     */
    tif_data = (tidata_t) malloc(sizeof(JPEGState));
    if (tif_data == NULL) 
    {
        string m = "No space for JPEG state block";
        throw TiffError(TIFF_MEM_ALLOC_FAIL, m, EKTIFF_WHERE);
    }
    
    sp = (JPEGState*)tif_data;
    sp->codec = this;	// back link
    
    /* Default values for codec-specific fields */
    sp->jpegtables = NULL;
    sp->jpegtables_length = 0;
    sp->jpegquality = 75;			/* Default IJG quality */
    sp->jpegcolormode = JPEGCOLORMODE_RAW;
    sp->jpegtablesmode = JPEGTABLESMODE_QUANT | JPEGTABLESMODE_HUFF;
    
    tiffio->flags() |= TIFF_NOBITREV;	/* no bit reversal, please */

    /*
     * Initialize libjpeg.
     */
    if (tiffio->mode() == O_RDONLY) 
    {
        if (!TIFFjpeg_create_decompress(sp))
            return (0);
    } 
    else 
    {
        if (!TIFFjpeg_create_compress(sp))
            return (0);
    }
    
    return (1);
}


int TiffCodecJpeg::decodeRow(tidata_t buf, tsize_t cc, tsample_t s)
{
    JPEGState *sp = (JPEGState *)tif_data;
    
    if (sp->cinfo.d.raw_data_out)
        return (decodeRaw(buf, cc, s));
    else
        return (decode(buf, cc, s));
}

int TiffCodecJpeg::encodeRow(tidata_t buf, tsize_t cc, tsample_t s)
{
    JPEGState *sp = (JPEGState *)tif_data;
    
    if (sp->cinfo.c.raw_data_in)
        return (encodeRaw(buf, cc, s));
    else
        return (encode(buf, cc, s));
}

int TiffCodecJpeg::decodeStrip(tidata_t buf, tsize_t cc, tsample_t s)
{
    JPEGState *sp = (JPEGState *)tif_data;
    
    if (sp->cinfo.d.raw_data_out)
        return (decodeRaw(buf, cc, s));
    else
        return (decode(buf, cc, s));
}

int TiffCodecJpeg::encodeStrip(tidata_t buf, tsize_t cc, tsample_t s)
{
    JPEGState *sp = (JPEGState *)tif_data;
    
    if (sp->cinfo.c.raw_data_in)
        return (encodeRaw(buf, cc, s));
    else
        return (encode(buf, cc, s));
}

int TiffCodecJpeg::decodeTile(tidata_t buf, tsize_t cc, tsample_t s)
{
    JPEGState *sp = (JPEGState *)tif_data;
    
    if (sp->cinfo.d.raw_data_out)
        return (decodeRaw(buf, cc, s));
    else
        return (decode(buf, cc, s));
}

int TiffCodecJpeg::encodeTile(tidata_t buf, tsize_t cc, tsample_t s)
{
    JPEGState *sp = (JPEGState *)tif_data;
    
    if (sp->cinfo.c.raw_data_in)
        return (encodeRaw(buf, cc, s));
    else
        return (encode(buf, cc, s));
    
}


void TiffCodecJpeg::getJpegInfo()
{
    JPEGState* sp = (JPEGState*)tif_data;

    // read in jpegtable
    TiffTagEntry* entry = tiffdir->getGenericTag(TIFFTAG_JPEGTABLES);
    if (!entry)
    {
        throw TiffError(TIFF_JPEG_ERROR, "No jpeg table", EKTIFF_WHERE);
    }
    sp->jpegtables_length = entry->getCount();
    sp->jpegtables = (void*)malloc(sp->jpegtables_length);
    if (!sp->jpegtables)
    {
        throw TiffError(TIFF_MEM_ALLOC_FAIL, "TiffCodecJpeg::readJpegInfo - failed to allocate memory", EKTIFF_WHERE);
    }
    const vector<int8>& v = (dynamic_cast<TiffTagEntryT<vector<int8> >*>(entry))->getValue();
    char* p = (char*)sp->jpegtables;
    for (int i=0; i<sp->jpegtables_length; i++)
        p[i]= v[i];

    // read in jpeg quality, colormode and tablesmode
    entry = tiffdir->getGenericTag(TIFFTAG_JPEGQUALITY);
    if (entry)
    {
        sp->jpegquality = (dynamic_cast<TiffTagEntryT<tiff_int32>*>(entry))->getValue();
    }
    entry = tiffdir->getGenericTag(TIFFTAG_JPEGCOLORMODE);
    if (entry)
    {
        sp->jpegcolormode = (dynamic_cast<TiffTagEntryT<tiff_int32>*>(entry))->getValue();
    }
    entry = tiffdir->getGenericTag(TIFFTAG_JPEGTABLESMODE);
    if (entry)
    {
        sp->jpegtablesmode = (dynamic_cast<TiffTagEntryT<tiff_int32>*>(entry))->getValue();
    }

}


void TiffCodecJpeg::setJpegInfo()
{
    JPEGState* sp = (JPEGState*)tif_data;

    vector<int8> v;
    char* p = (char*)sp->jpegtables;
    for (int i=0; i<sp->jpegtables_length; i++)
        v.push_back(p[i]);
    TiffTagEntry* entry = new TiffTagEntryT<vector<int8> >(TIFFTAG_JPEGTABLES, EKTIFF_UNDEFINED, sp->jpegtables_length, v);
    tiffdir->setGenericTag(*entry);
    delete entry;
}


#endif // JPEG_SUPPORT




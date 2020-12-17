/*===============================================================================

  FILE: mat2las.cpp

 * Write LAStools lasreader header and points from Matlab struct
 * >> mat2las(str,parsestring);
 *
 * the parsestring contains the name of the output file:
 * >> parsestring = ['-o ',fullfile('path\filename.laz')];
 *

  PROGRAMMERS:
  		paula.litkey@nls.fi  - https://orcid.org/0000-0002-8934-6985
 		eetu.puttonen@nls.fi - https://orcid.org/0000-0003-0985-4443
 		mariana.campos@nls.fi - https://orcid.org/0000-0003-3430-7521

  COPYRIGHT UNDER MIT LICENSE:
  
    (c) 2020, National Land Survey of Finland - Finnish Geospatial Research Institute
    (c) 2015, Finnish Geospatial Research Institute, FGI
    (c) 2014, Finnish Geodetic Institute

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE..

  CHANGE HISTORY:
    2013 -- First public matlas_tools version
    18 March 2014 -- First public matlas_tools version
    October 2015 -- Waveform read
    February 2020 -- Read laz point format > 5 
	March 2020 -- Read and Write ExtraBytes (MNGW6.3 for Matlab 2018b and beyond)
	 
	 
	 

=============================================================================== */

#include "mex.h"
#include <time.h>
#include <math.h>
#include "lasreader.hpp"
#include "laswriter.hpp"
#include "lasdefinitions.hpp"
#include "mex_lasz_fun_fgi.hpp"
#include "mex_lasz_fun_fgi.cpp"
#include "laszip_api.h"
#include "demzip_api.h"


void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    char *fname=NULL;
    int next = 0, rgbnext = 0, Nextra = 0, extranext = 0;
    I32 extra_attribute_array_offsets[10];
    LASpoint *point;
    LASheader *header = 0;
    mxArray *tmpD_14, *tmpD_15;
    mwSize M, NStructElems, NStructElems2;
    mwSize    nc, nr;
    int ifield, nfields, nfields2, nfieldsh;
    int argc = 0;
    int last = 1;
    char **argv;
    char *parse_string;
    size_t buflen;
    int set_gps_time_endcoding = -1;
    const mxArray *field_array_ptr;
    double *maxclass;
    int *tmpint;
    BOOL header_provided = false;
    I64 offset[3];
    I64 orig_x_offset, orig_y_offset, orig_z_offset;
    fgi_options* fgiopts;
    fgiopts = new fgi_options();

    int i;
    bool verbose = false;
    double start_time = 0.0;

/*CHECK INPUTS*/
    maxclass = (double *)mxCalloc(1,sizeof(double));
    if (!maxclass)
        mexErrMsgTxt("Memory error");
    *maxclass=0;

    if (nrhs == 0) {
        mexPrintf ("usage: writeLASZ_hdrstruct(data_struct,parsestring)\n");
        return;
    }
    if (!mxIsStruct(prhs[0])) mexErrMsgTxt ("First arg must contain Matlab data structure\n");
    if (mxIsStruct(prhs[1])) {
        last++;
        header_provided = true;
    }
    if (!mxIsChar(prhs[last])) mexErrMsgTxt ("Last arg must contain the parse string for lastools.\n");
    if (nlhs !=0) mexErrMsgTxt ("No outputs needed.\n");

/*copy the string data from prhs[2] into a C string "parse_string"*/
    parse_string = mxArrayToString(prhs[last]);
    // Get string arguments
    get_argc(argc,parse_string);
    // Allocate argv
    argv = (char **) mxCalloc(argc+1, sizeof(char *)); // Add one to 'argc' for command line compatibility
    // Fill argv
    set_argv(argv,parse_string);
    mxFree(parse_string);
    argc++;
    nfields = mxGetNumberOfFields(prhs[0]);
    mexPrintf("NumberOfFields = %d\n",nfields);
    fgiparse(argc, argv, fgiopts);

/* create header */
    header = new LASheader;
    header->clean();

    point = new LASpoint;
    if (header_provided)    {
        maxcoord(maxclass,mxGetPr(mxGetField(prhs[0],0,"classification")),mxGetScalar(mxGetField(prhs[1],0,"number_of_point_records")));
        if (*maxclass > 31 && mxGetScalar(mxGetField(prhs[1],0,"point_data_format"))<6) {
            mexErrMsgTxt ("The selected point data format does not allow for classification codes bigger than 31.\n Change the classification codes or the point data format and the corresponding point record length\n");
        }

        if (mxGetFieldNumber(prhs[0], "wavepacket")!= -1) {
            mexPrintf ("Warning: waveform writing not supported!\n");
        }
        read_mex_hdr(header,point,prhs[1],prhs[0]);
    }
    else {
        read_mex(header,point,prhs[0]);
    }
    if (fgiopts->set_offset) {
        header->x_offset = fgiopts->x_offset;
        header->y_offset = fgiopts->y_offset;
        header->z_offset = fgiopts->z_offset;
    }
    if (fgiopts->set_scale)
    {
        header->x_scale_factor = fgiopts->x_scale_factor;
        header->y_scale_factor = fgiopts->y_scale_factor;
        header->z_scale_factor = fgiopts->z_scale_factor;
        if (!fgiopts->set_offset) {
            // auto_reoffset copied from lasreader_las.cpp
            if (F64_IS_FINITE(header->min_x) && F64_IS_FINITE(header->max_x))
                offset[0] = ((I64)((header->min_x + header->max_x)/header->x_scale_factor/20000000))*10000000*header->x_scale_factor;
            else
                offset[0] = 0;

            if (F64_IS_FINITE(header->min_y) && F64_IS_FINITE(header->max_y))
                offset[1] = ((I64)((header->min_y + header->max_y)/header->y_scale_factor/20000000))*10000000*header->y_scale_factor;
            else
                offset[1] = 0;

            if (F64_IS_FINITE(header->min_z) && F64_IS_FINITE(header->max_z))
                offset[2] = ((I64)((header->min_z + header->max_z)/header->z_scale_factor/20000000))*10000000*header->z_scale_factor;
            else
                offset[2] = 0;

            orig_x_offset = header->x_offset;
            orig_y_offset = header->y_offset;
            orig_z_offset = header->z_offset;

            if (header->x_offset != offset[0])
            {
                header->x_offset = offset[0];
            }

            if (header->y_offset != offset[1])
            {
                header->y_offset = offset[1];
            }

            if (header->z_offset != offset[2])
            {
                header->z_offset = offset[2];
            }
        }
    }

    if (!header->check()) mexErrMsgTxt("header not valid!!\n");
    LASwriteOpener laswriteopener;
    if (!laswriteopener.parse(argc, argv)) mexErrMsgTxt("laswriteopener parse error!!\n");
    LASwriter* laswriter = laswriteopener.open(header);
    if (laswriter == 0) mexErrMsgTxt("ERROR: could not open laswriter\n");
    if (!laswriter->update_header(header, false, true) ) {
        mexWarnMsgTxt("Writer header update not succesfull\n");
    }

    field_array_ptr = mxGetFieldByNumber(prhs[0], 0, 0); // pointer to struct mxArray holding points
    nr = mxGetM(field_array_ptr);
    nc = mxGetN(field_array_ptr);
    mexPrintf("Looping %d points\n",nr);

    while (next < nr)

    {
        copy_point_arr_mex(header,point,prhs[0],next);
        next++;
        laswriter->write_point(point);
        laswriter->update_inventory(point);
    }


    //laswriter->update_header(header,1,1);
    laswriter->update_header(header,TRUE);

    I64 total_bytes = laswriter->close();

#ifdef _WIN32
  if (verbose) fprintf(stderr,"total time: %g sec %I64d bytes for %I64d points\n", taketime()-start_time, total_bytes, laswriter->p_count);
#else
  if (verbose) fprintf(stderr,"total time: %g sec %lld bytes for %lld points\n", taketime()-start_time, total_bytes, laswriter->p_count);
#endif

    //delete point;
    //point = 0;
    //laswriter->close();
    //delete header;
    //header = 0;
    delete laswriter;
    mxFree(maxclass);
}



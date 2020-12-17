/*
===============================================================================

  FILE:  las2hdr.mat

  CONTENTS:

    Matlab mex gateway function for using LASlib writer from Matlab
    (test.laz can be found in lastools/data/)
    >> [hdr,str] = las2mat('-i test.laz')
    >> parsestring = '-i test.laz -clip 2483789 366396 2483809 366416 -keep_class 2 ';
    >> [hdr,str] = las2mat(parsestring)


  PROGRAMMERS:
             paula.litkey@nls.fi   - https://orcid.org/0000-0002-8934-6985
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

    18 March 2014 -- First public matlas_tools version
    October 2015 -- Waveform read
    February 2020 -- Read laz point format > 5 
	March 2020 -- Read and Write ExtraBytes (MNGW6.3 for Matlab 2018b and beyond)

===============================================================================
*/

#include "mex.h"
//#include <time.h>
#include <math.h>
#include "lasreader.hpp"
#include "laswaveform13reader.hpp"
#include "lasdefinitions.hpp"
#include "lasutility.hpp"
#include "mex_lasz_fun_fgi.hpp" 
#include "mex_lasz_fun_fgi.cpp" 

void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    int next = 0, Nextra = 0, extranext = 0;
    int wfnext = 0;
    int NUMBER_OF_HDRFIELDS = 33;
    int NUMBER_OF_PNTFIELDS = 12;
    I32 attribute_array_offsets[10];
    mxArray *tmpI32;
    LASpoint *point;
    int argc = 0;
    int wfs_num = 0;
    char **argv;
    char *parse_string;
    int samples=0;
    int WFMAXSAMPLES=0;
    fgi_options* fgiopts;
    fgiopts = new fgi_options();
    
    if (nrhs == 0) {
        mexPrintf ("usage: las2mat(lastools parsestring)\n");
        return;
    }
    if (!mxIsChar(prhs[0])) mexErrMsgTxt ("first arg must contain the parse string for lastools.\n");
    if (nlhs !=1) mexErrMsgTxt ("One output needed.( header struct )\n");
    
    /* copy the string data from prhs[2] into a C string "parse_string".    */
    parse_string = mxArrayToString(prhs[0]);
    get_argc(argc,parse_string);
    argv = (char **) mxCalloc(argc+1, sizeof(char *)); // Add one to 'argc' for command line compatibility
    
    // Fill argv
    set_argv(argv,parse_string);
    mxFree(parse_string);
    argc++;
    fgiparse(argc, argv, fgiopts);
    LASreadOpener lasreadopener;
    if (!fgiopts->header && !fgiopts->populate)
    {
        mexPrintf("no header detected!\n");
        lasreadopener.set_populate_header(true);
    }
    if (!lasreadopener.parse(argc, argv)) mexErrMsgTxt("lasreadopener parse error!!\n");
    LASreader *lasreader = lasreadopener.open();
    if (!lasreader) mexErrMsgTxt("LASREADER Error! could not open lasreader!");
    lasreader->header.clean_laszip(); // re-created if needed in write
    LASwaveform13reader *laswaveform13reader = lasreadopener.open_waveform13(&lasreader->header);
    /*if (!laswaveform13reader) {
        mexPrintf ("Waveform reader not opened\n");
    }*/

    LASheader *header = &(lasreader->header);
    if (!header) mexErrMsgTxt("LASREADER: Unable to fetch header for file");
    header->clean_laszip();
    plhs[0] = mxCreateStructMatrix(1, 1, NUMBER_OF_HDRFIELDS, hdr_field_names);
    WFMAXSAMPLES = get_header_fields(header, plhs[0]);
        
    if (header->number_attributes >0) { // init variables to read extra attributes
        Nextra = header->number_attributes;
        
        mxAddField(plhs[0],"attribute_array_offsets");
        tmpI32 = mxCreateDoubleMatrix(1,header->number_attributes,mxREAL);
        double *pI32 = mxGetPr(tmpI32);
        for (int i=0; i<header->number_attributes; i++) {
            attribute_array_offsets[i] = lasreader->header.get_attribute_start(i);
            *(pI32+i) = lasreader->header.get_attribute_start(i);
        }
        mxSetField(plhs[0], 0, "attribute_array_offsets", tmpI32);
    }
    
//     for (wfnext = 0; wfnext < wfs_num; wfnext++) {
//         mexPrintf("\t%d",wfnext);
//         for (int i = 0; i < WFMAXSAMPLES; i++)
//         {
//             *(pwp+(wfs_num*wfnext+i)) = wfs_num*wfnext+i;
//             mexPrintf("\t%d",WFMAXSAMPLES*wfnext+i);
//         }
//         mexPrintf("\n");
//     }
    

    if (laswaveform13reader)
    {
        laswaveform13reader->close();
        delete laswaveform13reader;
    }


    // close the reader
    lasreader->close();
    delete lasreader;
    delete point;
    point = 0;
    mxFree(argv);
}



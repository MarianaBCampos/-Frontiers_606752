/*
===============================================================================
FILE: main.CPP 20.11.2020 v.01

CONTENTS:
This source code generate a dynamic library with 2 main functions to:
Read  > LAZ2NP > converts LAS format or - its compressed, but identical twin - the LAZ format - into numpy arrays
Write > NP2LAZ > save LAS format or - its compressed, but identical twin - the LAZ format - from numpy arrays

VERSIONS:
--> MinGW GCC 8.1 (64 bits) and 
--> Python 3.7 (64 bits), 
--> LAStools v200131

REFERENCES and LIBRARIES COPYRIGHT:
(c) LASZIP EXAMPLES: 2007-2014, martin isenburg, rapidlasso - fast tools to catch reality
This is free software; you can redistribute and/or modify it under the
terms of the GNU Lesser General Licence as published by the Free Software
Foundation. See the LICENSE.txt file for more information.
This software is distributed WITHOUT ANY WARRANTY and without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
martin.isenburg@rapidlasso.com - http://rapidlasso.com
SEE FOLDER:LAStools\LASzip\example

PROGRAMMERS:

MARIANA BATISTA CAMPOS - mariana.campos@nls.fi
RESEARCHER AT FINNISH GEOSPATIAL INSTITUTE - NATIONAL LAND SURVEY OF FINLAND

COPYRIGHT UNDER MIT LICENSE

Copyright (C) 2019-2020, NATIONAL LAND SURVEY OF FINLAND

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
SOFTWARE.

HISTORY:
DEC/2020 - FIRST RELEASE
======================================================================================
*/

/*
======================================================================================
I.PROJECT CONFIGURATION (WIN) > CHECK PDF INSTRUCTION FOR LINUX COMPILATION 

At Code Blocks --> Open dynamic library project
Change compiler at:
Settings --> Compiler --> Global compiler settings --> Toolchain Executable  Find MinGW GCC 8.1.
Common path: C:\Program Files\CodeBlocks\MinGW
PROJECT --> Build options --> Compiler settings -->Compiler Flags [-std=c++11]
PROJECT --> Build options --> Compiler settings --> OtherCompilerOptions:
																					-fexceptions
																					-fpermissive
PROJECT --> Build options --> Linker settings --> Linker options include standard C/C++ libs:
kernel32, user32, gdi32, winspool, comdlg32, advapi32, shell32, ole32, oleaut32, uuid,odbc32
PROJECT -->Build options --> Search directory -->compiler --> point to:
																		Lastools/Laslib/src
																	    Lastools/Laslib/inc
																		Lastools/Laszip/src
																		Lastools/Laszip/dll

PROJECT -->Build options --> Search directory --> linker --> point to:
																Lastools/Laslib/src
																Lastools/Laslib/inc
																Lastools/Laszip/src
																Lastools/Laszip/dll
On the workspace --> Right click on the project name --> add files
HEADERS:
LASZIP – ALL HEADERS (.h) from LASzip/src and LASzip/dll
LASLIB – ALL HEADERS (.h) at LASlib/inc
MAIN HEADER - main.h

SOURCES:
LASZIP – ALL SOURCES(.cpp) from LASzip/src and LASzip/dll
LASLIB – ALL SOURCES(.cpp) at LASlib/src
MAIN SOUCE CODE - main.cpp

At this point modifications at main.h and main.cpp can be required according to user application
and the Las fields required to read and write. If more pointers are add in the C source code,
ReadLaz.py and WriteLaz.py scripts also need to be modified.
======================================================================================
*/
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <windows.h>
#include <cstring>

#include "main.h"
#include "laszip_api.h"
#include "lasreader.hpp"
#include "laswriter.hpp"
#include "lasdefinitions.hpp"
#include "lasutility.hpp"

#if defined(_MSC_VER) && \
    (_MSC_FULL_VER >= 150000000)
#define LASCopyString _strdup
#else
#define LASCopyString strdup
#endif

#define N_TARG 1024

void usage(bool wait=false)
{
  fprintf(stderr,"usage:\n");
  fprintf(stderr,"lasexample_write_only out.las\n");
  fprintf(stderr,"lasexample_write_only -o out.las -verbose\n");
  fprintf(stderr,"lasexample_write_only > out.las\n");
  fprintf(stderr,"lasexample_write_only -h\n");
  if (wait)
  {
    fprintf(stderr,"<press ENTER>\n");
    getc(stdin);
  }
  exit(1);
}

static void byebye(bool error=false, bool wait=false)
{
  if (wait)
  {
    fprintf(stderr,"<press ENTER>\n");
    getc(stdin);
  }
  exit(error);
}

static double taketime()
{
  return (double)(clock())/CLOCKS_PER_SEC;
}

int main (){

 printf("CONNECTION DONE\n");

}


int Read(char* name, double*&x, double*&y, double*&z, int*&return_number,  int*&number_of_returns, float*&intensity,  float**&pm , char** &name_attribute, int*&NEB){


//----------------------------------LOAD DLL ----------------------------------
  double start_time = 0.0;
  char* file_name_in = 0;
  file_name_in = name;
  int argc=1;

 //----------------------------------OPEN LAZ FILE ----------------------------------
  start_time = taketime();
  fprintf(stderr,"Reading native compressed LAS 1.4 and copying to array)\n");

  LASreadOpener lasreadopener;
  lasreadopener.set_file_name(file_name_in);
  if (!lasreadopener.active()) { fprintf(stderr,"ERROR: no input specified\n"); usage(argc == 1);}

  LASreader* lasreader = lasreadopener.open();
  if (lasreader == 0){fprintf(stderr, "ERROR: could not open lasreader\n"); byebye(argc==1);}
  LASheader *header = &(lasreader->header);

  // how many points does the file have
  laszip_I64 npoints = (header->number_of_point_records ? header->number_of_point_records : header->extended_number_of_point_records);
  // report how many points the file has
  fprintf(stderr,"file '%s' contains %I64d points\n", file_name_in, npoints);

// END OPEN LAZ FILE


// ------------------------------   CREATING ARRAYS   -------------------------------------------

     //AUXILIAR VARIABLES
     int p_count = 0;
     int next = 0, Nextra = 0, extranext = 0;
     int l, i;
     int MAX_STRING=20;
     Nextra=lasreader->header.number_attributes;
     NEB=Nextra;
     name_attribute[Nextra][MAX_STRING];
     float x_scale_factor = header->x_scale_factor;
     float y_scale_factor = header->y_scale_factor;
     float z_scale_factor = header->z_scale_factor;

     //CREATING ARRAYS --> np.array
     x = (double*)malloc(npoints* sizeof(double));
     y = (double*)malloc(npoints* sizeof(double));
     z = (double*)malloc(npoints* sizeof(double));
     return_number = (int*)malloc(npoints* sizeof(int));
     number_of_returns = (int*)malloc(npoints* sizeof(int));
     name_attribute=(char**)malloc(Nextra*sizeof (char*));


     //CHECKING USER INPUT AND MALLOC
     intensity = (float*)malloc(npoints* sizeof(float));
     //scan_direction_flag = (int*)malloc(npoints* sizeof(int));
     //edge_of_flight_line = (float*)malloc(npoints* sizeof(float));
     //classification = (float*)malloc(npoints* sizeof(float));
     //user_data = (float*)malloc(npoints* sizeof(float));
     //gps_time = (float*)malloc(npoints* sizeof(float));


    //EXTRA BYTES
    //LIMIT FOR EXTRA BYTES 1.4 SPECIFICATION - Record Length after Header: n records x 192 bytes

    if (lasreader->header.number_attributes > 0) { // init variables to read extra attributes
         pm = (float **) calloc(Nextra,sizeof(float *));
         for(l=0;l<Nextra;l++) {
            pm[l] = (float*)malloc(npoints*sizeof(float));
            name_attribute[l]=(char*) malloc(MAX_STRING * sizeof(char));
            printf("Name of attributes: %s\n", header->get_attribute_name(i));
            strcpy(name_attribute[l],header->get_attribute_name(l));
           }
     }


     while (lasreader->read_point()){

      // COPING COORDINATES
      x[p_count]=lasreader->point.get_x();
      y[p_count]=lasreader->point.get_y();
      z[p_count]=lasreader->point.get_z();

      // COPING ATTRIBUTES
      intensity[p_count] = lasreader->point.get_intensity();

      //float scale_attribute=0.01;
      //COPING EXTRA BYTES
      if (header->number_attributes>0) {
      F32 value1; I16 value2; F64 value3;
      I32* TT = lasreader->header.attribute_sizes;
          for (i=0; i<Nextra; i++) {
               if(TT[i] == sizeof(float)){
                lasreader->point.get_attribute(lasreader->header.get_attribute_start(i), value1);
                pm[i][p_count]=value1;}

               if(TT[i] == sizeof(short)){
                lasreader->point.get_attribute(lasreader->header.get_attribute_start(i), value2);
                pm[i][p_count]=value2;}

               if(TT[i] == sizeof(double)){
                 lasreader->point.get_attribute(lasreader->header.get_attribute_start(i), value3);
                 pm[i][p_count]=value3;}
          }
      }



      // COPING RETURN NUMBER AND NUMBER OF RETURNS
      if (header->point_data_format < 5)
      {
        return_number[p_count] = lasreader->point.get_return_number();
        number_of_returns[p_count] =lasreader->point.get_number_of_returns();
        //classification[p_count] = point_read->extended_classification;

      }

      else
      {
        return_number[p_count] = lasreader->point.get_extended_return_number();
        number_of_returns[p_count] =lasreader->point.get_extended_number_of_returns();
        //classification[p_count] = point_read->classification;
      }

      p_count++;
    }

    fprintf(stderr,"successfully read and written %I64d points\n", p_count);


    // close the reader
     lasreader->close();

    // destroy the reader
      delete lasreader;


  return npoints;
}

int Write(char* name, double*x, double*y, double*z, int*return_number,  int*number_of_returns, float*intensity, float*classification, const int *p_count, int* NEB,  char** name_attribute, float**pm_att){
//int Write(char* name, double*x, double*y, double*z, int*return_number,  int*number_of_returns, float*intensity, float*classification, const int *p_count, int* NEB,  char** name_attribute,  short**pm_att){
// ----------------------READING FILE NAME------------------------

//LASZIP Variables
laszip_I64 number_ExtraBytes = 0;
int number_attribute=NEB;
int npoints=p_count;
bool verbose = false;
double start_time = 0.0;
char* file_name_out = 0;
int argc;
//Atributtes Variables
F32 scale=1;
F32 offset=0.0;
I32 temp_i;
I32 Temp_Att;
I32 *Temp_Start;
int i;
int n;

// ------------------CREATING WRITER -------------------

LASwriteOpener laswriteopener;
file_name_out = name;
laswriteopener.set_file_name(file_name_out);

start_time = taketime();

if (!laswriteopener.active())
  {
    fprintf(stderr,"ERROR: no output specified\n");
    usage(argc == 1);
  }


// ----------------------------------HEADER-------------------------------

// get a pointer to the header of the writer so we can populate it
LASheader lasheader;

//GET TIME OF CREATION
time_t t = time(NULL);
tm* timePtr = localtime(&t);

//HEADER DEFINE
lasheader.file_source_ID = 4711;
lasheader.global_encoding = (1<<0) | (1<<4);// see LAS specification for details
//header->global_encoding = 0x0001; // time stamps are in adjusted standard GPS time
lasheader.version_major=1;
lasheader.version_minor=4;
lasheader.file_creation_day=timePtr->tm_mday;
lasheader.file_creation_year=timePtr->tm_year+1900;
lasheader.header_size = 375; // must be 375 for LAS 1.4
lasheader.offset_to_point_data = 375;
//lasheader.point_data_format = 1;//Version 1.4(V1)to use all extra bytes
lasheader.point_data_format = 1;//Version 1.4(0 to 10), but 6 to 10  are preferred formats
lasheader.number_of_point_records = 0;// legacy 32-bit counters should be zero for new point types > 5
lasheader.x_scale_factor = 0.0001;//X,Y,Z recorded on mm level
lasheader.y_scale_factor = 0.0001;
lasheader.z_scale_factor = 0.0001;
lasheader.x_offset = 0.0;
lasheader.y_offset = 0.0;
lasheader.z_offset = 0.0;


// SET EXTRA BYTES: 30 + using short format(2 extra bytes per info)
//header->point_data_record_length = 30 + 12;//reflectance, range, deviation, row, line #TO BE USED WITH FORMAT 6.0
lasheader.point_data_record_length = 28;//reflectance, range, deviation, theta, phi, row, line #TO BE USED WITH FORMAT 1.0
lasheader.start_of_waveform_data_packet_record = 0;
lasheader.number_of_extended_variable_length_records = 0;
lasheader.start_of_first_extended_variable_length_record = 0;
//SET MAX AND MIN VALUES OF X,Y,Z
//lasheader.max_x =;
//lasheader.min_x =;
//lasheader.max_y =;
//lasheader.min_y =;
//lasheader.max_z =;
//lasheader.min_z =;

// array of the total point records per return.
for (i = 0; i < 5; i++)//standard - 0 to 5
    {
      lasheader.number_of_points_by_return[i] = 0;
    }
//SET record return number - extension available at 1.4 format - MAX 15
lasheader.extended_number_of_point_records = 0;// a-priori unknown number of points
for (i = 0; i < 15; i++)
    {
      lasheader.extended_number_of_points_by_return[i] = 0;
    }

//DECLARING EXTRA BYTES
Temp_Start = (I32*)malloc(number_attribute * sizeof(I32));

for (i = 0; i < number_attribute; i++){
// add description for the  attributes in the  "extra bytes": reflectance, deviation and range
Temp_Att=-1;

  try {
    I32 type = 8; // unsigned short
    LASattribute attribute(type, name_attribute[i], name_attribute[i]);
    attribute.set_scale(scale);
    attribute.set_offset(offset);
    Temp_Att= lasheader.add_attribute(attribute);
  }

  catch(...) {
    fprintf(stderr,"ERROR: initializing first additional attribute\n");
    usage(argc == 1);
  }

Temp_Start[i]= lasheader.get_attribute_start(Temp_Att);

}
lasheader.update_extra_bytes_vlr();
lasheader.point_data_record_length += lasheader.get_attributes_size();
//END OF HEADER DEFINITION
// -------------------------------------------------------------


/** ------------------------WRITING-----------------------------
// I - OPEN THE WRITER*/
LASpoint laspoint;
laspoint.init(&lasheader, lasheader.point_data_format, lasheader.point_data_record_length, 0);


/** II - Define a pointer to the point of the writer that we will populate and write */
LASwriter* laswriter = laswriteopener.open(&lasheader);
//LASwriter* laswriter = laswriteopener.open(&lasreader->header);
  if (laswriter == 0)
  {
    fprintf(stderr, "ERROR: could not open laswriter\n");
    byebye(argc==1);
  }

  if (verbose) fprintf(stderr, "writing 100 points to '%s'.\n", laswriteopener.get_file_name());


/**-------------------------------------------------------------
 V - populating points*/

for (n=0; n<npoints; ++n) {

//saving standard parameters
//intensity, return number, number of returns, classification, scan angle, scanner channel, gps_time
        laspoint.set_X(x[n]);
        laspoint.set_Y(y[n]);
        laspoint.set_Z(z[n]);
        laspoint.set_intensity((intensity[n])) ;
        laspoint.set_number_of_returns(number_of_returns[n]);
        laspoint.set_return_number(return_number[n]);
        laspoint.set_classification(classification[n]);
        //laspoint.set_gps_time(23365829.0 + 0.0006*i);


// set attribute 'reflectance', 'Deviation' and 'Range'
for (i = 0; i < number_attribute; i++){

F32 temp_f =((pm_att[i][n]-offset)/scale);
laspoint.set_attribute(Temp_Start[i], (F32(temp_f)));

} //END FOR ATTRIBUTES


// write the point
laswriter->write_point(&laspoint);
// add it to the inventory
laswriter->update_inventory(&laspoint);
} //END FOR
// ----------------------------------------------------------------------

// update the header
laswriter->update_header(&lasheader, TRUE);
// close the writer
I64 total_bytes = laswriter->close();

#ifdef _WIN32
  if (verbose) fprintf(stderr,"total time: %g sec %I64d bytes for %I64d points\n", taketime()-start_time, total_bytes, laswriter->p_count);
#else
  if (verbose) fprintf(stderr,"total time: %g sec %lld bytes for %lld points\n", taketime()-start_time, total_bytes, laswriter->p_count);
#endif

delete laswriter;

return 0;

 }




void freeme(double* x, double* y, double* z, int* return_number,  int* number_of_returns, float* intensity, float* classification,  float** pm, char**  name_attribute, int* NEB){ //FREE MEMORY

    free(x);
    x=NULL;
    free(y);
    y=NULL;
    free(z);
    z=NULL;

    free (return_number);
    return_number=NULL;
    free (number_of_returns);
    number_of_returns=NULL;
    free (intensity);
    intensity=NULL;
    free (classification);
    classification=NULL;

    int j;

    for(j=0; j<NEB; j++){
            free(pm[j]);
            pm[j]=NULL;
            free(name_attribute[j]);
            name_attribute[j]=NULL;
    }

    free(pm);
    free(name_attribute);
    pm=NULL;
    name_attribute=NULL;


}

/*
===============================================================================
FILE: Rxp2Lasdll

CONTENTS:
This source code converts rxp riegl format to LAS format (1.4)
or - its compressed, but identical twin - the LAZ format.
via RIVLIB (dll) + LasLib (dll)

VERSIONS:
--> CODEBLOCKS 16.01 + mingw 4.9 (GCC 4.9)
--> RIVLIB: rivlib-2_5_7-x86-windows-mgw49(Riegl DLL v7.1)
--> LASzip DLL v3.4 r1 (build 190728)

REFERENCES and LIBRARIES COPYRIGHT:
(c) Riegl 2008
pointclouddll.c - Example for reading pointcloud data (PC EXAMPLE)
SEE FOLDER:RIVLIB\doc\rivlib\examples

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

DEC/2020 - FIRST VERSION RELEASE
======================================================================================
*/

/*
======================================================================================
I.PROJECT CONFIGURATION
PROJECT (CLICK ON THE PROJECT NAME)
NOTE: rivilib expects a working C++ 11 setup - more instructions in CmakeLists.txt
PROJECT --> Build options --> Compiler settings --> Compiler Flags [-std=c++11] * Compatibility with RIVLIB
PROJECT --> Build options --> Compiler settings --> OtherCompilerOptions:
                                                                                    -std=gnu++11
                                                                                    -std=gnu++0x
                                                                                    -fexceptions

PROJECT --> Build options --> Linker settings --> Linker options -->include standard C/C++ libs (kernel32, user32, gdi32, winspool comdig32 advapi32,
                                                                                                 shell32, ole32, oleaut32, uuid, odbc32, obdc32)

PROJECT --> Build options --> Linker settings --> Other Linker options:
                                                                        -static-libstdc++
                                                                        -static-libgcc
                                                                        -static
                                                                        -Wl,--allow-multiple-definition

PROJECT --> Build options --> Search directory --> compiler/linker --> point to Lastools/Laslib/src
                                                                                Lastools/Laslib/inc
                                                                                Lastools/Laszip/src
                                                                                Lastools/Laszip/dll
                                                                                RIVLIB/include
                                                                                RIVLIB/lib

TO RUN IN DEBUG MODE (CLICK ON DEBUG)
SET APPEND TARGET OPTIONS TO PROJECT OPTIONS
PROJECT --> Build options --> Compiler settings (SET THE SAME CONF. AS PROJECT) - OPTIONAL
PROJECT --> Build options --> Linker settings --> Linker options -->include RIVLIB libraries (-s.lib)
PROJECT --> Build options --> Search directory --> compiler/linker --> point to
                                                                                RIVLIB/include
                                                                                RIVLIB/lib                                                                                RIVLIB/lib
TO RUN IN RELEASE MODE (CLICK ON RELEASE)
SET APPEND TARGET OPTIONS TO PROJECT OPTIONS
PROJECT --> Build options --> Compiler settings (SET THE SAME CONF. AS PROJECT)- OPTIONAL
PROJECT --> Build options --> Linker settings --> Linker options -->include RIVLIB libraries (-s.lib)
PROJECT --> Build options --> Search directory --> compiler/linker --> point to
                                                                                RIVLIB/include
                                                                                RIVLIB/lib

II. INPUT FILES: (Right click on the project name --> add files)
HEADERS: laszip_api.h, ctrlifc.h, scanifc.h
SOURCES:laszip_api.c, Rxp2Lasdll.cpp


III. By default, an executable will be build at bin\Debug or bin\Release
To change this path set Project --> Properties --> Build Targets
To run the executable, copy to the folder RIVLIB (dll) and LasLib (dll)

*MORE DETAILS AT DOCUMENTATION FILE
======================================================================================
*/

//------------------------------------INCLUDES---------------------------------
// C/C++ DEFAULT
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define _USE_MATH_DEFINES

#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <sstream>
#include<algorithm>
// RIEGL and LASZIP DLL HEADER
#include <riegl/scanifc.h> // HEADER FOR RIEGL DLL
#include <riegl/pointcloud.hpp>
#include "laszip_api.h" // HEADER FOR LASZIP DLL

#if defined(_MSC_VER) && \
    (_MSC_FULL_VER >= 150000000)
#define LASCopyString _strdup
#else
#define LASCopyString strdup
#endif
/*
==============================================================================================
                            INIT FUNCTIONS FROM LASZIP AND RIEGL
==============================================================================================
*/

void print_last_error() //FROM PC EXAMPLE (RIEGL)
{
    char buffer[512];
    scanifc_uint32_t bs;
    scanifc_get_last_error(buffer, 512, &bs);
    printf("%s\n", buffer);
}


void usage(bool wait = false){ //FROM LASZIP EXAMPLE
	fprintf(stderr, "usage:\n");
	fprintf(stderr, "lasexample_write_only_with_extra_bytes out.las\n");
	fprintf(stderr, "lasexample_write_only_with_extra_bytes -o out.las -verbose\n");
	fprintf(stderr, "lasexample_write_only_with_extra_bytes > out.las\n");
	fprintf(stderr, "lasexample_write_only_with_extra_bytes -h\n");
	if (wait)
	{
		fprintf(stderr, "<press ENTER>\n");
		getc(stdin);
	}
	exit(1);
}


static void dll_error(laszip_POINTER laszip) //FROM LASZIP EXAMPLE
{
  if (laszip)
  {
    laszip_CHAR* error;
    if (laszip_get_error(laszip, &error))
    {
      fprintf(stderr,"DLL ERROR: getting error messages\n");
    }
    fprintf(stderr,"DLL ERROR MESSAGE: %s\n", error);
  }
}

static void byebye(bool error=false, bool wait=false, laszip_POINTER laszip=0) //FROM LASZIP EXAMPLE
{
  if (error)
  {
    dll_error(laszip);
  }
  if (wait)
  {
    fprintf(stderr,"<press ENTER>\n");
    getc(stdin);
  }
  exit(error);
}

static double taketime() //FROM LASZIP EXAMPLE
{
  return (double)(clock())/CLOCKS_PER_SEC;
}

int Check_Intersection(scanifc_xyz32 *pm, int countt, laszip_F64 sub_min_x, laszip_F64 sub_min_y, laszip_F64 sub_min_z,laszip_F64 sub_max_x,laszip_F64 sub_max_y,laszip_F64 sub_max_z)
{
    int Flag_Intersection;
    double MaxMin[6];
    int n;
    MaxMin[0]=MaxMin[1]=MaxMin[2]=-100000000;//MAX x,y,z
    MaxMin[3]=MaxMin[4]=MaxMin[5]=10000000000;//MIN x,y,z
//FIND MAX AND MIN BUFFER
    for(n=0; n<countt; n++){
    if (pm[n].x > MaxMin[0]) { MaxMin[0]=pm[n].x;}
    if (pm[n].y > MaxMin[1]) { MaxMin[1]=pm[n].y;}
    if (pm[n].z > MaxMin[2]) { MaxMin[2]=pm[n].z;}
    if (pm[n].x < MaxMin[3]) { MaxMin[3]=pm[n].x;}
    if (pm[n].y < MaxMin[4]) { MaxMin[4]=pm[n].y;}
    if (pm[n].z < MaxMin[5]) { MaxMin[5]=pm[n].z;}
    }
//CHECK CASES WITH NO INTERSECTION
    if(MaxMin[0]<sub_min_x
       ||MaxMin[3]>sub_max_x
       ||MaxMin[1]<sub_min_y
       ||MaxMin[4]>sub_max_y
       ||MaxMin[2]<sub_min_z
       ||MaxMin[5]>sub_max_z
       ){
       Flag_Intersection=0;
       }
    else{Flag_Intersection=1;}

    return Flag_Intersection;
}

/* Number of points to fetch per call; adjust to your preference. */
#define N_TARG 1024

/*
==============================================================================================
                                       MAIN FUNCTIONS
==============================================================================================
*/

int main(int argc, char* argv[])
{


//-----------------------INIT VARS DEFINITIONS-----------------------
double start_time = 0.0;
char* file_name_out = 0;

//RIVLIB Variables
point3dstream_handle h3ds = 0; // This is the handle to the data stream (returned from open) */
int sync_to_pps = 0; // Set to 1 if reading data with gps sync information
scanifc_uint32_t countt;//Number of points in the buffer
scanifc_uint32_t n;
scanifc_uint32_t CropBuffer[3];
int end_of_frame;
int counter, counter_len;
counter=counter_len=0;
//These are the arrays for the RIVLIB buffer
scanifc_xyz32       rg_xyz32[N_TARG];       // The x,y,z coordinates*/
scanifc_attributes  rg_attributes[N_TARG]; // The amplitude and quality attributes */
scanifc_time_ns     rg_time[N_TARG];      // The timestamp (internal or gps) */

//LASZIP Variables
laszip_U32 i;
laszip_I64 p_count = 0;
scanifc_uint16_t major;
scanifc_uint16_t minor;
scanifc_uint16_t build;

//These are auxiliary Vars./arrays to build the LAS data (extra bytes)
laszip_F64 coordinates[3];
float range=0.0;
float theta=0.0;
float phi=0.0;

//VARS - BOUNDARY BOX DEFINITION
laszip_F64 sub_min_x;
laszip_F64 sub_min_y;
laszip_F64 sub_min_z;
laszip_F64 sub_max_x;
laszip_F64 sub_max_y;
laszip_F64 sub_max_z;
laszip_F64 sub_min_theta;
laszip_F64 sub_min_phi;
laszip_F64 sub_max_theta;
laszip_F64 sub_max_phi;
laszip_F64 sub_min_range;
laszip_F64 sub_max_range;

//RIVLIB BUFFER CHECK MAX/MIN AND INTERSECTION WITH BOUNDARY BOX
int Flag_Intersection;// TRUE IF FLAG ==1


//---------------------------------------------------------
//-----------------------LOADING DLL-----------------------
// load LASzip VIA DLL
if (laszip_load_dll())
   {
        fprintf(stderr,"DLL ERROR: loading LASzip DLL\n");
        byebye(true, argc==1);
   }

// get version of LASzip DLL
laszip_U8 version_major;
laszip_U8 version_minor;
laszip_U16 version_revision;
laszip_U32 version_build;

if (laszip_get_version(&version_major, &version_minor, &version_revision, &version_build))
  {
    fprintf(stderr,"DLL ERROR: getting LASzip DLL version number\n");
    byebye(true, argc==1);
   }
fprintf(stderr,"LASzip DLL v%d.%d r%d (build %d)\n", (int)version_major, (int)version_minor, (int)version_revision, (int)version_build);

// get version of Riegl DLL
if(scanifc_get_library_version(&major, &minor, &build)){
 fprintf(stderr,"DLL ERROR: getting LASzip DLL version number\n");
 return 1;
}
fprintf(stderr,"Riegl DLL v%d.%d\n", (int)major, (int)minor);

// ------------------------USER INPUT FILES------------------------------
if (argc == 1) {
    char file_name_in[256];
    char file_nameout[256];
    fprintf(stderr,"Running via command line:%s\n", argv[0]);
    fprintf(stderr,"enter input file: " ); fgets(file_name_in, 256, stdin);
    file_name_in[strlen(file_name_in)-1] = '\0';
    argv[1]=file_name_in;
    fprintf(stderr, "FILE >> %s LOADED >>\n", argv[1]);
    fprintf(stderr,"enter output file: "); fgets(file_nameout, 256, stdin);
    file_nameout[strlen(file_nameout)-1] = '\0';
    argv[2]=file_nameout;
    fprintf(stderr, ">>FILE %s OPENED>>\n", argv[2]);
    file_name_out = LASCopyString(argv[2]);
    //argc=3;
}

//DEFINING CLIPPING
cout << "Please enter the min values x y z and max values x y z for the bounding box (optional - press enter to convert the complete file):" << endl;
if (std::cin.peek() == '\n'|| std::cin.peek() == ' ') // pressed enter after string?
    {
        sub_min_x=-10000000000;
        sub_min_y=-10000000000;
        sub_min_z=-10000000000;
        sub_max_x= 10000000000;
        sub_max_y= 10000000000;
        sub_max_z= 10000000000;
    }
else
    {
        std::cin >> sub_min_x>> sub_min_y>> sub_min_z >> sub_max_x>> sub_max_y>> sub_max_z; // will not block if we have char in buffer

    }

cout << "Please enter the min values THETA PHI (DEGREES) RANGE (M) and max values THETA PHI RANGE for the bounding box (optional - press enter to convert the complete file):"<< endl;
cin.ignore();
if (std::cin.peek() == '\n'|| std::cin.peek() == ' ') // pressed enter after string?
    {
        sub_min_theta= -10000000000;
        sub_min_phi=   -10000000000;
        sub_min_range= -10000000000;
        sub_max_theta=  10000000000;
        sub_max_phi=    10000000000;
        sub_max_range=  10000000000;

    }


else
    {
        std::cin >> sub_min_theta>> sub_min_phi>> sub_min_range>>sub_max_theta >> sub_max_phi >>sub_max_range; // will not block if we have char in buffer

    }

//END OF BUFFER DEFINITION*/
//END OF USER INPUT

start_time = taketime();
if (scanifc_point3dstream_open(argv[1], sync_to_pps, &h3ds)) {
    fprintf(stderr, "Cannot open: %s\n", argv[1]);
    print_last_error();
    return 1;
    }

if (scanifc_point3dstream_add_demultiplexer(
    h3ds
    , "hk.txt", 0, "status protocol")) {
    print_last_error();
    return 1;
    }

// ------------------CREATING WRITER VIA DLL-------------------
laszip_POINTER laszip_writer;
if (laszip_create(&laszip_writer))
    {
        fprintf(stderr,"DLL ERROR: creating laszip writer\n");
        byebye(true, argc==1);
    }

//HERE WE HAVE TWO WRITER OPTIONS
//OPTION 1 - SAVE FORMAT 1.2 + COMPATIBILITY MODE
//OPTION 2 - SAVE NATIVE FORMAT

// OPTION 1 : enable the compatibility mode to 1.4
/*   laszip_BOOL request = 1;
   if (laszip_request_compatibility_mode(laszip_writer, request))
    {
        fprintf(stderr,"DLL ERROR: enabling laszip LAS 1.4 compatibility mode\n");
        byebye(true, argc==1, laszip_writer);
    }*/

// OPTION 2 :enable to save the version in 1.4 format
laszip_BOOL request_native = 1;
if (laszip_request_native_extension(laszip_writer, request_native))
   {
      fprintf(stderr,"DLL ERROR: requesting native LAS 1.4 extension for the writer\n");
      byebye(true, argc==1, laszip_writer);
   }

// ----------------------------------HEADER-------------------------------

// get a pointer to the header of the writer so we can populate it
laszip_header* header;
if (laszip_get_header_pointer(laszip_writer, &header))
    {
        fprintf(stderr,"DLL ERROR: getting header pointer from laszip writer\n");
        byebye(true, argc==1, laszip_writer);
    }

//GET TIME OF CREATION
time_t t = time(NULL);
tm* timePtr = localtime(&t);

//HEADER DEFINE
header->file_source_ID = 4711;
header->global_encoding = (1<<0) | (1<<4);// see LAS specification for details
//header->global_encoding = 0x0001; // time stamps are in adjusted standard GPS time
header->version_major=1;
header->version_minor=4;
strncpy(header->system_identifier, "rxp2laz", 32);
strncpy(header->generating_software, "LASZIP&RIEGLdll", 32);
header->file_creation_day=timePtr->tm_mday;
header->file_creation_year=timePtr->tm_year+1900;
header->header_size = 375; // must be 375 for LAS 1.4
header->offset_to_point_data = 375;
header->point_data_format = 1;//Version 1.4(V1)to use all extra bytes
//header->point_data_format = 6;//Version 1.4(0 to 10), but 6 to 10  are preferred formats
header->number_of_point_records = 0;// legacy 32-bit counters should be zero for new point types > 5
header->x_scale_factor = 0.0001;//X,Y,Z recorded on mm level
header->y_scale_factor = 0.0001;
header->z_scale_factor = 0.0001;
//header->x_offset = 0.0;
//header->y_offset = 0.0;
//header->z_offset = 0.0;

// SET EXTRA BYTES: 30 + using short format(2 extra bytes per info)
//header->point_data_record_length = 30 + 18;//TO BE USED WITH FORMAT 6.0
header->point_data_record_length = 28 + 18;//reflectance, range, deviation, theta, phi #TO BE USED WITH FORMAT 1.0
header->start_of_waveform_data_packet_record = 0;
header->number_of_extended_variable_length_records = 0;
header->start_of_first_extended_variable_length_record = 0;
//SET MAX AND MIN VALUES OF X,Y,Z
//header->max_x =;
//header->min_x =;
//header->max_y =;
//header->min_y =;
//header->max_z =;
//header->min_z =;

// array of the total point records per return.
for (i = 0; i < 5; i++)//standard - 0 to 5
    {
      header->number_of_points_by_return[i] = 0;
    }
//SET record return number - extension available at 1.4 format - MAX 15
header->extended_number_of_point_records = 0;// a-priori unknown number of points
for (i = 0; i < 15; i++)
    {
      header->extended_number_of_points_by_return[i] = 0;
    }

// If the offset is not set in the header -->use the bounding box and the scale factor to create a offset (optional)
if (laszip_auto_offset(laszip_writer))
    {
        fprintf(stderr,"DLL ERROR: during automatic offset creation\n");
        byebye(true, argc==1, laszip_writer);
    }

// add description for the  attributes in the  "extra bytes": reflectance, deviation and range
if (laszip_add_attribute(laszip_writer, 8, "Reflectance", "Reflectance of a point", 1.00, 0.0))
    {
        fprintf(stderr,"DLL ERROR: adding 'Reflectance' attribute\n");
        byebye(true, argc==1, laszip_writer);
    }

if (laszip_add_attribute(laszip_writer, 3, "Deviation", "Deviation of a point", 1.00, 0.0))
    {
        fprintf(stderr,"DLL ERROR: adding 'Deviation' attribute\n");
        byebye(true, argc==1, laszip_writer);
    }

if (laszip_add_attribute(laszip_writer, 8, "Range", "Range of a point", 1.00, 0.0))
    {
    fprintf(stderr,"DLL ERROR: adding 'Range' attribute\n");
    byebye(true, argc==1, laszip_writer);
    }

//add description for the  attributes in the  "extra bytes": theta and phi
if (laszip_add_attribute(laszip_writer, 8, "Theta", "Scan_Angle_Theta", 1.00, 0.0))
    {
    fprintf(stderr,"DLL ERROR: adding 'Theta' attribute\n");
    byebye(true, argc==1, laszip_writer);
    }

if (laszip_add_attribute(laszip_writer, 8, "Phi", "Scan_Angle_Phi", 1.00, 0.0))
    {
    fprintf(stderr,"DLL ERROR: adding 'Phi' attribute\n");
    byebye(true, argc==1, laszip_writer);
    }

//END OF HEADER DEFINITION
// -------------------------------------------------------------

fprintf(stderr,"\n opening rxp format.... \n");
fprintf(stderr,"writing LAS 1.4 points with \"extra bytes\" *with* compatibility to compressed LAZ *and* also uncompressed LAS\n");

/** ------------------------WRITING-----------------------------
// I - OPEN THE WRITER*/
laszip_BOOL compress = (strstr(file_name_out, ".laz") != 0);
if (laszip_open_writer(laszip_writer, file_name_out, compress))
    {
        fprintf(stderr,"DLL ERROR: opening laszip writer for '%s'\n", file_name_out);
        byebye(true, argc==1, laszip_writer);
    }
fprintf(stderr,"writing file '%s' %scompressed\n", file_name_out, (compress ? "" : "un"));


/** II - Define a pointer to the point of the writer that we will populate and write */
laszip_point* point;
if (laszip_get_point_pointer(laszip_writer, &point))
    {
    fprintf(stderr,"DLL ERROR: getting point pointer from laszip writer\n");
    byebye(true, argc==1, laszip_writer);
    }

do {

/**************************************************************************//**
 *III -The read function will fill point data into user supplied buffers.
 * \param h3ds [in] the stream heandle that has been returned from open.
 * \param want [in] the size of the result buffers (count of points).
 * \param pxyz32 [out] pointer to xyz buffer
 * \param pattributes [out] pointer to amplitude and quality buffer
 * \param ptime [out] pointer to timestamp buffer
 * \param got [out] number of points returned (may be smaller than want).
 * \param end_of_frame [out] != 0 if end of frame detected
 * \return 0 for success, !=0 for failure

 pattributes - Amplitude, deviation, reflectance, background radiation, flags
 flags (rg_attributes[n].flags) -->  The flags combine some attributes into a single value for compact storage.
 bit0-1: 0 single echo 1 first echo 2 interior echo 3 last echo (rg_attributes[n].flags & 0x03 to select multi return bits)
 bit2: reserved
 bit3: waveform available
 bit4: 1  pseudo echo with fixed range 0.1m
 bit5: 1  sw calculated target
 bit6: 1  pps not older than 1.5 sec
 bit7: 1  time in pps timeframe (rg_attributes[n].flags & 0x40)>>6)
 bit8-9:  facet or segment number 0 to 3
 bits10-15: reserved
 *****************************************************************************/

    if (scanifc_point3dstream_read(
        h3ds
        , N_TARG
        , rg_xyz32 //3d COORDINATES
        , rg_attributes //Amplitude, deviation, reflectance, background radiation, flags
        , rg_time
        , &countt
        , &end_of_frame
    )) {
    fprintf(stderr, "Error reading %s\n", argv[1]);
    return 1;
    }

//Checking intersection between buffer and bounding box
    Flag_Intersection=Check_Intersection(rg_xyz32, countt, sub_min_x, sub_min_y, sub_min_z, sub_max_x,sub_max_y,sub_max_z);
    if (Flag_Intersection==1)
{

/**-------------------------------------------------------------
 V - populating points*/

   for (n=0; n<countt; ++n) {

// Checking points from buffer inside of the bounding box
   if  ( rg_xyz32[n].x <= sub_max_x
       &&rg_xyz32[n].x >= sub_min_x
       &&rg_xyz32[n].y <= sub_max_y
       &&rg_xyz32[n].y >= sub_min_y
       &&rg_xyz32[n].z <= sub_max_z
       &&rg_xyz32[n].z >= sub_min_z
       )

    {

// Range = Distance
        range=sqrt((rg_xyz32[n].x*rg_xyz32[n].x)+(rg_xyz32[n].y*rg_xyz32[n].y)+(rg_xyz32[n].z*rg_xyz32[n].z));
// Polar coordinates Phi(Horizontal) and Theta(Vertical)
        theta=acos(rg_xyz32[n].z/range);
        phi=atan2(rg_xyz32[n].y, rg_xyz32[n].x); //scanning right to left
        phi=((phi < 0.0) ? (phi + 2.0 * M_PI) : phi);

//sub_min_theta>> sub_min_phi>> sub_max_theta >> sub_max_phi
if (phi > (sub_min_phi*M_PI/180) && phi < (sub_max_phi*M_PI/180) && theta > (sub_min_theta*M_PI/180) && theta < (sub_max_theta*M_PI/180) && (range > sub_min_range) && (range < sub_max_range)){

//Saving X,Y,Z coordinates
        coordinates[0] = rg_xyz32[n].x;
        coordinates[1] = rg_xyz32[n].y;
        coordinates[2] = rg_xyz32[n].z;

// Calculating Row and line
        //RC=RowColumn(theta, phi, thetaStart, phiStart, Angle_Step);

        if (laszip_set_coordinates(laszip_writer, coordinates))
        {
                fprintf(stderr,"DLL ERROR: setting coordinates for point %I64d\n", p_count);
                byebye(true, argc==1, laszip_writer);
        }

//saving standard parameters
//intensity, return number, number of returns, classification, scan angle, scanner channel, gps_time
        point->intensity = rg_attributes[n].amplitude;
        point->gps_time = rg_time[n];
        //point->classification = 5; // it must be set because it "fits" in 5 bits
        //point->edge_of_flight_line=;
        //point->rgb= ;
        //point->user_data= ;
        //point->scan_angle_rank= ;
        //point->scan_direction_flag= ;

// saving extended parameters (V 1.4)
//extended_number_of_returns and extended_return_number
        //SINGLE RETURN
        if  ((rg_attributes[n].flags & 0x03) == 0){
            point->number_of_returns =1;
            point->return_number = 1;
            //point->extended_number_of_returns =1; //FOR FORMAT > 5 USE EXTENDED...
            //point->extended_return_number = 1;
            counter=0;
            counter_len=0;
        }

        //MULTIPLE RETURNS(1,2,3)
        else{
                if ((rg_attributes[n].flags & 0x03) == 1){
                    counter=0;
                    counter_len=0;
                    int it=n+1;
                    while ((rg_attributes[it].flags & 0x03)>1){
                        counter=counter+1;
                        it=it+1;
                        }
                   if(counter>5){ counter=4;}
                   point->number_of_returns=counter+1;
                   point->return_number=1;
                   //point->extended_number_of_returns=counter+1;
                   //point->extended_return_number=1;

                   it=counter=0;
                   counter_len=1;
                 }


                if ((rg_attributes[n].flags & 0x03) == 2){
                    counter=counter_len;
                    int it=n;

                    while ((rg_attributes[it].flags & 0x03)==2){
                        counter=counter+1;
                        it=it+1;
                        }
                    if(counter>5){counter=3;}
                    if(counter_len>5){ counter_len=3;}
                    point->number_of_returns=counter+1;
                    point->return_number=counter_len+1;
                    //point->extended_number_of_returns=counter+1;
                    //point->extended_return_number=counter_len+1;

                it=counter=0;
                counter_len=counter_len+1;
                }

                if ((rg_attributes[n].flags & 0x03) == 3){
                point->number_of_returns=counter_len+1;
                point->return_number=counter_len+1;
                //point->extended_number_of_returns=counter_len+1;
                //point->extended_return_number=counter_len+1;
                counter=0;
                counter_len=0;
                }

         }

        //point->extended_classification = 5; //3 to 5 vegetation
        //point->extended_scan_angle =;
        //point->extended_scanner_channel = ;
        //point->extended_classification_flags= ;
        //point->extended_point_type= ;

// set attribute 'reflectance', 'Deviation' and 'Range', 'Theta' and 'Phi'
        *((laszip_F32*)(point->extra_bytes+0))=rg_attributes[n].reflectance; //reflectance of a point
        *((laszip_I16*)(point->extra_bytes+4))=rg_attributes[n].deviation+1; //deviation - shape deviation of point
        *((laszip_F32*)(point->extra_bytes+6))=(laszip_F32)range;//distance computed from coordinates - range of point
        *((laszip_F32*)(point->extra_bytes+10))=(laszip_F32)theta*(180/M_PI);//theta angle computed - Vertical resolution
        *((laszip_F32*)(point->extra_bytes+14))=(laszip_F32)phi*(180/M_PI);//phi angle computed - Horizontal Resolution


        if (laszip_write_point(laszip_writer))
        {
            fprintf(stderr,"DLL ERROR: writing point %I64d\n", p_count);
            byebye(true, argc==1, laszip_writer);
        }

        if (laszip_update_inventory(laszip_writer))
        {
            fprintf(stderr,"DLL ERROR: setting header pointer from laszip writer\n");
            byebye(true, argc==1, laszip_writer);
        }

        p_count++;


          } //END IF POINT INSIDE BOUNDARY THETA PHI CHECK

        } //END IF POINT INSIDE BOUNDARY XYZ CHECK

      } //END IF BUFFER AND BOUNDARY INTERSECTION

    } //END FOR

  } while(countt >0); /* END DO: count will be zero at the end of available data */


// ----------------------------------------------------------------------
// get the number of points written so far
if (laszip_get_point_count(laszip_writer, &p_count))
    {
        fprintf(stderr,"DLL ERROR: getting point count\n");
        byebye(true, argc==1, laszip_writer);
    }
fprintf(stderr,"successfully written %I64d points\n", p_count);


 /* CLOSING LASZIP AND RIEGL , to prevent a memory leak. */
// close the writer
if (laszip_close_writer(laszip_writer))
    {
        fprintf(stderr,"DLL ERROR: closing laszip writer\n");
        byebye(true, argc==1, laszip_writer);
    }
// destroy the writer
if (laszip_destroy(laszip_writer))
    {
        fprintf(stderr,"DLL ERROR: destroying laszip writer\n");
        byebye(true, argc==1);
    }
fprintf(stderr,"total time: %g sec for writing %scompressed\n", taketime()-start_time, (compress ? "" : "un"));
// unload LASzip DLL
if (laszip_unload_dll())
    {
        fprintf(stderr,"DLL ERROR: unloading LASzip DLL\n");
        byebye(true, argc==1);
    }
// close the scanifc
if (scanifc_point3dstream_close(h3ds)) {
       fprintf(stderr, "Error, closing %s\n", argv[1]);
       return 1;
    }
    return 0;

fprintf(stderr, "Usage: %s <input-uri>\n", argv[0]);
return 1;

} // END MAIN CODE

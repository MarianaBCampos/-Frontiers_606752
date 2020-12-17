/*
===============================================================================
FILE: main.h 20.11.2020 v.01
CONTENTS:
This source code generate a dynamic library with 3 functions to:
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


#ifndef __MAIN_H__
#define __MAIN_H__

#include <windows.h>

/*  To use this exported function of dll, include this header
 *  in your project.
 */

#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT __declspec(dllimport)
#endif



#ifdef __cplusplus
extern "C"
{
#endif

int main();

int Read(char* name, double*&x, double*&y, double*&z, int *&return_number,  int *&number_of_returns, float *&intensity, float**&pm, char** &name_attribute, int*&NEB);

int Write(char* name, double*x, double*y, double*z, int*return_number,  int*number_of_returns, float*intensity, float* classification, const int *p_count, int* NEB,  char** name_attribute,  float**pm_att);
//int Write(char* name, double*x, double*y, double*z, int*return_number,  int*number_of_returns, float*intensity, float* classification, const int *p_count, int* NEB,  char** name_attribute,  short**pm_att);

void freeme(double* x, double* y, double* z, int* return_number,  int* number_of_returns, float* intensity, float* classification, float** pm , char**  name_attribute, int* NEB);

#ifdef __cplusplus
}
#endif

#endif // __MAIN_H__

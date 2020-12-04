# -*- coding: utf-8 -*-
"""
PROGRAMMERS:

MARIANA BATISTA CAMPOS - mariana.campos@nls.fi
RESEARCHER AT FINNISH GEOSPATIAL INSTITUTE - NATIONAL LAND OF SURVEY

THIS SCRIPT IS AN EXAMPLE OF MAIN FUNCTION TO USE READLAZ.PY AND WRITELAZ.PY FUNCTIONS

**USER SETUP**

1. UPDATE THE PATHS(Lines 48-50): 
                    DLLPATH=r"path_to_dll_file\Name_dll.dll"
                    inputLas=r"path_to_input_laz_folder\file.laz"
                    outputLas=r"path_to_output_laz_folder\file.laz"
                    
2. ATTENTION: SETUP THE WRITING ACCORDING TO LAZ FILE ATTRIBUTES


COPYRIGHT UNDER MIT LICENSE

Copyright (C) 2019-2020, NATIONAL LAND OF SURVEY

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

"""
import ReadLaz
import WriteLaz
import numpy as np

#!!!CHANGE THE PATHS HERE!!!
DLLPATH=r"Las2Array_StaticLibrary.dll"
inputLas='SampleData.laz'
outputLas='TESTE.laz'


"""
USAGE READING:
INPUT:PATH TO LAZ FILE (inputLas) AND PATH TO NP2LAZ DLL (DLLPATH)
OUTPUT:CLASS WITH LAS CONTENT (x, y, z, return_number, number_of_returns, intensity, extra_bytes)    
"""
LAZCONTENT=ReadLaz.ReadLaz(inputLas, DLLPATH)
#%%


"""
USAGE WRITING:
INPUT:  PATH TO LAZ FILE (inputLas)
        PATH TO NP2LAZ DLL (DLLPATH)
        CLASS WITH MAIN LAZ CONTENT (x, y, z, return_number, number_of_returns, intensity, extra_bytes_name)
        NUMPY WITH EXTRA BYTES VALUES
OUTPUT:A CLASS WITH LAS CONTENT    
"""

#EXAMPLE INCLUDING CLASSIFICATION
classification=np.zeros(len(LAZCONTENT.x), dtype=np.float32)


class MAINCONTENT:
        
    pass
        
setattr (MAINCONTENT, 'x', LAZCONTENT.x*1000) #SETUP HERE ACCORDING TO THE COORDINATES SCALE (0.001)       
setattr (MAINCONTENT, 'y', LAZCONTENT.y*1000)  
setattr (MAINCONTENT, 'z', LAZCONTENT.z*1000) 
setattr (MAINCONTENT, 'return_number', LAZCONTENT.return_number ) 
setattr (MAINCONTENT, 'number_of_returns', LAZCONTENT.number_of_returns ) 
setattr (MAINCONTENT, 'intensity', LAZCONTENT.intensity ) 
setattr (MAINCONTENT, 'classification', classification )
setattr (MAINCONTENT, 'ExtraBytes_name',  LAZCONTENT.ExtraBytes_name) 


#DEFINING EXTRA BYTES
NEB=7; #SET THE NUMBER OF EXTRABYTES
NP=len(LAZCONTENT.x) #SET THE NUMBER OF POINTES

#HERE THE EXTRA PARAMETERS WILL BE SAVE AS FLOAT. Please modify MAIN.h and Main.cpp to use other data formats such as short
ExtraBytes=np.empty((NEB,NP), dtype=np.float32); 

#ASSIGNING EXTRA PARAMETERS
ExtraBytes[0]=(LAZCONTENT.Reflectance)
ExtraBytes[1]=(LAZCONTENT.Deviation)
ExtraBytes[2]=(LAZCONTENT.Range)
ExtraBytes[3]=(LAZCONTENT.Theta)
ExtraBytes[4]=(LAZCONTENT.Phi)
ExtraBytes[5]=(LAZCONTENT.Row)
ExtraBytes[6]=(LAZCONTENT.Column)

#CALLING WRITE FUNCTIONS
WriteLaz.WriteLaz (outputLas, DLLPATH, MAINCONTENT, ExtraBytes)

#TO SAVE THE FILE WITHOUT EXTRA PARAMETERS:
#LAZCONTENT.ExtraBytes_name=[]
#WriteLaz.WriteLaz (outputLas, DLLPATH, MAINCONTENT)
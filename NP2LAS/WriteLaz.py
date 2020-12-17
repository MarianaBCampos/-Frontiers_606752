"""
PROGRAMMERS:

MARIANA BATISTA CAMPOS - mariana.campos@nls.fi
RESEARCHER AT FINNISH GEOSPATIAL INSTITUTE - NATIONAL LAND SURVEY OF FINLAND

THIS SCRIPT IS AN PYTHON FUNCTION WHICH CALLS THE WRITE FUNCTION FROM THE DLL USING CTYPES

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

"""
from ctypes import *
import numpy as np
import time
from numpy.ctypeslib import ndpointer
import gc

def WriteLaz (outputLas, DLLPATH, ATTRIBUTES, Value=[]):

    t1_start = time.perf_counter()
    t2_start = time.process_time()

    #USER INPUT INFORMATION
    FILE_NAME=outputLas
    EXTRABYTES=ATTRIBUTES.ExtraBytes_name
    NEB=len (ATTRIBUTES.ExtraBytes_name);
    size=len(ATTRIBUTES.x)
    
    #LOADING C FUNCTION TO READ WITH LASZIP
    libCalc=CDLL(DLLPATH)
    DATAWRITE=libCalc.Write
    
    #SENDING NUMPY ARRAYS TO C VIA CTYPE
    array_2d_double=ndpointer(dtype=np.uintp, ndim=1, flags='C') 

    
    DATAWRITE.argtypes = [(c_char_p),
                 ndpointer(c_double),ndpointer(c_double), ndpointer(c_double), #X,Y,Z
                 ndpointer(c_int), ndpointer(c_int), #RETURN NUMBER AND NUMBER OF RETURN
                 ndpointer(c_float), ndpointer(c_float),#intensity and classification
                 c_int, c_int,
                 POINTER(c_char_p), array_2d_double] 
    
    DATAWRITE.restype = c_int
    string=(c_char_p)(FILE_NAME.encode('utf-8'))
    x=np.array(ATTRIBUTES.x, dtype=np.float64); 
    y=np.array(ATTRIBUTES.y, dtype=np.float64); 
    z=np.array(ATTRIBUTES.z, dtype=np.float64); 
    return_number=np.array(ATTRIBUTES.return_number); 
    number_of_returns=np.array(ATTRIBUTES.number_of_returns); 
    intensity=np.array(ATTRIBUTES.intensity); 
    classification=np.array(ATTRIBUTES.classification, dtype=np.float32);
    #EXTRA PARAMETERS INFO
    strArrayType=(c_char_p * 20)
    strArray= strArrayType()
    
    if (NEB>0):
        for i in range (NEB):
            strArray[i] =  (EXTRABYTES[i].encode('utf-8'))
        Value_pp= np.array(Value.ctypes.data + np.arange(Value.shape[0]) * Value.strides[0]).astype(np.uintp)
        print(len(Value_pp))
        print((Value_pp.shape))
    else:Value_pp=np.array(Value, dtype=np.uintp);
    
    #CALLING FUNCTION
    npoints=DATAWRITE(string,
                 x, y , z, 
                 return_number, number_of_returns,
                 intensity, classification,
                 size, NEB,
                 strArray, Value_pp)
    
    t1_stop = time.perf_counter()
    t2_stop = time.process_time()

    print("\n Writing Elapsed time: %.1f [sec]" % ((t1_stop-t1_start)))
    print("CPU process time: %.1f [sec] \n" % ((t2_stop-t2_start)))
    Default_atributes=['x', 'y', 'z','return_number','number_of_returns', 'intensity']
    print ("LAZ CONTENT:", Default_atributes)   
    print ("LAZ EXTRA BYTES:", EXTRABYTES) 
    
    del x, y, z, return_number, number_of_returns, intensity, classification, size, NEB, strArray, Value, Value_pp
    gc.collect()
    

"""
PROGRAMMERS:

MARIANA BATISTA CAMPOS - mariana.campos@nls.fi
RESEARCHER AT FINNISH GEOSPATIAL INSTITUTE - NATIONAL LAND SURVEY OF FINLAND

THIS SCRIPT IS AN PYTHON FUNCTION WHICH CALLS READ FUNCTION FROM THE DLL USING CTYPES

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
import gc

def ReadLaz (inputLas, DLLPATH):

    t1_start = time.perf_counter()
    t2_start = time.process_time()

    #INSERT FILE TO BE READ
    FILE_NAME=inputLas
    ExtraBytes_name=[]; x_arr=[];  y_arr=[]; z_arr=[];
    return_number_arr=[]; number_of_returns_arr=[]; intensity_arr=[]; number_attributes=[];
    x_arr_F=[];  y_arr_F=[]; z_arr_F=[];
    return_number_arr_F=[]; number_of_returns_arr_F=[]; intensity_arr_F=[]; number_attributes_F=[];
    
    #LOADING C FUNCTION TO READ WITH LASZIP
    libCalc=CDLL(DLLPATH)
    DATA=libCalc.Read
    FREEMEMO=libCalc.freeme

    DATA.argtypes = [(c_char_p),
                 POINTER(POINTER(c_double)), POINTER(POINTER(c_double)), POINTER(POINTER(c_double)), #X,Y,Z
                 POINTER(POINTER(c_int)), POINTER(POINTER(c_int)), #RETURN NUMBER AND NUMBER OF RETURN
                 POINTER(POINTER(c_float)), #INTESITY
                 POINTER(POINTER(POINTER(c_float))), # POINTER TO EXTRA BYTES
                 POINTER(POINTER((c_char_p))), #EXTRA BYTE NAME
                 POINTER(c_int)] #NUMBER OF POINTS

    DATA.restype = c_int
    string= (c_char_p)(FILE_NAME.encode('utf-8'))
    x=POINTER(c_double)()
    y=POINTER(c_double)()
    z=POINTER(c_double)()
    return_number=POINTER(c_int)() 
    number_of_returns=POINTER(c_int)()
    intensity=POINTER(c_float)()
    pm=POINTER(POINTER(c_float))()
    attributes_names=POINTER(c_char_p)()
    extra=(c_int)()
    
    #CALLING FUNCTION
    npoints=DATA(string,
                 byref(x),byref(y), byref(z), 
                 byref(return_number), byref(number_of_returns),
                 byref(intensity), byref(pm), byref(attributes_names), extra)

    x_arr=np.ctypeslib.as_array((c_double*npoints).from_address(addressof(x.contents)))
    y_arr=np.ctypeslib.as_array((c_double*npoints).from_address(addressof(y.contents)))
    z_arr=np.ctypeslib.as_array((c_double*npoints).from_address(addressof(z.contents)))
    return_number_arr=np.ctypeslib.as_array((c_int*npoints).from_address(addressof(return_number.contents)))
    number_of_returns_arr=np.ctypeslib.as_array((c_int*npoints).from_address(addressof(number_of_returns.contents)))
    intensity_arr=np.ctypeslib.as_array((c_float*npoints).from_address(addressof(intensity.contents)))
    number_attributes=extra.value
    x_arr_F=np.array(x_arr); y_arr_F=np.array(y_arr); z_arr_F=np.array(z_arr);
    return_number_arr_F=np.array(return_number_arr); number_of_returns_arr_F=np.array(number_of_returns_arr); intensity_arr_F=np.array(intensity_arr);
   
    
    class ATRIBUTES:
        
        pass
        
    setattr (ATRIBUTES, 'x', x_arr_F )        
    setattr (ATRIBUTES, 'y', y_arr_F )  
    setattr (ATRIBUTES, 'z', z_arr_F ) 
    setattr (ATRIBUTES, 'return_number', return_number_arr_F ) 
    setattr (ATRIBUTES, 'number_of_returns', number_of_returns_arr_F ) 
    setattr (ATRIBUTES, 'intensity', intensity_arr_F ) 
    
    default=[]; default_F=[]; 
    if number_attributes > 0:
       for i in range (number_attributes):
           ExtraBytes_name.append(attributes_names[i].decode('utf-8'))
           default=np.ctypeslib.as_array((c_float*npoints).from_address(addressof(pm[i].contents)))
           default_F=np.array(default); 
           setattr (ATRIBUTES, ExtraBytes_name[i], default_F )
           del default, default_F
       setattr (ATRIBUTES, 'ExtraBytes_name', ExtraBytes_name)    
                   
    
    t1_stop = time.perf_counter()
    t2_stop = time.process_time()

    print("\n Reading Elapsed time: %.1f [sec]" % ((t1_stop-t1_start)))
    print("CPU process time: %.1f [sec] \n" % ((t2_stop-t2_start)))
    Default_atributes=['x', 'y', 'z','return_number','number_of_returns', 'intensity']
    print ("LAZ CONTENT:", Default_atributes)   
    print ("LAZ EXTRA BYTES: ExtraBytes_name", ExtraBytes_name) 
    
# FOR HUGE POINT CLOUD CALL FREE MEMORY FUNCTION IF NEED    
#    FREEMEMO(x, y , z, 
#            return_number, number_of_returns,
#            intensity, pm, attributes_names, number_attributes)
    
    gc.collect() 
    del x, y, z, return_number, number_of_returns, intensity, pm, attributes_names, extra
    del x_arr, y_arr, z_arr, return_number_arr, number_of_returns_arr, intensity_arr,  ExtraBytes_name   
    del x_arr_F, y_arr_F, z_arr_F, return_number_arr_F, number_of_returns_arr_F, intensity_arr_F
    del libCalc, DATA, npoints, number_attributes
    
    return ATRIBUTES

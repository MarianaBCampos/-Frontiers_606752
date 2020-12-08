# Frontiers_606752

Supplementary code and documentation for the Frontiers manuscript no. 606752

Summary of contents:

RXP2LAZ: The source code rxp2laz was developed in C/C++ language to converts .rxp Riegl format to LAS format (1.4) or - its compressed, but identical twin - the LAZ format, using RIVLIB and LASzip dynamic libraries (.dll). The source code can be compile at Windows or Linux platform. Instructions to user compilation can be found in the documentation at Gitlab folder. RIVLIB (http://www.riegl.com/index.php?id=224) and LASzip (https://laszip.org/) dynamic libraries are not provided, which must be obtained directly from the owners company with the appropriate copyright.


MATLAS: is a MATLAB MEX code for using LAStools library functionalities to read and write las/.laz files to/from MATLAB structure, which consists of a reader function las2mat and a writer function mat2las. Instructions for compiling matlas_tools in Linux or Windows can be found in the documentation at Gitlab folder. LASlib files are not provided. They can be obtained at https://github.com/LAStools/LAStools/tree/master/LASlib. The point-wise data are read and stored in MATLAB as a structure of vectors, enabling the development of MATLAB script for point cloud data processing, according to the user needs. After processing and point cloud manipulation the MATLAB structure can be exported by the user as las/laz file format (1.4).


NP2LAS: Similar to Matlas_tools, nplas_tools is a source code for using LAStools library functionalities to read and write las/.laz files to/from numpy arrays. The source code was developed in C/C++ language and generates as output a dynamic library (dll). The dll is converted to a reader and writer python functions using the python library ctypes (ReadLaz.py and WriteLaz.py). The source code can be compile at Windows or Linux platform. Instructions to user compilation and settings can be found in the documentation at Gitlab folder. LASlib files are not provided.

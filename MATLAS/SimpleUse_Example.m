%   CONTENTS:
% 
%    Example use of simple use of matlas gateway to LASlib
% 
% PROGRAMMERS:
%
%   paula.litkey@nls.fi, mariana.campos@nls.fi 
%
% COPYRIGHT UNDER MIT LICENSE:
%
%   (c) 2014, Finnish Geodetic Institute
%	(c) 2020, National Land Survey of Finland - Finnish Geospatial Research Institute
%
%	Permission is hereby granted, free of charge, to any person obtaining a copy
%	of this software and associated documentation files (the "Software"), to deal
%	in the Software without restriction, including without limitation the rights
%	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
%	copies of the Software, and to permit persons to whom the Software is
%	furnished to do so, subject to the following conditions:
%
%	The above copyright notice and this permission notice shall be included in all
%	copies or substantial portions of the Software.
%
%	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
%	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
%	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
%	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
%	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
%	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
%	SOFTWARE.
% ---------------------------------------------------------------------------------------


addpath('path_to\OS_Win\Binaries\Win64')

%% READING DATA

tic, [hdr,str] = las2mat('-i PATH\FILENAME.laz'); toc
%tic, [hdr,str] = las2mat('-i PATH\FILENAME.las'); toc


%% SAVING LAZ INFORMATION IN A NEW STRUCTURE - 
% ATTENTION: CHANGE THE NAME OF ATTRIBUTES ACCORDING TO YOUR LAZ FILE

strnew.x = str.x;
strnew.y = str.y;
strnew.z = str.z;
strnew.return_number = str.return_number;
strnew.number_of_returns=str.number_of_returns;
strnew.intensity=str.Amplitude;
strnew.Reflectance=str.Reflectance;
strnew.Deviation=str.Deviation;

% extra attributes
ea_hdr_info(1,1) = struct(...
     'name', 'Reflectance', ...
     'type', 3, ... % Available numbers [1,2,3,4,5,6,9,10]
     'description', 'reflectance', ...
     'scale', 100, ...
     'offset', 0 ...
     );

ea_hdr_info(1,2) = struct(...
     'name', 'Deviation', ...
     'type', 3, ... % Available numbers [1,2,3,4,5,6,9,10]
     'description', 'deviation', ...
     'scale', 100, ...
     'offset', 0 ...
     );
 
%% WRITING LAS/LAZ FILE

strnew.attribute_info = ea_hdr_info;
mat2las(strnew, ['-o ',fullfile('FILANAME.laz')])
%mat2las(strnew, ['-o ',fullfile('FILANAME.laz')])





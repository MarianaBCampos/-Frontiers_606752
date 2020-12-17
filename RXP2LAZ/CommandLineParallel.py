# -*- coding: utf-8 -*-
"""
THIS SCRIPT CALL THE RXP2LAS EXECUTABLE USING A INPUT FILE (CONFIGURATION FILE)

**USER SETUP**

1. UPDATE THE PATHS (LINES: 50 TO 54): 
                    Executable path\Name_executable.exe
                    RXP files folder
                    Output folder for Laz files
                    Output folder to configuration files
                    
2. SETUP THE CONFIGURATION FILE IN LINES 60-70, WHICH WILL BE GENERATED AUTOMATICALLY FOR EACH RXP FILE. 


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
import os, random, sys, subprocess, glob
import time
import numpy as np
from functools import partial
from multiprocessing.dummy import Pool

t1_start = time.perf_counter()
t2_start = time.process_time()

#INPUT LISTS
command=[]; command_it=[]; k=0
#INPUT FILES 
Rxp2LasTool = r"CHANGE_HERE_PATH\TO\EXECUTABLE.exe"
DataDir=r"CHANGE_HERE_PATH\TO\RXPFILEFOLDER"
OutputDir=r"CHANGE_HERE_PATH\TO\LAZFILEOUTPUTFOLDER"
ConfDir=r"CHANGE_HERE_PATH\TO\OUTPUTCONFIGURATIONFILES_IF_BOUDERINGBOX"
DataList=(glob.glob(DataDir+"/*.rxp"))

print('RUNNING:', Rxp2LasTool) 

for i in range (len(DataList)):
    AUX=os.path.basename(DataList[i]).split('.')[0]
    ConfigList=ConfDir+"\\"+AUX+".txt"
    f= open(ConfigList,"w+")
    f.write(DataList[i]+"\n") 
    f.write(OutputDir+"\\"+AUX+".laz"+"\n")
    f.write("\n")
    #f.write("Xmin Ymin Zmin Xmax Ymax Zmax\n") # COMMENT THE PREVIOUS LINE (\n) and include the coordinate box here
    f.write("\n")
    #f.write("Thetamin Phimin Rangemin Thetamax Phimax Rangemax\n") # COMMENT THE PREVIOUS LINE (\n) and include the scan angles and range box here
    f.close()
    command.append ("%s <%s" %(Rxp2LasTool, ConfigList))

count=(len(command))

while count>0:
#DEFINING THE NUMBER OF PARALLEL PROCESS (MAX 4)
    if count>=4:
        pool = Pool(4); 
        command_it.append(command[k]); command_it.append(command[k+1]); command_it.append(command[k+2]); command_it.append(command[k+3])
        k=k+4; count=count-4 # two concurrent commands at a time
        #print("4 cores in use")
    else:    
        if count==3:
            pool = Pool(3); 
            command_it.append(command[k]);command_it.append(command[k+1]);command_it.append(command[k+2])
            k=k+3; count=count-3; # three concurrent commands at a time
            #print("3 cores in use")
        else: 
            if count==2:
                pool = Pool(2); 
                command_it.append(command[k]);command_it.append(command[k+1]);
                k=k+2; count=count-2; # two concurrent commands at a time
                #print("2 cores in use")
            else:    
                if count==1:
                    pool = Pool(1)  # one concurrent commands if the number of files in the folder is impar
                    command_it.append(command[k])
                    count=count-1
                    #print("1 core in use")

#RUNNING PARALLEL PROCESS     
    for i, returncode in enumerate(pool.imap(partial(subprocess.call, shell=True), command_it)):
        if returncode != 0:
           print("%d command failed: %d" % (i, returncode))    
    command_it=[];
    
# close the pool and wait for the work to finish 
pool.close() 
pool.join() 

t1_stop = time.perf_counter()
t2_stop = time.process_time()

print("--------------------------------------------------")
print("Elapsed time: %.1f [sec]" % ((t1_stop-t1_start)))
print("CPU process time: %.1f [sec]" % ((t2_stop-t2_start)))
print("--------------------------------------------------") 


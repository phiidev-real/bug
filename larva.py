import subprocess
import json
import os
from os import listdir

LARVA_MSG = "\x1b[1;31mLARVA: \x1b[0m";

#I'm still really bad at python, but at least it's cross-platform.

#https://stackoverflow.com/questions/5137497/
#I should have used string manip lol

##No tailing / lmaoooo
#dir_root = os.path.dirname( os.path.realpath( __file__ ) ) + "/";
dir_root = "./";
dir_tmp  = dir_root + "tmp/";
dir_obj  = dir_root + "obj/";

macros = [  ];
compile_obj_template        = "g++ -c {finput} {fmacros} -ldl -lglfw -o {foutput}";
compile_standalone_template = "g++ {finput} {fmacros} -ldl -lglfw -o {foutput}";

def GetTimestamp(path):
    return None if not os.path.isfile(path) else os.path.getmtime(path);
    

#Returns None if there's no update needed, else returns the new timestamp
def ShouldUpdate(finput, foutput):
    
    #Get the timestamps
    cpp_ts = GetTimestamp(finput );
    obj_ts = GetTimestamp(foutput);
    
    #If there's no object file, compile anyways
    if obj_ts == None:
        return True;
        
    
    #If there's no cpp file, something has gone horribly wrong. 
    #Check the Scan function. 
    if cpp_ts == None:
        raise Excpetion(file.src + " does not exist (Why is it here?)");
        
    
    #If the object file is older than the cpp file
    #We know the cpp file has been modified after last compiliation
    return cpp_ts > obj_ts;
    

def Check(fin, fout, callback):
    
    
    #Self explanitory
    if ShouldUpdate(fin, fout):
        print(LARVA_MSG + "\"{}\" -> \"{}\"".format(fin, fout));
        callback(fin, fout);
        
    
    #If should update still returns True, we didn't actually do anything. 
    if ShouldUpdate(fin, fout):
        raise OSError("Callback failed. ");
    

class DirectoryPrinter():
    
    def __init__(self, file_count):
        self.array = [ ];
        self.array.append(file_count);
    
    def Push(self, file_count):
        self.array.append(file_count);
        return;
    
    def Print(self, file_name):
        
        out = [ ];
        last_index = len(self.array) - 1;
        for i, num in enumerate(self.array):
            
            if i == last_index:
                if num > 1:
                    out.append(" ├──");
                elif num == 1:
                    out.append(" ╰──");
                else:
                    out.append("    ");
            else:
                if num == 0:
                    out.append("    ");
                else:
                    out.append(" │  ");
                    
                
            
        
        self.array[-1] -= 1;
        while self.array[-1] == 0:
            self.array.pop();
            
            if len(self.array) == 0:
                break;
                
            
        
        print("\t", "".join(out) + file_name);
        
        return;
        
    

#Scans the tmp directory for files to compile. 
def Scan(directory, file_printer = None):
    
    files = [ ];
    files_in_directory = os.listdir(directory);
    cpp_files = list(filter(lambda a : not os.path.isdir(directory + a), files_in_directory));
    dir_files = list(filter(lambda a :     os.path.isdir(directory + a), files_in_directory));
    
    if file_printer == None:
        file_printer = DirectoryPrinter( len(cpp_files) + len(dir_files) );
        print("\t" + directory);
        root_directory = directory;
    else:
        file_printer.Push( len(cpp_files) + len(dir_files) );
        
    
    files_printed = 0;
    files_to_print = len(list(cpp_files));
    
    for file in cpp_files:
        file_printer.Print(file);
        files.append( directory + file );
        
    
    for folder in dir_files:
        folder += "/";
        file_printer.Print(folder);
        
        files.extend(
            Scan(directory + folder, file_printer)
        );
        
    
    return files;
    


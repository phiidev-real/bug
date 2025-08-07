import subprocess
import json
import os
from os import listdir

LARVA_MSG = "\x1b[1;34mLARVA: \x1b[0m";
LARVA_PMT = "\x1b[1;35mLARVA: \x1b[0;35m";
LARVA_CMD = "\x1b[1;33mLARVA: \x1b[0;33mCommand: \x1b[0m";
LARVA_ERR = "\x1b[1;31mLARVA: \x1b[0;31mError: ";

#I'm still really bad at python, but at least it's cross-platform.

#https://stackoverflow.com/questions/5137497/
#I should have used string manip lol

#No tailing / lmaoooo
#dir_root = os.path.dirname( os.path.realpath( __file__ ) ) + "/";
dir_root = "./";
dir_src  = dir_root + "src/";
dir_obj  = dir_root + "obj/";

macros = [  ];
compile_obj_template        = "g++ -c {finput} {fmacros} -ldl -lglfw -o {foutput}";
compile_standalone_template = "g++ {finput} {fmacros} -ldl -lglfw -o {foutput}";

#Just to make things slightly easier. 
class Compileable:
    
    def __init__(self, finput):
        self.finp = finput[:];
        self.full = "src/" +           finput                            ;
        self.name =                    finput   .split("/")[ -1]         ;
        self.ext  = "."    +           finput   .split(".")[ -1]         ;
        self.bald =          ".".join( finput   .split(".")[:-1] )       ;
        self.out  = "obj/" +           self.bald                   + ".o";
        
    

def Compile(finput, foutput = None):
    
    global macros;
    command = compile_obj_template.format(
            finput      = finput,
            fmacros     = " ".join(macros),
            foutput     = foutput);
    
    print(LARVA_CMD + command);
    
    print(LARVA_MSG + "Compiling file \"{}\" to \"{}\"".format(finput, foutput));
    print();
    
    dir_file = os.path.dirname( os.path.realpath( foutput ) );
    subprocess.run( [ "mkdir", "-p", dir_file ] );
    subprocess.run( command.split(" ") );
    
    print();
    

def GetTimestamp(path):
    return None if not os.path.isfile(path) else os.path.getmtime(path);
    

#Returns None if there's no update needed, else returns the new timestamp
def ShouldUpdate(file):
    
    #Get the timestamps
    cpp_ts = GetTimestamp(file.full);
    obj_ts = GetTimestamp(file.out );
    
    #If there's no object file, compile anyways
    if obj_ts == None:
        return True;
        
    
    #If there's no cpp file, something has gone horribly wrong. 
    #Check the Scan function. 
    if cpp_ts == None:
        raise Excpetion(file.full + " does not exist (Why is it here?)");
        
    
    #If the object file is older than the cpp file
    #We know the cpp file has been modified after last compiliation
    return cpp_ts > obj_ts;
    

def Check(file):
    
    print(LARVA_MSG + "Checking file \"{}\"".format(file.full));
    
    #Self explanitory
    if ShouldUpdate(file):
        Compile(file.full, file.out);
    else:
        print("     -> \x1b[32mUp to date. \x1b[0m");
        
    

class DirectoryPrinter():
    
    array = [ ];
    def __init__(self, file_count):
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
        
    

#Scans the src directory for files to compile. 
def Scan(directory = "", file_printer = None, sub_path = "", depth = 1):
    
    files = [ ];
    files_in_directory = os.listdir(dir_src + directory + sub_path);
    cpp_files = list(filter(lambda a : a.endswith(".cpp") or a.endswith(".c"), files_in_directory));
    dir_files = list(filter(lambda a : os.path.isdir(dir_src + directory + a), files_in_directory));
    
    if file_printer == None:
        file_printer = DirectoryPrinter( len(cpp_files) + len(dir_files) );
        print("\tsrc/");
    else:
        file_printer.Push( len(cpp_files) + len(dir_files) );
        
    
    files_printed = 0;
    files_to_print = len(list(cpp_files));
    
    for file in cpp_files:
        file_printer.Print(file);
        files.append( Compileable(directory + file) );
        
    
    for folder in dir_files:
        
        folder += "/";
        file_printer.Print(folder);
        
        files.extend(
            Scan(directory + folder, file_printer)
        );
        
    
    return files;
    

available_versions = [     "gl",     "vk" ];
version_macros     = [     "GL",     "VK" ];
program_names      = [ "opengl", "vulkan" ];

def CompileProject():
    
    while True:
        print(LARVA_PMT + "Enter the verison to compile... \x1b[0m");
        print(available_versions);
        
        verison = "";
        verid   = input();
        if verid.isdigit():
            verison = available_versions[verid];
        elif verid in available_versions:
            version = verid;
            verid = available_versions.index(version);
        else:
            print(LARVA_ERR + "Input not valid. ");
            continue;
            
        
        print();
        break;
        
    
    global macros;
    macros = [ "-D" + version_macros[verid] ];
    
    print(LARVA_MSG + "Checking \"src/\" for files. ");
    print();
    cpp_files = Scan(); 
    print();
    
    #list comprehension go brrr
    #Removes everything past the . for each file.
    for file in cpp_files:
        Check(file);
        

    print();
    print(LARVA_MSG + "Compiling {} version... ".format(version));
    
    #Finally, we can compile the damn thing.

    #First we need to filter out all the vk files. 
    def CheckVersion(output):
        parent_dir = output.split("/")[-2];
        
        #If the parent directory's name is in the available versions array,
        #and it is not the current version, the file belongs to another version and we should quit. 
        return not (parent_dir in available_versions and parent_dir != version);
        
    
    object_files_to_compile = filter(CheckVersion, [ f.out for f in cpp_files ]);
    command = compile_standalone_template.format(
            finput      = " ".join(object_files_to_compile),
            fmacros     = " ".join(macros),
            foutput     = program_names[verid]);
    
    subprocess.run( command.split(" ") );
    
    #If this fails, it still reports a success. idk...
    print("     -> \x1b[32mSuccess! \x1b[0m");
    print();
    print( LARVA_MSG + "Compiled to file \"{}\"".format(program_names[verid]) );
    

CompileProject();


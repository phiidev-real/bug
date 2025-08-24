import larva
import os
import subprocess

#phosis.py scans and copies the files in src/ to tmp/
#Then precompile and send the .c files from tmp/ to obj/

#This just generates some macro stuff
def Macro(text):
    return "PHOSIS_AUTOGEN_" + text;
    

def File(name, path="/"):
    return "../" * (path.count("/") - 2) + "autogen/" + name;
    

xmacros = dict();
hooks = dict();

PHOSIS_MSG = "\x1b[1;34mPHOSIS: \x1b[0m";

debug_template = """
#define {}_ERROR_MESSAGE(message)       std::cout << ::tab_spaces << \"\\x1b[1;31m{}: Error: \\x1b[0;31m\" << message << \"\\x1b[0m\" << std::endl;
#define {}_ERROR_LOG(message, log)      std::cout << ::tab_spaces << \"\\x1b[1;31m{}: Error: \\x1b[0;31m\" << message << std::endl << std::endl\\
						<< \"\\x1b[0;1;31m<log>\\x1b[0;31m\" << std::endl << log << \"\\x1b[1;31m<\\\\log>\\x1b\" << std::endl;

#define {}_MESSAGE(message)	            std::cout << ::tab_spaces << \"\\x1b[1;34m{}: \\x1b[0m\" << message << std::endl;
#define {}_MESSAGE_MISC(message)	    std::cout << ::tab_spaces << \"\\x1b[1;35m{}: \\x1b[0m\" << message << std::endl;

#define {}_SUCCESS(message)	            std::cout << ::tab_spaces << \"\\x1b[1;32m{}: \\x1b[0;32m\" << message << \"\\x1b[0m\" << std::endl << std::endl;

#define {}_SUBSUCCESS(message)		    std::cout << ::tab_spaces << \"      -> \\x1b[0;32m\" << message << \"\\x1b[0m\" << std::endl;
#define {}_SUBFAIL(message)             std::cout << ::tab_spaces << \"      -> \\x1b[0;31m\" << message << \"\\x1b[0m\" << std::endl;

#define {}_SUBMESSAGE(message)	        std::cout << ::tab_spaces << \"      -> \" << message << std::endl;
#define {}_NEWLINE()                    std::cout << ::tab_spaces << std::endl;

namespace {
	
	char tab_spaces[8];
	int current_depth = 0;
	void PushTab() { tab_spaces[current_depth++] = \'\\t\'; {}_NEWLINE(); }
	void PopTab () { tab_spaces[--current_depth] = \'\\0\'; {}_NEWLINE(); }
	
};
""";

def Pseudoparenthesis(string, popen, pclose):
    
    depth = 0;
    substrings = [ ];
    substring = "";
    
    while len(string) > 0:
        
        next_open_char = string.find(popen);
        
        if next_open_char == -1:
            substrings.append(string);
            break;
        else:
            if next_open_char != 0:
                substrings.append(string[:next_open_char]);
                
            
            string = string[next_open_char + 1:];
            depth  = 1;
        
        while depth != 0:
            
            next_close_char = string.find(pclose);
            next_close_char = next_close_char if next_close_char != -1 else len(string);
            
            if next_close_char == -1:
                substring += string;
                break;
                
            else:
                addition   = string[:next_close_char + 1];
                substring += addition;
                string     = string[next_close_char + 1:];
                
                #We are now processing the string JUST AFTER when there is a ")".
                #There is exactly 1 ")", and depth should change by "(" - ")"
                
                depth += addition.count(popen) - 1;
                
            
        
        substrings.append(substring[:-1]);
        substring = "";
        
    
    return substrings;
    

def PseudoSeekPend(string, popen, pclose):
    depth = 0;
    for i, c in enumerate(string):
        if c == popen:
            depth += 1;
        elif c == pclose:
            depth -= 1;
        if depth == 0:
            return i;
        
    

def PreCallback(path_in, path_out):
    
    #Files twitwitwitwitwitwitwi
    fin  = open(path_in  , "r");
    fout = open(fout_path, "w");
    
    file_name = ".".join(path_out.split("/")[-1].split(".")[:-1]);
    file_name_caps = file_name.upper();
    
    for line in fin:
        
        words = line.split(" ");
        words[-1] = words[-1][:-1];
        
        if words[0] == "#larva":
            
            match words[1]:
                case "debug":
                    fout.write(debug_template.replace("{}", file_name_caps));
                case "def-xmac":
                    
                    fout.write("#define " + Macro(words[2]) + "\\\n");
                    
                    #17 is len("#define def-mac ") + len(" ")
                    args = line[18 + len(words[2]) + len(words[3]):];
                    
                    #The split-join thing gets rid of spaces.
                    value_entries = Pseudoparenthesis("".join(args.split(" ")), "[", "]")[0];
                    
                    end_index = PseudoSeekPend(value_entries, "(", ")");
                    print(end_index);
                    
                    array = None;
                    if end_index != 0: #This case only triggers if there are no parenthesis
                        
                        array = [ ];
                        
                        while len(value_entries):
                            
                            array.append( str(value_entries[1:end_index].split(","))[1:-1] );
                            value_entries = value_entries[end_index:];
                            end_index = PseudoSeekPend(value_entries, "(", ")");
                            
                            if end_index == None:
                                break;
                            
                        
                    else:
                        array = value_entries.split(",");
                        
                    
                    #
                    for value in array[:-1]:
                        fout.write("\tX(" + value + ")\\\n");
                        
                    
                    fout.write("\tX(" + array[-1] + ")\n");
                    
                
                case "use-xmac":
                    args = line[16:];
                    
                    #Get the name of the xmacro to be called
                    open_index = args.index("(");
                    name = args[:open_index];
                    
                    #Everything including and after the "(" can just be pasted in.
                    #The -1 is for the line feed
                    fout.write("#define X" + args[open_index:-1] + "\n\t" + Macro(name) + "\n#undef X");
                    
                
                case "impl-hook":
                    fout.write("#include \"{}.cpp\"".format(File(words[2], path_in)));
                    
                
                case "apnd-hook":
                    print(line);
                    hooks.update({ words[2] : [ words[3] ] });
                    
                
                case _:
                    print("fuck");
                    
                
            
            continue;
            
        
        fout.write(line);
        
    

def CompileCallback(path_in, path_out):
    
    #The parent directory, to make sure we don't get an error
    parent_out = "/".join(path_out.split("/")[:-1]);
    
    #Make sure the path for the output file exists
    if not os.path.exists(parent_out):
        os.makedirs(parent_out);
        
    
    #Call the compile function (TODO: make this compatible with different compilers
    subprocess.call([ "g++", "-c", path_in, "-ldl", "-lglfw", "-o", path_out ]);
    

versions = [ "gl", "vk", ];
not_this_version = [ ];

while True:
    print(PHOSIS_MSG + "Select a version...");
    global version;
    version = input("[" + ", ".join(versions) + "] ");
    print();
    if version in versions:
        break;
        
    

for v in versions:
    if v != version:
        not_this_version.append(v);
        
    

print(PHOSIS_MSG + "Scanning \"src/\". ");
print();
src_files = larva.Scan("src/");
print();

print(PHOSIS_MSG + "Starting preprocessing... ");
print();

for file in src_files:
    
    subpath = "/".join(file.split("/")[1:]);
    
    #
    path_out = "tmp/" + subpath;
    
    #The parent directory, to make sure we don't get an error
    path_out_list   = subpath.split("/");
    
    #If this file is from a different version, we don't need to check this file
    if path_out_list[-2] in not_this_version: continue;
    
    #If this is the version, remove that folder
    if path_out_list[-2] ==          version: path_out_list.remove(version);
    
    parent_out = "tmp/" + version + "/" + "/".join(path_out_list[:-1]);
    fout_path = parent_out + "/" + path_out_list[-1]
    
    #Make sure the path for the output file exists
    if not os.path.exists(parent_out):
        os.makedirs(parent_out);
        
    
    larva.Check("src/" + subpath, fout_path, PreCallback);
    

print();
print(PHOSIS_MSG + "Starting compilation... ");
print();
tmp_files = larva.Scan("tmp/");
print();

for file in tmp_files:
    
    #Don't check this one, we don't need to compile non-c files. 
    if not (file.endswith(".cpp") or file.endswith(".c")): continue;
    
    subpath = "/".join(file.split("/")[1:]);
    
    larva.Check("tmp/" + subpath, "obj/" + ".".join(subpath.split(".")[:-1]) + ".o", CompileCallback);
    

print();
print(PHOSIS_MSG + "Finalizing compilation... ");
print();

print(PHOSIS_MSG + "Scanning obj/{}/".format(version));
print();
out_files = larva.Scan("obj/{}/".format(version));
print();

print(PHOSIS_MSG + "Compiling final project... ");
subprocess.call([ "g++", *out_files, "-ldl", "-lglfw", "-o", version ]);
print(PHOSIS_MSG + "Compilation complete ({})!".format(version));


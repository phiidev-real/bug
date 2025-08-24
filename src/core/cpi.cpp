
//Compound Eye. 
//Reads files. 

#include "cpi.hpp"
#include "bug.hpp"

#include <iostream>
#include <climits>
#include <unistd.h>
#include <fstream>
#include <cstdint>
#include <cstring>

#larva debug

namespace {
	
	//At first I was gonna do dynamic data alloc, but I don't care. 
	const int PATH_SIZE = PATH_MAX + 1;
	char path[PATH_SIZE] = { '\0' };
	char* path_head;
	
};

int cpi::Awake()
{
	
	#ifdef __linux__
		
		//Read out the path data and push a \0
		ssize_t length = readlink("/proc/self/exe", path, PATH_MAX);
		
		//Get the last '/' of the file
		//btw, it "ends before the slash"
		//example/path/blah/blah/blah /executable
		//			     ^ before the slash, at least with memcpy
		int path_head_start = 0;
		for (path_head_start = length; ; path_head_start--)
		{
			
			if (::path[path_head_start] == '/') break;
			if (path_head_start         ==  0 ) return 1; //This crashes, because what the fuck
			
		}
		
		//i starts at the /, so we need to clear from i+1 to the entire length of the path string
		for (int j = path_head_start + 1; j < length; j++)
			::path[j] = '\0';
		
		::path_head = ::path + path_head_start + 1; //Add one to push it past the /
		
	#else
		
		#error "Platform not supported. "
		
	#endif
	
	return 0;
	
}

cpi::Omma cpi::Read(const char* filename)
{
	
	CPI_MESSAGE("Opening file \"" << filename << "\". ");
	
	//The string copy yippie
	std::strcpy(::path_head, filename);
	std::ifstream ifs(::path);
	
	if (!ifs)
	{
		
		CPI_ERROR_MESSAGE("Could not open file, crashing and burning... ");
		CPI_NEWLINE();
		
		ifs.close();
		bug::Kill();
		
	}
	
	CPI_MESSAGE("File \"" << filename << "\" opened, creating buffer. ");
	
	//Get the end of the file
	ifs.seekg(0, std::ios_base::end);
	size_t length = ifs.tellg();
	ifs.seekg(0, std::ios_base::beg);
	
	if (length > (4 << 20))
	{
		
		CPI_ERROR_MESSAGE("Could not create buffer; file size exceeds limit (" << length << "B > 4MiB)");
		CPI_NEWLINE();
		
		ifs.close();
		bug::Kill();
		
	}
	
	//Create the buffer
	char* p_buffer = new char[length];
	
	//Read all the data into the buffer
	ifs.read(p_buffer, length);
	ifs.close();
	
	CPI_SUCCESS("Allocated and read " << length << " bytes of data. ");
	
	cpi::Omma omma;
	omma.length = length;
	omma.p_data = p_buffer;
	
	return omma;
	
}


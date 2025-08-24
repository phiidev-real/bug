#include <iostream>
#include <climits>
#include <unistd.h>
#include <fstream>
#include <cstdint>
#include <cstring>

#include "bug.hpp"
#include "cpi.hpp"
#include "exo.hpp"

#define MAX_MODEL_NAME_LENGTH		30

#larva debug

//The way this works is by taking the buffer and reserving some bits for the file name. 
//So something like
// /this/is/the/path/mdl/**********
//And then to read a file we just modify that "end" data
// /this/is/the/path/mdl/file.edat*

namespace {
	
	bool initialized = false;
	bool edianness = false;
	
	char* mdl_path;
	int starting_offset;
	
	//Doing it like this cuz delete[] can do cleanup
	class FileInfo {
		
	public:
		uint	length;
		char*	p_filename;
		uint*	p_content;
		uint**	pp_offsets;
		
		 FileInfo()
		{
			
			p_content = nullptr;
			
		}
		~FileInfo()
		{
			
			delete[] p_filename;
			if (p_content  != nullptr) delete[] p_content;
			if (pp_offsets != nullptr) delete[] pp_offsets;
			
		}
		
		int Init(uint file);
		bool is_initialized() { return initialized; }
		
	private:
		
		bool	initialized = 0;
		
	};
	
	unsigned int file_count;
	FileInfo *p_file_info;
	
	int ReadRoutingFile()
	{
		
		cpi::Omma omma = cpi::Read("mdl/route");
		unsigned char *p_data = (unsigned char*)omma.p_data; //Gotta do this cuz unsigned stuff yippie
		
		//Read the file count and alloc the fileinfos
		file_count = p_data[0];
		
		p_file_info = new FileInfo[file_count];
		
		EXO_MESSAGE("Routing table opened; " << (int)file_count << " mdls to init. ");
		::PushTab();
		
		int total_model_count = 0;
		for (int i = 0, head = 1; i < file_count; i++)
		{
			
			//Read the filename's length
			unsigned char filename_length = p_data[head++];
			
			//Read out the file name
			char* file_name = new char[filename_length];
			strcpy(file_name, (const char*)(p_data + head));
			
			head += filename_length;
			
			EXO_MESSAGE("File \"" << file_name << "\". ");
			p_file_info[i].p_filename = file_name;
			
			//Bits
			unsigned char	small = p_data[head], middle, large;
			
			//Assemble the length
			p_file_info[i].length = p_data[head++] | (p_data[head++] << 8) | (p_data[head++] << 16);
			EXO_SUBMESSAGE(p_file_info[i].length << " byte(s)");
			
			//Assemble the model count
			total_model_count += p_data[head++] | (p_data[head++] << 8);
			
		}
		
		::PopTab();
		
		return 0;
		
	}
	
	char* Read(const char* filename, unsigned int& file_size)
	{
		
		EXO_MESSAGE("Reading file \"" << filename << "\". ");
		EXO_NEWLINE();
		
		cpi::Omma omma = cpi::Read("mdl/test.edat");
		char* p_buffer = omma.p_data;
		
		unsigned int identifier = *((unsigned int*)p_buffer);
		
		//If this is the case, we're in the wrong edian. 
		if (identifier >= 0xFFFF)	
		{
			
			EXO_MESSAGE("Edianness incorrect, fixing... ");
			
			//Iterate over every bit and flip the edianness
			for (int i = 0; i < omma.length; i++)
			{
				
				int tmp = p_buffer[i];
				p_buffer[i] =	((tmp & 0xFF000000) >> 24) |
						((tmp & 0x00FF0000) >>  8) |
						((tmp & 0x0000FF00) <<  8) |
						((tmp & 0x000000FF) << 24);
				
			}
			
			EXO_SUBSUCCESS("Fixed (I hope). ");
			
		}
		else
		{
			
			EXO_SUCCESS("File \"" << filename << "\" has been loaded into memory. ");
			
		}
		
		return p_buffer;
		
	}
	
	
};

uint32_t* exo::Model(int file, int model)
{
	
	EXO_MESSAGE("Reading model <" << file << "-" << model << ">. ");
	FileInfo& fi = p_file_info[file];
	
	if (!fi.is_initialized())
	{
		
		//No error handling, what am I doing???
		fi.Init(file);
		
	}
	
	return fi.pp_offsets[model];
	
}

int ::FileInfo::Init(uint file)
{
	
	uint length; //length is in bytes
	uint* p_content = (uint*)::Read(p_file_info[file].p_filename, length);
	p_file_info[file].p_content = p_content;
	
	//Init the offets buffer
	uint model_count = p_content[0];
	p_file_info[file].pp_offsets = new uint*[model_count];
	uint** pp_offsets = p_file_info[file].pp_offsets;
	
	uint head = 1;
	for (int i = 0; i < model_count; i++)
	{
		
		pp_offsets[i] = p_content + head;
		uint	attrib_bytes	= p_content[head++],
			index_bytes	= p_content[head++];
		
		head += attrib_bytes + index_bytes;
		
	}
	
	return 1;
	
}

int exo::Init()
{
	
	EXO_MESSAGE("Initializing \x1b[1;34mEXO\x1b[0m. ");
	EXO_MESSAGE("Searching for path. ");
	
	#ifdef __linux__
	{
		
		char path[PATH_MAX + 1]; //Allocate a buffer for us to use
		
		//Read out the path data and push a \0
		ssize_t length = readlink("/proc/self/exe", path, PATH_MAX);
		path[length] = '\0'; //Why do we do '\0'? Like why not just 0?
		
		//Get the last '/' of the file
		//btw, it "ends before the slash"
		//example/path/blah/blah/blah /executable
		//			     ^ before the slash, at least with memcpy
		int i = 0;
		for (i = length; ; i--)
		{
			
			if (path[i] == '/') break;
			if (i       ==  0 ) return 1; //This crashes, because what the fuck
			
		}
		
		//The end of the data is going to be i + 5 (length(path) + length("mdl/"))
		::starting_offset = i + 5;
		
		//Using the data, reconstruct the data
		//We add some extra data to make loading easier
		int mdl_path_length = ::starting_offset + MAX_MODEL_NAME_LENGTH;
		::mdl_path = new char[mdl_path_length];
		std::memcpy(::mdl_path, path, i + 1);
		
		::mdl_path[i + 1] = 'm';
		::mdl_path[i + 2] = 'd';
		::mdl_path[i + 3] = 'l';
		::mdl_path[i + 4] = '/';
		
		//Clear out the last bit of data
		for (int j = i + 5; j < mdl_path_length; j++)
			::mdl_path[j] = '\0';
		
	}
	#else
		
		#error "Platform not supported. "
		
	#endif
	
	::ReadRoutingFile();
	
	initialized = true;
	
	return 0;
	
}

int exo::Cleanup()
{
	
	if (!::initialized)
	{
		
		EXO_MESSAGE("EXO is uninitialized. Quitting cleanup. ");
		return 0;
		
	}
	
	EXO_MESSAGE_MISC("Starting cleanup");
	
	::PushTab();
	
	::PopTab();
	
	if (::mdl_path		== nullptr)	delete[] ::mdl_path	;
	if (::p_file_info	== nullptr)	delete[] ::p_file_info	;
	
	EXO_SUCCESS("Cleanup complete. ");
	
	return 0;
	
}

uint exo::AddToBuffers( uint32_t* model, seg::Segment& segment )
{
	
	//How many bytes we need to add
	int	att_byte_count = model[0] * sizeof(GLfloat),
		ind_byte_count = model[1] * sizeof(GLuint );
	
	//Copy the model data into the arrays (unsafe as hell, but idc)
	memcpy(segment.attribs_head, model + 2           , att_byte_count); segment.attribs_head += att_byte_count / sizeof(GLfloat);
	memcpy(segment.indices_head, model + 2 + model[0], ind_byte_count); segment.indices_head += ind_byte_count / sizeof(GLuint );
	
	segment.end += model[1];
	
	return 0;
	
}


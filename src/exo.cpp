#include <iostream>
#include <climits>
#include <unistd.h>
#include <fstream>
#include <cstdint>
#include <cstring>

#include "bug.hpp"
#include "exo.hpp"

#define MAX_MODEL_NAME_LENGTH		10

#define EXO_ERROR_MESSAGE(message)	std::cout << ::tab_spaces << "\x1b[1;31mEXO: Error: \x1b[0;31m" << message << "\x1b[0m" << std::endl;
#define EXO_ERROR_LOG(message, log)	std::cout << ::tab_spaces << "\x1b[1;31mEXO: Error: \x1b[0;31m" << message << std::endl << std::endl\
						<< "\x1b[0;1;31m<log>\x1b[0;31m" << std::endl << log << "\x1b[0m<\\log>" << std::endl;

#define EXO_WARNING_MESSAGE(message)	std::cout << ::tab_spaces << "\x1b[1;33mEXO: Warning: \x1b[0;33m" << message << "\x1b[0m" << std::endl;
#define EXO_WARNING_LOG(message, log)	std::cout << ::tab_spaces << "\x1b[1;33mEXO: Warning: \x1b[0;33m" << message << std::endl << std::endl\
						<< "\x1b[0;1;33m<log>\x1b[0;33m" << std::endl << log << "\x1b[0m<\\log>" << std::endl;

#define EXO_MESSAGE(message)		std::cout << ::tab_spaces << "\x1b[1;34mEXO: \x1b[0m" << message << std::endl;
#define EXO_MESSAGE_MISC(message)	std::cout << ::tab_spaces << "\x1b[1;35mEXO: \x1b[0m" << message << std::endl;

#define EXO_SUCCESS(message)		std::cout << ::tab_spaces << "\x1b[1;32mEXO: \x1b[0;32m" << message << "\x1b[0m" << std::endl << std::endl;

#define EXO_SUBSUCCESS(message)		std::cout << ::tab_spaces << "      -> \x1b[0;32m" << message << "\x1b[0m" << std::endl;
#define EXO_SUBFAIL(message)		std::cout << ::tab_spaces << "      -> \x1b[0;31m" << message << "\x1b[0m" << std::endl;

#define EXO_SUBMESSAGE(message)		std::cout << ::tab_spaces << "      -> " << message << std::endl;
#define EXO_NEWLINE()			std::cout << ::tab_spaces << std::endl;

//The way this works is by taking the buffer and reserving some bits for the file name. 
//So something like
// /this/is/the/path/mdl/**********
//And then to read a file we just modify that "end" data
// /this/is/the/path/mdl/file.edat*

namespace {
	
	bool initialized = false;
	bool edianness = false;
	
	char tab_spaces[16];
	int current_depth = 0;
	void PushTab() { tab_spaces[current_depth++] = '\t'; EXO_NEWLINE(); }
	void PopTab () { tab_spaces[--current_depth] = '\0'; EXO_NEWLINE(); }
	
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
		
		//Load in the route file
		::mdl_path[::starting_offset    ] = 'r';
		::mdl_path[::starting_offset + 1] = 'o';
		::mdl_path[::starting_offset + 2] = 'u';
		::mdl_path[::starting_offset + 3] = 't';
		::mdl_path[::starting_offset + 4] = 'e';
		
		//Open the routing file
		EXO_MESSAGE("Opening routing file \"" << ::mdl_path << "\". ");
		std::ifstream ifs(::mdl_path, std::ios::in | std::ios::binary);
		
		//If the file is open
		if (!ifs.is_open())
		{
			
			EXO_ERROR_MESSAGE("Could not open file\"" << ::mdl_path << "\". ");
			bug::Kill();
			
		}
		
		EXO_SUBMESSAGE("Opened");
		
		//Read the file count and init the lengths buffer
		ifs.read((char*)&file_count, 1);
		
		p_file_info = new FileInfo[file_count];
		
		EXO_MESSAGE("Routing table opened; " << (int)file_count << " mdls to init. ");
		
		::PushTab();
		
		int total_model_count = 0;
		for (int i = 0; i < file_count; i++)
		{
			
			//Read the filename's length
			unsigned char filename_length;
			ifs.read((char*)&filename_length, 1);
			
			//Read out the file name
			char* file_name = new char[filename_length];
			ifs.read(file_name, filename_length);
			
			EXO_MESSAGE("File \"" << file_name << "\". ");
			p_file_info[i].p_filename = file_name;
			
			//Bits
			unsigned char small, middle, large;
			
			//Read length
			ifs.read((char*)&small , 1);
			ifs.read((char*)&middle, 1);
			ifs.read((char*)&large , 1);
			
			//Assemble the length
			p_file_info[i].length = (small | (middle << 8) | (large << 16));
			EXO_SUBMESSAGE(p_file_info[i].length << " byte(s)");
			
			//Read the model count
			ifs.read((char*)&small , 1);
			ifs.read((char*)&large , 1);
			
			//Assemble the model count
			total_model_count += small | (large << 8);
			
		}
		
		::PopTab();
		
		ifs.close();
		
		return 0;
		
	}
	
	char* Read(const char* file_name, unsigned int& file_size)
	{
		
		EXO_MESSAGE("Reading file \"" << file_name << "\". ");
		
		//Fill in that reserve data
		for (int i = 0, j = ::starting_offset; ; (i++, j++))
		{
			
			if (i >= MAX_MODEL_NAME_LENGTH)
			{
				
				EXO_ERROR_MESSAGE("File name too large \"" << file_name << "\". ");
				
			}
			
			//Add in the bit
			::mdl_path[j] = file_name[i];
			
			//If we just added a null, then we're done
			if (file_name[i] == '\0') break;
			
		}
		
		EXO_MESSAGE("Successfully gotten path \"" << ::mdl_path << "\". ");
		
		std::ifstream file_stream(::mdl_path, std::ios::in | std::ios::binary);
		
		EXO_NEWLINE();
		
		if (!file_stream.is_open())
		{
			
			//Fuck
			EXO_ERROR_MESSAGE("Could not open file \"" << ::mdl_path << "\". ");
			bug::Kill();
			
			return nullptr; //More of a formality
			
		}
		
		EXO_MESSAGE("Reading file contents... ");
		
		//Get the end of the file
		file_stream.seekg(0, std::ios::end);
		size_t length = file_stream.tellg();
		file_stream.seekg(0, std::ios::beg);
		
		//Save that
		file_size = length;
		
		//Create the buffer
		char* p_buffer = new char[length];
		
		//Read all the data into the buffer
		file_stream.read(p_buffer, length);
		
		//Delete the filestream
		file_stream.close();
		
		unsigned int identifier = *((unsigned int*)p_buffer);
		
		//If this is the case, we're in the wrong edian. 
		if (identifier >= 0xFFFF)	
		{
			
			EXO_MESSAGE("Edianness incorrect, fixing... ");
			
			//Iterate over every bit and flip the edianness
			for (int i = 0; i < length; i++)
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
			
			EXO_SUCCESS("File \"" << ::mdl_path << "\" has been loaded into memory. ");
			
		}
		
		uint * p_uint  = (uint *)p_buffer;
		float* p_float = (float*)p_buffer;
		
		uint float_count = p_uint[1], uint_count = p_uint[2];
		uint place = 3;
		
		return p_buffer;
		
	}
	
	
};

uint32_t* exo::Model(int file, int model)
{
	
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
	p_file_info[file].p_content  = (uint*)::Read(p_file_info[file].p_filename, length);
	uint *p_content = p_file_info[file].p_content;
	
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
	
	EXO_SUBMESSAGE(::mdl_path);
	EXO_NEWLINE();
	
	::ReadRoutingFile();
	
	std::ofstream of(::mdl_path, std::ios::out | std::ios::binary);
	
	const int FILE_LENGTH = 15;
	of.write("\001\012test.edat\0\124\000\000\000\000", FILE_LENGTH);
	
	of.close();
	
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


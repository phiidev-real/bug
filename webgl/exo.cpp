#include "exo.hpp"
#include <iostream>
#include <climits>
#include <unistd.h>
#include <fstream>
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
	
	class FIFObuff;
	FIFObuff *p_head, *p_tail;
	
	class FIFObuff
	{
		
	public:
		FIFObuff *p_next = nullptr, *p_last = nullptr;
		char *p_buffer;
		
		//This is mostly just ensuring there's no ptr loss
		//Scary stuff...
		 FIFObuff(FIFObuff* ap_last)
		{
			
			//Failsafes
			if (::p_head == nullptr)
				::p_head = this;
			if (::p_tail == nullptr)
				::p_tail = this;
			
			if (ap_last != nullptr)
			{
				
				//If there is no next, we're adding onto the last element (ie this will be last)
				if (ap_last->p_next == nullptr)
					::p_tail = this;
				
				ap_last->p_next->p_last	= this;
				 p_last			= ap_last;
				ap_last->p_next		= this;
				
			}
			
		}
		~FIFObuff()
		{
			
			//If this is the head
			if (p_last == nullptr)	p_head = p_next; //It doesn't matter if p_next is nullptr
								 //It just handles itself
			else			p_last->p_next = p_next;
			if (p_next != nullptr)	p_next->p_last = p_last;
			
			delete[] p_buffer;
			
		}
		
	};
	
};

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
			
			if (i       ==  0 ) return 1;
			if (path[i] == '/') break;
			
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
	
	initialized = true;
	
	return 0;
	
}

int exo::Read(const char* file_name)
{
	
	EXO_MESSAGE("Reading file \"" << file_name << "\"");
	
	//std::cout << ::mdl_path[::starting_offset] << std::endl;
	//Fill in that reserve data
	for (int i = 0, j = ::starting_offset; ; (i++, j++))
	{
		
		if (i >= MAX_MODEL_NAME_LENGTH)
		{
			
			EXO_ERROR_MESSAGE("File name too large \"" << file_name << "\"");
			
		}
		
		//Add in the bit
		::mdl_path[j] = file_name[i];
		
		//If we just added a null, then we're done
		if (file_name[i] == '\0') break;
		
	}
	
	EXO_MESSAGE("Successfully gotten path \"" << ::mdl_path << "\"");
	
	std::ifstream file_stream(::mdl_path, std::ios::in | std::ios::binary);
	
	EXO_NEWLINE();
	
	if (!file_stream.is_open())
	{
		
		//Fuck
		EXO_ERROR_MESSAGE("Could not open file \"" << ::mdl_path << "\". ");
		
		bug::Kill();
		
		return -1;
		
	}
	
	EXO_MESSAGE("Reading file contents");
	
	::p_head = new FIFObuff(nullptr);
	
	//Get the end of the file
	file_stream.seekg(0, std::ios::end);
	size_t length = file_stream.tellg();
	file_stream.seekg(0, std::ios::beg);
	
	//Create the buffer
	::p_head->p_buffer = new char[length];
	
	//Read all the data into the buffer
	file_stream.read(::p_head->p_buffer, length);
	
	//Delete the filestream
	file_stream.close();
	
	unsigned int identifier = *((unsigned int*)::p_head->p_buffer);
	
	//If this is the case, we're in the wrong edian. 
	if (identifier >= 0xFFFF)	
	{
		
		EXO_MESSAGE("Edianness incorrect, fixing...");
		
		//Iterate over every bit and flip the edianness
		for (int i = 0; i < length; i++)
		{
			
			int tmp = ::p_head->p_buffer[i];
			::p_head->p_buffer[i] =	((tmp & 0xFF000000) >> 24) |
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
	
	if (::p_head == nullptr && ::p_tail == nullptr)
	{
		
		EXO_MESSAGE_MISC("Head and tail are nullptr. Exiting. ");
		return 0;
		
	}
	else if (::p_tail == nullptr)
	{
		
		EXO_ERROR_MESSAGE("Tail is nullptr. Cannot initialize cleanup. ");
		return -1;
		
	}
	
	if (::p_tail->p_next != nullptr)
	{
		
		EXO_ERROR_MESSAGE("Tail pointer not last, attempting rectification. ");
		
		for (int i = 0; ::p_tail->p_next != nullptr; i++)
		{
			
			::p_tail = ::p_tail->p_next;
			if (i >= 100)
			{
				
				EXO_ERROR_MESSAGE("Safety limit hit. ");
				bug::Kill();
				
			}
			
		}
		
	}
	
	::PushTab();
	
	while (::p_tail != nullptr)
	{
		
		EXO_MESSAGE_MISC("Deleting buffer. ");
		
		FIFObuff *tmp = ::p_tail;
		::p_tail = p_tail->p_last;
		delete tmp;
		
		EXO_SUBSUCCESS("Deleted");
		
	}
	
	::PopTab();
	
	delete[] mdl_path;
	
	EXO_SUCCESS("Cleanup complete. ");
	
	return 0;
	
}


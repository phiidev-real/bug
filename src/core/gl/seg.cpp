
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "seg.hpp"
#include "bug.hpp"

#include <iostream>
#include <optional>

#larva debug

namespace {
	
	inline GLuint CompileShader(unsigned int type, const char* src, bool& ok)
	{
		
		SEG_MESSAGE("Loading shader. ");
		
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &src, nullptr);
		glCompileShader(shader);
		
		int success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			
			char info_log[512];
			glGetShaderInfoLog(shader, 512, NULL, info_log);
			SEG_ERROR_LOG("Failed to compile shader. ", info_log);
			
			ok = false;
			return shader; //The value doesn't matter
			
		}
		
		SEG_SUBSUCCESS("Successfully compiled shaader (" << shader << ")");
		
		ok = true;
		return shader;
		
	}
	
	GLuint CreateProgram(const char* v_src, const char* f_src)
	{
		
		bool	frg_ok, vrt_ok;
		GLuint	maybe_vrt_shd	= CompileShader(GL_VERTEX_SHADER  , v_src, vrt_ok),
			maybe_frg_shd	= CompileShader(GL_FRAGMENT_SHADER, f_src, frg_ok);
		
		//Shader compiliation failed in some way
		if (!(frg_ok && vrt_ok))
		{
			
			if (vrt_ok) glDeleteShader(maybe_vrt_shd);
			if (frg_ok) glDeleteShader(maybe_vrt_shd);
			
			SEG_ERROR_MESSAGE("Could not create program, shaders invalid. (vrt: " << vrt_ok << ", frg: " << frg_ok << "). ");
			
			bug::Kill();
			
		}
		
		GLuint program = glCreateProgram();
		glAttachShader(program, maybe_vrt_shd);
		glAttachShader(program, maybe_frg_shd);
		glLinkProgram(program);
		
		// check for linking errors
		int success = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success)
		{
			
			char info_log[512];
			glGetProgramInfoLog(program, 512, NULL, info_log);
			
			SEG_ERROR_LOG("Could not create program, linking error. ", info_log);
			bug::Kill();
			
		}
		
		//Delete the maybe shaders
		glDeleteShader(maybe_vrt_shd);
		glDeleteShader(maybe_frg_shd);
		
		SEG_MESSAGE("Created new program. (" << program << "). ");
		
		return program;
		
	}
	
};
	
void seg::Segment::Update()
{
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * attrib_count, attribs, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * index_count, indices, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	
}

void seg::Segment::Create(const char* v_src, const char* f_src)
{
	
	SEG_MESSAGE("Creating new pass");
	PushTab();
	
	//Set the pass as valid
	valid			= 1;
	
	//Create the probram
	program			= ::CreateProgram(v_src, f_src);
	
	//Reset these
	if (attribs != nullptr || indices != nullptr)
	{
		
		SEG_ERROR_MESSAGE("Found data in pointer that should be empty. (attribs: " << attribs << ", indices: " << indices << "). ");
		bug::Kill();
		
	}
	attrib_count		= 0;
	index_count		= 0;
	
	//Generate the buffers
	glGenVertexArrays	(1, &VAO);
	glGenBuffers		(1, &VBO);
	glGenBuffers		(1, &EBO);
	
	glBindVertexArray	(VAO);
	
	PopTab();
	SEG_SUCCESS("Successfully created pass");
	
}

void seg::Segment::Alloc(const unsigned char* a_layout, unsigned int a_attrib_count, unsigned int a_index_count)
{
	
	SEG_MESSAGE("Allocating buffers for pass");
	PushTab();
	
	//Copy layout into the class member, so no weird shit happens
	unsigned int length = 0;
	for (; a_layout[length] != 0 && length < 12; length++)
	{
		
		attrib_length += a_layout[length];
		if (a_layout[length] > 4)
		{
			
			SEG_ERROR_MESSAGE("Layout is invalid; " << a_layout[length] << " > 4. ");
			bug::Kill();
			
		}
		
	}
	
	//Multiply the attrib count so the buffer is sized for a_attrib_count attribs. 
	a_attrib_count *= attrib_length;
	
	SEG_MESSAGE("Allocating layout buffer with size " << length + 1 << ". ");
	
	//Copy lololololol
	layout = new unsigned char[length + 1];
	for (int i = 0; i < length; i++)
	{
		
		layout[i] = a_layout[i];
		
	}
	
	int attrib_size = a_attrib_count * attrib_length;
	if (	attrib_size	<= 0 || attrib_size	>= (1 << 24) ||
		a_index_count	<= 0 || a_index_count	>= (1 << 24))
	{
		
		SEG_ERROR_MESSAGE("Suggested buffer size invalid. (attrib: " << attrib_size << ", index: " << a_index_count << "). ");
		bug::Kill();
		
	}
	SEG_MESSAGE	("Allocating buffers. Sizes: ");
	SEG_SUBMESSAGE	("Attrib buffer sizes\t(bytes) " << attrib_size   * sizeof(GLfloat) << "\t\t(elements) " << a_attrib_count
				<< "\t\t(components) " << attrib_size  );
	SEG_SUBMESSAGE	( "Index buffer sizes\t(bytes) " << a_index_count * sizeof(GLuint ) << "\t\t(elements) " << a_index_count
				<< "\t\t(components) " << a_index_count);
	
	//Init the attrib buffer
	attribs			= new GLfloat[attrib_size];
	attrib_count		= attrib_size;
	
	//Init the index buffer
	indices			= new GLuint[a_index_count];
	index_count		= a_index_count;
	
	PopTab();
	SEG_SUCCESS("Buffers allocated");
	
}

void seg::Segment::Delete()
{
	
	//Quit if the block is freed
	if (!valid)
	{
		
		SEG_SUBSUCCESS("Empty");
		return;
		
	}
	
	//Set this block as invalid
	valid = 0;
	
	//Clean up the program
	glDeleteProgram		(program);
	glDeleteVertexArrays	(1, &VAO);
	glDeleteBuffers		(1, &VBO);
	glDeleteBuffers		(1, &EBO);
	
	//Clean up the data we allocated (if we allocated it)
	if (attribs != nullptr) delete[] attribs; attribs = nullptr; attrib_count = 0;
	if (indices != nullptr) delete[] indices; indices = nullptr; index_count  = 0;
	if (layout  != nullptr) delete[] layout ; layout  = nullptr;
	
	//Delete the start and end of the buffer
	start	= 0;
	end	= 0;
	
	SEG_SUBSUCCESS("Deleted");
	
}


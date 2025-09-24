
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "seg.hpp"
#include "bug.hpp"

#include <iostream>
#include <optional>
#include <cstring>

//TODO: look, idk this is probably one of the best files. Maybe put each class into it's own file?

#larva debug

namespace {
	
	//I hate the way these namespace blocks looks, why did I do it like this
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

  /////////////
 // SEGMENT //
/////////////

seg::Segment:: Segment(const char* v_src, const char* f_src)
{
	
	SEG_MESSAGE("Creating new pass");
	PushTab();
	
	//Create the probram
	program			= ::CreateProgram(v_src, f_src);
	
	//Reset these
	attribs			= nullptr;
	attrib_count		= 0;
	indices			= nullptr;
	index_count		= 0;
	
	//Generate the buffers
	glGenVertexArrays	(1, &VAO);
	glGenBuffers		(1, &VBO);
	glGenBuffers		(1, &EBO);
	
	glBindVertexArray	(VAO);
	
	PopTab();
	SEG_SUCCESS("Successfully created pass");
	
}
seg::Segment::~Segment()
{
	
	//Clean up the program
	glDeleteProgram		(program);
	glDeleteVertexArrays	(1, &VAO);
	glDeleteBuffers		(1, &VBO);
	glDeleteBuffers		(1, &EBO);
	
	//Clean up the data we allocated (if we allocated it)
	if (attribs != nullptr) delete[] attribs;
	if (indices != nullptr) delete[] indices;
	if (layout  != nullptr) delete[] layout ;
	
	SEG_SUBSUCCESS("Deleted! ");
	
}

void seg::Segment::Update()
{
	
	glBindVertexArray(VAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO); //No need to do this for each var, cuz interleaving 'n' whatnot
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * attrib_count, attribs, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * index_count, indices, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, attrib_length, GL_FLOAT, GL_FALSE, attrib_length * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	
}

void seg::Segment::Alloc(const unsigned char* a_layout, unsigned int a_attrib_count, unsigned int a_index_count)
{
	
	SEG_MESSAGE("Allocating buffers for pass");
	PushTab();
	
	//Copy layout into the class member, so no weird shit happens
	attrib_length = 0;
	unsigned int length = 0;
	for (; a_layout[length] != 0 && length < 12; length++)
	{
		
		attrib_length += a_layout[length];
		if (a_layout[length] > 4)
		{
			
			SEG_ERROR_MESSAGE("Layout is invalid (" << a_layout[length] << " > 4). ");
			bug::Kill();
			
		}
		
	}
	
	SEG_MESSAGE("Allocating layout buffer with size " << length + 1 << ". ");
	
	//Copy lololololol
	layout = new unsigned char[length + 1];
	strcpy((char*)layout, (char*)a_layout);
	
	int attrib_size = a_attrib_count * attrib_length;
	if (	attrib_size	< 0 || attrib_size	> (1 << 24) ||
		a_index_count	< 0 || a_index_count	> (1 << 24))
	{
		
		std::cout << attrib_size << "\n";
		SEG_ERROR_MESSAGE("Suggested buffer size invalid. (attrib: " << attrib_size << ", index: " << a_index_count << "). ");
		bug::Kill();
		
	}
	SEG_MESSAGE	("Allocating buffers. Sizes: ");
	SEG_SUBMESSAGE	("Attrib buffer sizes\t(bytes) " << attrib_size   * sizeof(GLfloat) << "\t\t(elements) " << a_attrib_count
				<< "\t\t(components) " << attrib_size  );
	SEG_SUBMESSAGE	( "Index buffer sizes\t(bytes) " << a_index_count * sizeof(GLuint ) << "\t\t(elements) " << a_index_count
				<< "\t\t(components) " << a_index_count);
	
	//Init the buffers
	if (attrib_size   == 0)	attribs = nullptr;
	else			attribs = new GLfloat[attrib_size];
	if (a_index_count == 0)	indices = nullptr;
	else			indices = new GLuint [a_index_count];
	
	//Set the index count
	index_count		= a_index_count;
	attrib_count		= attrib_size;
	std::cout << attrib_size << "\n";
	
	PopTab();
	SEG_SUCCESS("Buffers allocated");
	
}

void seg::Segment::Call(int window_width, int window_height)
{
	
	glViewport(0, 0, width == 0 ? window_width : width, height == 0 ? window_height : height);
	
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	
	glUseProgram(program);
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	glClear(
		
		(mask & 2 ? GL_COLOR_BUFFER_BIT : 0) |
		(mask & 4 ? GL_DEPTH_BUFFER_BIT : 0)
		
	);
	
	glEnable(GL_DEPTH_TEST);
	if (mask & 8)	glEnable (GL_CULL_FACE);
	else			glDisable(GL_CULL_FACE);
	glEnable(GL_PROGRAM_POINT_SIZE);
	
	glBindVertexArray(VAO);
	if (mask & 1)	glDrawArrays  (GL_TRIANGLES, start, end);
	else		glDrawElements(GL_TRIANGLES, end  , GL_UNSIGNED_INT, (void*)(start * sizeof(uint)));
	
}

void seg::Segment::Bind(seg::Target* ap_target)
{
	
	framebuffer = ap_target->m_framebuffer;
	
	//Check the framebuffer's valility
	glBindFramebuffer(GL_FRAMEBUFFER, ap_target->m_framebuffer);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) bug::Kill();
	
}

  /////////////
 // TEXTURE //
/////////////

seg::Texture:: Texture(int a_width, int a_height)
{
	
	m_width  = a_width ;
	m_height = a_height;
	
	glGenTextures(1, &texture);
	
}
seg::Texture::~Texture()
{
	
	glDeleteTextures(1, &texture);
	
}
void seg::Texture::Update()
{
	
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	//Fill it up with 0s
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	
	//Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
}

  ////////////
 // Target //
////////////

seg::Target::Target(int a_width, int a_height)
{
	
	//Framebuffer
	glGenFramebuffers(1, &m_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
	
	//Depth buffer
	glGenRenderbuffers(1, &m_depthbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_depthbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, a_width, a_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthbuffer);
	
}

void seg::Target::SetTexture(seg::Texture* ap_texture)
{
	
	mp_texture = ap_texture;
	
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ap_texture->texture, 0);
	
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);
	
}


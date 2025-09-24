
#ifndef GL_SEG_H
#define GL_SEG_H

#include "bug.hpp"

#include <vector>

#define SEG_TARGET_SCREEN 0

namespace seg {
	
	class Segment;
	class Texture;
	class Target;
	
	//TODO: Clean up this bullshit
	//So much stuff, none of it needed
	class Segment {
		
	public:
		bool	valid = 0;
		
		GLuint	program;
		GLuint	VAO, VBO, EBO;
		
		GLfloat* attribs;
		GLuint * indices;
		unsigned int attrib_count; //This isn't actually the number of attribs, it's the number of floats in the buffer
		unsigned int index_count;
		
		GLfloat* attribs_head;
		GLuint * indices_head;
		
		unsigned char *layout;
		unsigned int  attrib_length;
		
		unsigned int start;
		unsigned int end;
		
		unsigned short width;
		unsigned short height;
		
		//Replacing this with a template and adding templates for this would maybe make it work better?
		//It would be more modern c++, I guess?
		void Alloc (const unsigned char*, unsigned int, unsigned int);
		void Update();
		void Delete();
		void Call  (int, int);
		
		void Bind  (Target* ap_target);
		
		//Render target stuff
		GLuint framebuffer, color_texture, depth_texture;
		uint8_t mask = 0;
		
		~Segment();
		 Segment(const char*, const char*);
		
	};
	
	class Texture {
		
	public:
		 Texture(int, int);
		~Texture();
		
		void Update();
		
		GLuint	texture;
		uint	unit;
		
	private:
		int	m_width;
		int	m_height;
		
	};
	
	//This doesn't really support pingponging
	//oh well
	class Target
	{
		
		friend Segment;
		
	public:
		Target(int, int);
		void SetTexture(Texture*);
		
	private:
		GLuint m_framebuffer;
		GLuint m_depthbuffer;
		Texture *mp_texture;
		
	};
	
};

#endif // GL_SEG_H


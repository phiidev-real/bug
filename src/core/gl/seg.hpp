
#ifndef GL_SEG_H
#define GL_SEG_H

#include "bug.hpp"

namespace seg {
	
	struct Segment {
		
		bool	valid = 0;
		
		GLuint	program;
		GLuint	VAO, VBO, EBO;
		
		GLfloat* attribs;
		GLuint * indices;
		unsigned int attrib_count;
		unsigned int index_count;
		
		GLfloat* attribs_head;
		GLuint * indices_head;
		
		unsigned char *layout;
		unsigned int  attrib_length;
		
		unsigned int start;
		unsigned int end;
		
		void Create(const char*, const char*);
		void Alloc (const unsigned char*, unsigned int, unsigned int);
		void Update();
		void Delete();
		
	};
	
};

#endif // GL_SEG_H


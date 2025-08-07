
//I have changed this file

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <optional>
#include <cstring>
#include <cmath>

#include "../bug.hpp"
#include "../exo.hpp"

#define BUG_ERROR_MESSAGE(message)	std::cout << ::tab_spaces << "\x1b[1;31mBUG: Error: \x1b[0;31m" << message << "\x1b[0m" << std::endl;
#define BUG_ERROR_LOG(message, log)	std::cout << ::tab_spaces << "\x1b[1;31mBUG: Error: \x1b[0;31m" << message << std::endl << std::endl\
						<< "\x1b[0;1;31m<log>\x1b[0;31m" << std::endl << log << "\x1b[0m<\\log>" << std::endl;

#define BUG_MESSAGE(message)		std::cout << ::tab_spaces << "\x1b[1;34mBUG: \x1b[0m" << message << std::endl;
#define BUG_MESSAGE_MISC(message)	std::cout << ::tab_spaces << "\x1b[1;35mBUG: \x1b[0m" << message << std::endl;

#define BUG_SUCCESS(message)		std::cout << ::tab_spaces << "\x1b[1;32mBUG: \x1b[0;32m" << message << "\x1b[0m" << std::endl << std::endl;

#define BUG_SUBSUCCESS(message)		std::cout << ::tab_spaces << "      -> \x1b[0;32m" << message << "\x1b[0m" << std::endl;
#define BUG_SUBFAIL(message)		std::cout << ::tab_spaces << "      -> \x1b[0;31m" << message << "\x1b[0m" << std::endl;

#define BUG_SUBMESSAGE(message)		std::cout << ::tab_spaces << "      -> " << message << std::endl;
#define BUG_NEWLINE()			std::cout << ::tab_spaces << std::endl;

float cam_x = 0.0f, cam_y = 0.0f, cam_z = 0.0f;

const char *v_std = R"(#version 330 core

uniform mat4 u_cam_matrix;

layout (location = 0) in vec3 a_position;

out vec3 worldpos;

void main()
{
	
	gl_Position = vec4(
		u_cam_matrix * vec4(a_position.xyz, 1.0)
	);
	
	worldpos = a_position;
	
})";
const char *f_std = R"(#version 330 core

in vec3 worldpos;

out vec4 FragColor;

void main()
{
	
	FragColor = vec4( worldpos.zzz, 1.0 );
	
})";

namespace {
	
	char tab_spaces[8];
	int current_depth = 0;
	void PushTab() { tab_spaces[current_depth++] = '\t'; BUG_NEWLINE(); }
	void PopTab () { tab_spaces[--current_depth] = '\0'; BUG_NEWLINE(); }
	
	struct Segment {
		
		bool	valid = 0;
		GLuint	program;
		GLuint	VAO, VBO, EBO;
		
		GLfloat* attribs;
		GLuint * indices;
		unsigned int attrib_count;
		unsigned int index_count;
		
		unsigned char *layout;
		unsigned int  attrib_length;
		
		unsigned int start;
		unsigned int end;
		
		void Create(const char*, const char*);
		void Alloc (const unsigned char*, unsigned int, unsigned int);
		void Update();
		void Delete();
		
	};
	
	const int MAX_PASS_COUNT = 16;
	int passes_in_use = 0;
	Segment passes[MAX_PASS_COUNT];	//If you need more than 16 calls, you're doing something wrong.
						//Unless it's like bloom or something, then you should modify the source to add post stuff.
	
	//Originally this would just be in exit, but it's cleaner this way. 
	void Cleanup(int init_stage)
	{
		
		exo::Cleanup();
		
		BUG_MESSAGE_MISC("Starting cleanup. ");
		
		PushTab();
		
		switch (init_stage)
		{
			
			case 2:
				BUG_MESSAGE_MISC("Deleting passes...");
				for (int i = 0; i < ::MAX_PASS_COUNT; i++)
				{
					
					BUG_MESSAGE_MISC("Deleting pass #" << i);
					::passes[i].Delete();
					
				}
				BUG_SUCCESS("All passes deleted");
				BUG_NEWLINE();
				
			case 1:
				BUG_MESSAGE_MISC("Terminating GLFW. ");
				glfwTerminate();
				BUG_SUBSUCCESS("GLFW terminated");
				
			
		}
		
		PopTab();
		
		BUG_SUCCESS("Cleanup complete. Quitting. ");
		
	}
	
	void Exit(int init_stage, int status)
	{
		
		Cleanup(init_stage);
		
		if (status)
		{
			
			BUG_ERROR_MESSAGE("Exiting with nonzero value");
			
		}
		else
		{
			
			BUG_MESSAGE("Exiting with 0. ");
			
		}
		
		exit(status);
		
	}
	inline void Fail(int init_stage) { Exit(init_stage, EXIT_FAILURE); }
	
	inline std::optional<GLuint> CompileShader(unsigned int type, const char* src)
	{
		
		BUG_MESSAGE("Loading shader. ");
		
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &src, nullptr);
		glCompileShader(shader);
		
		int success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			
			char info_log[512];
			glGetShaderInfoLog(shader, 512, NULL, info_log);
			BUG_ERROR_LOG("Failed to compile shader. ", info_log);
			
			return std::optional<GLuint>();
			
		}
		
		BUG_SUBSUCCESS("Successfully compiled shaader (" << shader << ")");
		
		return std::optional<GLuint>(shader);
		
	}
	
	GLuint CreateProgram(const char* v_src, const char* f_src)
	{
		
		std::optional<GLuint>	maybe_vrt_shd	= CompileShader(GL_VERTEX_SHADER  , v_src),
					maybe_frg_shd	= CompileShader(GL_FRAGMENT_SHADER, f_src);
		
		//Shader compiliation failed in some way
		if (!(maybe_vrt_shd.has_value() && maybe_frg_shd.has_value()))
		{
			
			if (maybe_vrt_shd.has_value()) glDeleteShader(*maybe_vrt_shd);
			if (maybe_frg_shd.has_value()) glDeleteShader(*maybe_vrt_shd);
			
			BUG_ERROR_MESSAGE("Could not create program, shaders invalid. (vrt: " << maybe_vrt_shd.has_value() << ", " << maybe_frg_shd.has_value() << "). ");
			
			Fail(2);
			
		}
		
		GLuint program = glCreateProgram();
		glAttachShader(program, *maybe_vrt_shd);
		glAttachShader(program, *maybe_frg_shd);
		glLinkProgram(program);
		
		// check for linking errors
		int success = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success)
		{
			
			char info_log[512];
			glGetProgramInfoLog(program, 512, NULL, info_log);
			
			BUG_ERROR_LOG("Could not create program, linking error. ", info_log);
			Fail(2);
			
		}
		
		//Delete the maybe shaders
		glDeleteShader(*maybe_vrt_shd);
		glDeleteShader(*maybe_frg_shd);
		
		BUG_MESSAGE("Created new program. (" << program << "). ");
		
		return program;
		
	}
	
	//This just handles resizing the viewport. 
	//Yehaw
	void FramebufferCallback(GLFWwindow* window, int width, int height)
	{
		
		glViewport(0, 0, width, height);
		
	}
	
	bool key_d, key_a;
	void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		
		     if (key == GLFW_KEY_D && action == GLFW_RELEASE) key_d = 0;
		else if (key == GLFW_KEY_D && action == GLFW_PRESS  ) key_d = 1;
		     if (key == GLFW_KEY_A && action == GLFW_RELEASE) key_a = 0;
		else if (key == GLFW_KEY_A && action == GLFW_PRESS  ) key_a = 1;
		
	}
	
};

void ::Segment::Update()
{
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * attrib_count, attribs, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * index_count, indices, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	
}

void ::Segment::Create(const char* v_src, const char* f_src)
{
	
	BUG_MESSAGE("Creating new pass");
	PushTab();
	
	//Set the pass as valid
	valid			= 1;
	
	//Create the probram
	program			= ::CreateProgram(v_src, f_src);
	
	//Reset these
	if (attribs != nullptr || indices != nullptr)
	{
		
		BUG_ERROR_MESSAGE("Found data in pointer that should be empty. (attribs: " << attribs << ", indices: " << indices << "). ");
		Fail(2);
		
	}
	attrib_count		= 0;
	index_count		= 0;
	
	//Generate the buffers
	glGenVertexArrays	(1, &VAO);
	glGenBuffers		(1, &VBO);
	glGenBuffers		(1, &EBO);
	
	glBindVertexArray	(VAO);
	
	PopTab();
	BUG_SUCCESS("Successfully created pass");
	
}

void ::Segment::Alloc(const unsigned char* a_layout, unsigned int a_attrib_count, unsigned int a_index_count)
{
	
	BUG_MESSAGE("Allocating buffers for pass");
	PushTab();
	
	//Copy layout into the class member, so no weird shit happens
	unsigned int length = 0;
	for (; a_layout[length] != 0 && length < 12; length++)
	{
		
		attrib_length += a_layout[length];
		if (a_layout[length] > 4)
		{
			
			BUG_ERROR_MESSAGE("Layout is invalid; " << a_layout[length] << " > 4. ");
			::Fail(2);
			
		}
		
	}
	
	BUG_MESSAGE("Allocating layout buffer with size " << length + 1 << ". ");
	
	//Copy lololololol
	layout = new unsigned char[length + 1];
	for (int i = 0; i < length; i++)
	{
		
		layout[i] = a_layout[i];
		
	}
	
	int attrib_size = a_attrib_count * attrib_length;
	if (
		attrib_size	<= 0 || attrib_size	>= (1 << 24) ||
		a_index_count	<= 0 || a_index_count	>= (1 << 24))
	{
		
		BUG_ERROR_MESSAGE("Suggested buffer size invalid. (attrib: " << attrib_size << ", index: " << a_index_count << "). ");
		::Fail(2);
		
	}
	BUG_MESSAGE	("Allocating buffers. Sizes: ");
	BUG_SUBMESSAGE	("Attrib buffer sizes\t(bytes) " << attrib_size   * sizeof(GLfloat) << "\t\t(elements) " << a_attrib_count
				<< "\t\t(components) " << attrib_size  );
	BUG_SUBMESSAGE	( "Index buffer sizes\t(bytes) " << a_index_count * sizeof(GLuint ) << "\t\t(elements) " << a_index_count
				<< "\t\t(components) " << a_index_count);
	
	//Init the attrib buffer
	attribs			= new GLfloat[attrib_size];
	attrib_count		= attrib_size;
	
	//Init the indix buffer
	indices			= new GLuint[a_index_count];
	index_count		= a_index_count;
	
	PopTab();
	BUG_SUCCESS("Buffers allocated");
	
}

void ::Segment::Delete()
{
	
	//Quit if the block is freed
	if (!valid)
	{
		
		BUG_SUBSUCCESS("Empty");
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
	
	BUG_SUBSUCCESS("Deleted");
	
}

int main()
{
	
	BUG_MESSAGE("Initialzing \x1b[1;34mBUG\x1b[0m. ");
	
	//Init glfw
	if (!glfwInit())
	{
		
		BUG_ERROR_MESSAGE("Could not initialize GLFW. ");
		::Fail(0);
		
	}
	
	//Version set go brrrr
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__ //Imagine using apple lmao. Can't be me. 
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	
	//Creating the window
	GLFWwindow* window = glfwCreateWindow(800, 800, "LearnOpenGL", NULL, NULL);
	
	if (window == NULL) //Error handling
	{
		
		BUG_ERROR_MESSAGE("Could not open the window. ");
		::Fail(1);
		
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, ::FramebufferCallback);
	
	//Load GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		
		BUG_ERROR_MESSAGE("Could not initialize GLAD. ");
		::Fail(1);
		
	}
	
	::passes[0].Create(v_std, f_std);
	::passes[0].Alloc((const unsigned char*)"\3\0", 512, 512 * 3);
	
	::passes_in_use++;
	
	exo::Init();
	uint triangles = 0;
	uint32_t* m = exo::Model(0, 0);
	uint32_t* models[1];
	
	models[0] = m;
	
	for (int i = 0; i < 1; i++)
	{
		
		//Load the model data
		uint32_t* model = models[0];
		
		memcpy(::passes[0].attribs, model + 2           , model[0] * sizeof(GLfloat));
		memcpy(::passes[0].indices, model + 2 + model[0], model[1] * sizeof(GLuint ));
		
		triangles += model[1];
		
		::passes[0].start = 0;
		::passes[0].end   = triangles;
		
		::passes[0].Update();
		
	}
	
	GLint	uniform_location = glGetUniformLocation(::passes[0].program, "temp"),
		u_cam_loc	 = glGetUniformLocation(::passes[0].program, "u_cam_matrix");
	
	float mat[] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	
	glfwSetKeyCallback(window, ::KeyCallback);
	
	while (!glfwWindowShouldClose(window))
	{
		
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		mat[12] = cam_x;
		
		for (int i = 0; i < ::passes_in_use; i++)
		{
			
			glUseProgram(::passes[i].program);
			glEnable(GL_DEPTH_TEST);
			
			glUniformMatrix4fv(u_cam_loc, 1, GL_FALSE, mat);
			
			glBindVertexArray(::passes[i].VAO);
			glDrawElements(GL_TRIANGLES, ::passes[i].end, GL_UNSIGNED_INT, (void*)::passes[i].start);
			
		}
 						
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	}
	
	BUG_MESSAGE("Main loop exited. ");
	::Exit(2, EXIT_SUCCESS);
	
	return 0;
	
}

void bug::Kill()
{
	
	BUG_MESSAGE("Killing the program. ");
	BUG_NEWLINE();
	
	::Fail(2);
	
}


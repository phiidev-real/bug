
//I have changed this file

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <optional>
#include <cstring>
#include <cmath>

#include "man.hpp"
#include "bug.hpp"
#include "exo.hpp"
#include "seg.hpp"
#include "ant.hpp"
#include "cpi.hpp"
#include "srm.hpp"

#larva debug

float cam_x = 0.0f, cam_y = 0.0f, cam_z = 0.0f;

#define DELTA_TIME 30.0f

//TODO: Get this out of here. 

class Test : private srm::Agent {
	
private:
	float dy = 0;
	
	int Update()
	{
		
		dy -= 2.0f / DELTA_TIME;
		
		p_mat->x += (ant::D() - ant::A()) / 128.0f; /// 64.0f;
		p_mat->y += dy / DELTA_TIME;
		
		if ( ant::W() && dy <= 0.0f ) dy = 1.0f;
		if (p_mat->y <= 0.0f)
		{
			
			if (dy <= 0.0f) dy = 0.0f;
			p_mat->y = 0.0f;
			
		}
		
		return 0;
		
	}
	int Init() { return 0; }
	
public:
	
	//Genuinely what the hell is this. How did I write something like this
	static int loopbackUpdate(Test* t) { return t->Update(); };
	static int loopbackInit  (Test* t) { return t->Init  (); };
	
};

#include "glsl.hpp"

namespace {
	
	//Originally this would just be in exit, but it's cleaner this way. 
	void Cleanup(int init_stage)
	{
		
		BUG_MESSAGE_MISC("Starting cleanup. ");
		
		PushTab();
		
		
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
	
	class Uniform {
		
	public:
		GLuint program ;
		GLuint location;
		
		virtual void Update() { };
		
	};
	class Uniform2f : public Uniform {
		
	public:
		GLfloat x, y;
		
		void Update()
		{
			
			glUseProgram(program);
			glUniform2f(location, x, y);
			
		}
		
	};
	
	std::vector<Uniform2f> viewports;
	
	//This just handles resizing the viewport. 
	//Yehaw
	int width = 256, height = 144;
	void FramebufferCallback(GLFWwindow* a_window, int a_width, int a_height)
	{
		
		width  = a_width ;
		height = a_height;
		
		for (auto it = viewports.begin(); it != viewports.end(); it++)
		{
			
			it->x = (float)a_width ;
			it->y = (float)a_height;
			
			it->Update();
			
		}
		
	}
	
};

namespace {
	
	void TerminateGLFW() {
		
		BUG_MESSAGE_MISC("Terminating GLFW. ");
		glfwTerminate();
		BUG_SUBSUCCESS("GLFW terminated. ");
		
	};
	
}

int main()
{
	
	//On program init. Before graphics pipeline is initialized. 
	cpi::Awake();
	
	BUG_MESSAGE("Initialzing \x1b[1;34mBUG\x1b[0m. ");
	
	//Init glfw
	if (!glfwInit())
	{
		
		BUG_ERROR_MESSAGE("Could not initialize GLFW. ");
		return -1;
		
	}
	
	//Version set go brrrr
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__ //Imagine using apple lmao. Can't be me. 
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	
	//Creating the window
	GLFWwindow* window = glfwCreateWindow(width, height, "LearnOpenGL", NULL, NULL);
	
	if (window == NULL) //Error handling
	{
		
		BUG_ERROR_MESSAGE("Could not open the window. ");
		TerminateGLFW();
		return -1;
		
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, ::FramebufferCallback);
	
	//Load GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		
		BUG_ERROR_MESSAGE("Could not initialize GLAD. ");
		TerminateGLFW();
		return -1;
		
	}
	
	const int height = 256 / 16 * 9;
	
	glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
	
	man::manager<seg::Segment> segment_manager;
	man::manager<seg::Texture> texture_manager;
	
	seg::Segment* segment0 = segment_manager.Create(v_std, f_std);
	segment0->Alloc((const unsigned char*)"\2\1\0", 512, 512);
	segment0->width  = 256;
	segment0->height = 256 / 16 * 9;
	segment0->mask   = 2 | 4;
	segment0->framebuffer = 0;
	
	seg::Segment* segment1 = segment_manager.Create(v_tex, f_tex);
	segment1->Alloc((const unsigned char*)"\2\0", 4, 6);
	segment1->width  = 0;
	segment1->height = 0;
	segment1->start  = 0;
	segment1->end    = 6;
	segment1->mask   = 2 | 4;
	
	
	
	segment1->attribs[ 0] =  1.0;
	segment1->attribs[ 1] =  1.0;
	
	segment1->attribs[ 2] = -1.0;
	segment1->attribs[ 3] =  1.0;
	
	segment1->attribs[ 4] =  1.0;
	segment1->attribs[ 5] = -1.0;
	
	segment1->attribs[ 6] = -1.0;
	segment1->attribs[ 7] = -1.0;
	
	
	segment1->indices[ 0] =    0;
	segment1->indices[ 1] =    1;
	segment1->indices[ 2] =    2;
	
	segment1->indices[ 3] =    3;
	segment1->indices[ 4] =    1;
	segment1->indices[ 5] =    2;
	
	
	
	segment1->Update();
	
	GLint	loc_texture	= glGetUniformLocation(segment1->program, "u_texture"  ),
		dimension	= glGetUniformLocation(segment1->program, "u_dimension");
	
	Uniform2f& uniform	= viewports.emplace_back();
	uniform.program		= segment1->program;
	uniform.location	= dimension;
	
	uniform.x		= ::width ;
	uniform.y		= ::height;
	uniform.Update();
	
	glUseProgram(segment1->program);
	glUniform1i(loc_texture, 1);
	
	GLint	u_cam_loc	 = glGetUniformLocation(segment0->program, "u_cam_matrix");
	
	float	f  = 10.0f,
		n  = 2.0f,
		sx = 1.0f,
		sy = 1.0f,
		sz = 1.0f / 10.0f,
		mv = 2*f / (f-n) - 1,
		bv = 2*f*n / (f-n) - 1,
		mat[] = {
			
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, sz  , 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f 
			
		};
	
	glfwSetKeyCallback(window, bug::KeyCallback);
	
	exo::Init();
	
	srm::Init();
	srm::Create((int (*)(void*))Test::loopbackUpdate, (int (*)(void*))Test::loopbackInit, exo::Model(0, 0), sizeof(Test));
	
	srm::ConstructBuffers( segment0 );
	segment0->Update();
	
	seg::Texture* texture_handle = texture_manager.Create(256, 144);
	texture_handle->unit = 1;
	texture_handle->Update();
	
{
	
	seg::Target target(256, 144);
	
	target.SetTexture(texture_handle);
	
	segment0->Bind(&target);
	
}
	
	double last_time = 0;
	while (!glfwWindowShouldClose(window))
	{
		
		bug::Update();
		
		double now = glfwGetTime(), dtime = now - last_time;
		if (dtime >= 1.5 / 60.0) std::cout << "Problem...\n";
		last_time = now;
		
		segment0->Call(::width, ::height);
		segment1->Call(::width, ::height);
		
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	}
	
	BUG_MESSAGE("Main loop exited. ");
	
	texture_manager.Empty();
	segment_manager.Empty();
	
	TerminateGLFW();
	
	exo::Cleanup();
	
	::Exit(2, EXIT_SUCCESS);
	
	return 0;
	
}

void bug::Kill()
{
	
	BUG_MESSAGE("Killing the program. ");
	BUG_NEWLINE();
	
	
	exo::Cleanup();
	::Fail(2);
	
}


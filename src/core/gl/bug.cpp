
//I have changed this file

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <optional>
#include <cstring>
#include <cmath>

#include "bug.hpp"
#include "exo.hpp"
#include "seg.hpp"
#include "ant.hpp"
#include "cpi.hpp"
#include "srm.hpp"

//#larva impl-hook bug_header

#larva debug

float cam_x = 0.0f, cam_y = 0.0f, cam_z = 0.0f;

const char *v_std = R"(#version 330 core

uniform mat4 u_cam_matrix;
uniform sampler2D u_matrix_texture;

layout (location = 0) in vec2  a_position;
layout (location = 1) in float a_matrix;

out vec2 v_uv;

void main()
{
	
	vec4	v0 = texture(u_matrix_texture, vec2(0.5 / 3.0, (a_matrix + 0.5) / 256.0)),
		v1 = texture(u_matrix_texture, vec2(1.5 / 3.0, (a_matrix + 0.5) / 256.0)),
		v2 = texture(u_matrix_texture, vec2(2.5 / 3.0, (a_matrix + 0.5) / 256.0));
	
	vec4 worldpos_vec4 = 
		mat4(
			
			1.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0,
			v0.xy, 0.0, 1.0
			
		) * vec4(a_position.xy, 0.0, 1.0);
	
	vec4 pos = u_cam_matrix * worldpos_vec4;
	gl_Position = pos;
	
	v_uv = sign(a_position);
	
})";
const char *f_std = R"(#version 330 core

uniform sampler2D u_matrix_texture;

in vec2 v_uv;

out vec4 FragColor;

void main()
{
	
	FragColor = vec4( float(dot(v_uv.xy, v_uv.xy) < 1.0), 0.0, 0.0, 1.0 );
	
})";

namespace {
	
	const int MAX_PASS_COUNT = 8;
	int passes_in_use = 0;
	seg::Segment passes[MAX_PASS_COUNT];	//If you need more than 16 calls, you're doing something wrong.G
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
				BUG_MESSAGE_MISC("Deleting passes... ");
				for (int i = 0; i < ::MAX_PASS_COUNT; i++)
				{
					
					BUG_MESSAGE_MISC("Deleting pass #" << i);
					::passes[i].Delete();
					
				}
				BUG_SUCCESS("All passes deleted! ");
				BUG_NEWLINE();
				
			case 1:
				BUG_MESSAGE_MISC("Terminating GLFW. ");
				glfwTerminate();
				BUG_SUBSUCCESS("GLFW terminated. ");
				
			
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
	
	//This just handles resizing the viewport. 
	//Yehaw
	void FramebufferCallback(GLFWwindow* window, int width, int height)
	{
		
		glViewport(0, 0, width, height);
		
	}
	
};

int test_ai(srm::Matrix& mat)
{
	
	mat.x += (ant::D() - ant::A()) / 32.0f;
	mat.y += (ant::W() - ant::S()) / 32.0f;
	
	return 0;
	
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
	GLFWwindow* window = glfwCreateWindow(256, 240, "LearnOpenGL", NULL, NULL);
	
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
	
	glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
	
	::passes[0].Create(v_std, f_std);
	::passes[0].Alloc((const unsigned char*)"\2\1\0", 512, 512);
	
	::passes_in_use++;
	
	exo::Init();
	
	int	attribs_offset[MAX_PASS_COUNT] = {},
		indices_offset[MAX_PASS_COUNT] = {};
	
	GLint	uniform_location = glGetUniformLocation(::passes[0].program, "temp"),
		u_cam_loc	 = glGetUniformLocation(::passes[0].program, "u_cam_matrix");
	
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
	
	srm::Init();
	srm::CreateAgent( exo::Model(0, 0), test_ai );
	
	srm::ConstructBuffers( ::passes );
	
	while (!glfwWindowShouldClose(window))
	{
		
		glClear(GL_DEPTH_BUFFER_BIT);
		
		bug::Update();
		
		for (int i = 0; i < ::passes_in_use; i++)
		{
			
			glUseProgram(::passes[i].program);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_CW);
			
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


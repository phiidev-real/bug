
//This is just a quick fix for the glsl. I should be reading from files tbh. smh my head

#ifndef GLSL_H
#define GLSL_H

const char *v_std = R"(#version 330 core

uniform mat4 u_cam_matrix;
uniform sampler2D u_matrix_texture;

layout (location = 0) in vec2  a_position;
layout (location = 1) in float a_matrix;

out vec2 v_uv;

void main()
{
	
	vec4 v0 = texture(u_matrix_texture, vec2(0.5, (a_matrix + 0.5) / 256.0)); //u doesn't matter
	
	vec4 worldpos_vec4 =
		mat4(
			
			9.0 / 16.0, 0.0, 0.0, 0.0,
			0.0	  , 1.0, 0.0, 0.0,
			0.0	  , 0.0, 1.0, 0.0,
			v0.xy, 0.0, 1.0
			
		) * vec4(a_position.xy, 0.0, 1.0);
	
	vec4 pos = worldpos_vec4;
	gl_Position = pos;
	
	v_uv = sign(a_position);
	
})",
	*f_std = R"(#version 330 core

uniform sampler2D u_matrix_texture;

in vec2 v_uv;

out vec4 FragColor;

float lines(float x) { return float(int(x) & 1); }

void main()
{
	
	FragColor = vec4( lines(gl_FragCoord.x), lines(gl_FragCoord.y), 1.0, 1.0 );
	
})",
	*v_tex = R"(#version 330 core

layout (location = 0) in vec2 a_position;

out vec2 v_uv;

void main()
{
	
	gl_Position = vec4(a_position, 0.0, 1.0);
	gl_PointSize = 100.0;
	v_uv = (a_position + 1.0) * 0.5;
	
})",
	*f_tex = R"(#version 330 core

uniform sampler2D u_texture;
uniform vec2	  u_dimension;

in vec2 v_uv;

out vec4 FragColor;

void main()
{
	
//	vec2	screen_size	= vec2(256.0, 144.0),
//		uv		= v_uv - vec2(0.5, 0.5) / screen_size, //(v_uv / screen_size - vec2(0.5, 0.5)) * screen_size,
//		pixel_ratio	= u_dimension / screen_size,
//		screen_uv	= uv * screen_size,
//		truncated	= (ceil(screen_uv) - 0.5) / screen_size,
//		remainder	= (uv - truncated) * screen_size,
//		blend		= (clamp(remainder * pixel_ratio, -1.0, 1.0) * 0.5 + 0.5) / screen_size;
	
	vec2	screen_size		= vec2(256.0, 144.0),
		centre_of_pixel		= (ceil(v_uv * screen_size) - 0.5) / screen_size,
		deviance_from_centre	= (v_uv - centre_of_pixel) * screen_size, //screen_size is normalized
		blend_coefficient	= max(abs(deviance_from_centre), 0.0) * sign(deviance_from_centre);
	
	FragColor = texture( u_texture, centre_of_pixel );
	
})";

#endif // GLSL_H


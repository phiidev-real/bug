
#include <iostream>
#include <vector>

#include "bug.hpp"
#include "ant.hpp"
#include "srm.hpp"

//Code made to manage ebtites
//LMAO EBTITIES

#larva debug

float data[4 * 3 * 256] = { 0 };
srm::Matrix* matrices;

GLuint texture;

namespace {
	
	char matrix_usage[SRM_MATRIX_COUNT / 8] = { 0 };
	uint first_free = 0;
	
	std::vector<srm::Agent> agents;
	
	inline bool mu_index(int ind		) { return matrix_usage[ind >> 3] & (1 << (ind & 0x7)); }
	inline void mu_write(int ind, bool val	)
	{
		
		if (val) matrix_usage[ind >> 3] |=  (1 << (ind & 0x7));
		else	 matrix_usage[ind >> 3] &= ~(1 << (ind & 0x7));
		
	}
	
};
		
int srm::Init()
{
	
	//Create the data texture
	glGenTextures(1, &texture);
	
	matrices = (srm::Matrix*)data;
	
	return 0;
	
}

void bug::Update()
{
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 3, 256, 0, GL_RGBA, GL_FLOAT, data);
	
	for (auto it = ::agents.begin(); it != ::agents.end(); ++it)
	{
		
		if (it->p_update == nullptr)	continue; //If this item is removed/disabled
		
		//Get the return value, and if it's nonzero, something's gone wrong
		//please dont copy please dont copy please dont copy please dont copy please dont copy please dont copy 
		//compiler will handle it 
		int return_value = it->p_update(*it->p_mat);
		
	}
	
}

int srm::ConstructBuffers(seg::Segment* passes)
{
	
	passes[0].attribs_head = passes[0].attribs;
	passes[0].indices_head = passes[0].indices;
	
	for (auto it = ::agents.begin(); it != ::agents.end(); it++) //hehe, it++ go BRRRRRR
	{
		
		exo::AddToBuffers( it->p_model, passes[0] );
		
	}
	
	passes[0].Update();
	
	return 0;
	
}

srm::Agent::Agent( uint32_t *ap_model, int (*ap_update)(srm::Matrix&), srm::Matrix* a_mat )
{
	
	p_model  = ap_model ;
	p_update = ap_update;
	p_mat	 = a_mat;
	
}

void srm::CreateAgent( uint32_t *ap_model, int (*ap_update)(Matrix&) )
{
	
	agents.emplace_back( ap_model, ap_update, matrices + first_free );
	
	mu_write(first_free, 1);
	
	for (; first_free <= SRM_MATRIX_COUNT; )
	{
		
		     if (matrix_usage[first_free == 256])	first_free += 8;
		else if (!mu_index(first_free))			break;
		else						first_free++;
		
	}
	
}



#include <iostream>
#include <cstring>
#include <vector>

#include "man.hpp"
#include "bug.hpp"
#include "ant.hpp"
#include "srm.hpp"

//Code made to manage ebtites
//LMAO EBTITIES

//TODO: can we please put this in a class? 
//I do not need to use multiple of these, but classes are just so much more convenient. 

#larva debug

//TODO: 1) this demo doesn't use matrices, just offsets and 2) there are only 4 components
float data[4 * 256] = { 0 };
srm::Matrix* matrices;

GLuint texture;

namespace {
	
	char matrix_usage[SRM_MATRIX_COUNT / 8] = { 0 };
	uint first_free = 0;
	srm::Data<srm::Agent> agents;
	
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
	
	//TODO: Make this use the new API. 
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 256, 0, GL_RGBA, GL_FLOAT, data);
	
	srm::Agent* agent = (srm::Agent*)::agents.p_data;
	for (int i = 0; i < agents.size(); i++)
	{
		
		if (agent->p_update != nullptr)
		{
			
			agent->p_update((void*)agent);
			
		}
		
		//waowie, ptr hacks!!!
		agent = (srm::Agent*)((char*)agent + agent->next);
		
	}
	
}

int srm::ConstructBuffers(seg::Segment* segment)
{
	
	segment->start = 0;
	segment->end   = 0;
	
	segment->attribs_head = segment->attribs;
	segment->indices_head = segment->indices;
	
	srm::Agent* agent = (srm::Agent*)::agents.p_data;
	
	for (int i = 0; i < agents.size(); i++)
	{
		
		if (agent->p_model == nullptr) break;
		exo::AddToBuffers( agent->p_model, segment );
		
		//waowie, ptr hacks!!!
		agent = (srm::Agent*)((char*)agent + agent->next);
		
	}
	
	segment->Update();
	
	return 0;
	
}

void srm::Create( int (*p_update)(void*), int (*p_init)(void*), uint32_t* ap_model, int width )
{
	
	int matrix_id = first_free;
	
	mu_write(first_free, 1);
	for (; first_free <= SRM_MATRIX_COUNT; )
	{
		
		//Optimization. 0b1111_1111 = 255 = skip the next 8
		     if (matrix_usage[first_free] == 255 && ((first_free & 7) == 0))	first_free += 8;
		else if (!mu_index(first_free))						break;
		else									first_free++;
		
	}
	
	srm::Agent* p_agent = agents.Request( width );
	
	p_agent->p_model	= ap_model;
	p_agent->p_update	= p_update;
	p_agent->p_mat		= matrices + matrix_id;
	p_agent->next		= sizeof(Agent) + width;
	
}

template <typename T>
T* srm::Data<T>::Request(int width)
{
	
	int start_head = head;
	head += width;
	
	//If the new byte usage is too wide for the buffer, we gotta resize
	if (head > alloced)
	{
		
		int next_alloced = alloced;
		while (head > next_alloced) next_alloced <<= 1; //Do it exponentially bc idk std::vector
		
		char* p_tmp = p_data;
		p_data = new char[next_alloced];
		
		memcpy(p_data, p_tmp, alloced);
		
		alloced = next_alloced;
		delete[] p_data;
		
	}
	
	//Agent constructor lmao
	T* p_agent = (T*)(p_data + start_head);
	m_size++;
	
	return p_agent;
	
}


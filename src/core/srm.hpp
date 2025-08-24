 
#ifndef SRM_H
#define SRM_H

#include "exo.hpp"
#include "seg.hpp"

#define SRM_MATRIX_COUNT 256
#define SRM_MATRIX_WIDTH 4

namespace srm {
	
	struct Matrix {
		
		float	x  , y  , z  , m03,
			m10, m11, m12, m13,
			m20, m21, m22, m23;
		
	};
	
	//Apperently "agent" is the "technical" term for an individual in a swarm. 
	//So uh, yeah. Entomology!!! Now with linked lists
	struct Agent; //Why do I need to do this to have ptrs to agent
	struct Agent {
		
		 Agent(uint32_t*, int (*)(Matrix&), Matrix*);
		
		uint32_t* p_model; // Don't delete pls
		uint8_t   pass;
		int	  (*p_update)(Matrix&);
		Matrix*	  p_mat;
		
		Agent *p_next, *p_last;
		
	};
	
	int Init();
	void CreateAgent(uint32_t*, int (*)(Matrix&));
	
	int Update();
	int ConstructBuffers(seg::Segment*);
	
};

#endif // SRM_H


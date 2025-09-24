 
#ifndef SRM_H
#define SRM_H

#include "exo.hpp"
#include "seg.hpp"

#define SRM_MATRIX_COUNT 256
#define SRM_MATRIX_WIDTH 1

#define DATA_SIZE sizeof(Agent) * 16

//I really hate modern c++'s look, but GOD it's so much better than having a pointer like this.
//TODO: Make this entire file modern c++
namespace srm {
	
	struct Matrix
	{
		
		float	x, y, z, m03;
		
	};
	
	//Apperently "agent" is the "technical" term for an individual in a swarm. 
	//So uh, yeah. Entomology!!! Now with linked lists
	struct Agent; //Why do I need to do this to have ptrs to agent
	struct Agent {
		
		 Agent(uint32_t*, int (*)(Matrix&), Matrix*);
		
		uint32_t *p_model; // Don't delete pls
		uint8_t   pass;
		uint16_t  next;
		int	(*p_update)(void*);
		Matrix   *p_mat;
		
	};
	
	template <typename T>
	class Data {
		
	public:
		 Data() { p_data = new char[DATA_SIZE]; };
		~Data() { delete[] p_data; };
		
		int size() { return m_size; }
		
		T* Request(int);
		
		char* p_data = nullptr;
		
		int alloced = DATA_SIZE;
		int head = 0;
		
	private:
		int m_size = 0;
		
	};
	
	int Init();
	
	void Create( int (*)(void*), int (*)(void*), uint32_t*, int );
	
	int Update();
	int ConstructBuffers(seg::Segment*);
	
};

#endif // SRM_H



#ifndef EXO_H
#define EXO_H

#include <vector>
#include <glad/glad.h>

#include "srm.hpp"
#include "seg.hpp"

namespace exo {
	
	int Init();
	int Cleanup();
	
	uint32_t* Model(int, int);
	
	typedef uint64_t MeshID;
	
	uint AddToBuffers( uint32_t*, seg::Segment& );
	
};

#endif //EXO_H


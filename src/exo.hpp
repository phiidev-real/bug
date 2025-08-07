
#ifndef EXO_H
#define EXO_H

#include <vector>
#include <glad/glad.h>

namespace exo {
	
	int Init();
	int Cleanup();
	
	uint32_t* Model(int, int);
	
	typedef uint64_t MeshID;
	
};

#endif //EXO_H


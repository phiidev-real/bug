
#include <GLFW/glfw3.h>

#larva def-xmac XMAC_KEYCODES 1 [ W,A,S,D ]

namespace ant {
	
#larva use-xmac XMAC_KEYCODES(code) bool code();
	
};



#include <GLFW/glfw3.h>

//Genuinely why did I decide this was a good idea. 
//None of this makes sense and will require being rebuilt. 
//Like I just found out about xmacros, but this is one of the worst applications. 
//How do you change hard-coded keybinds????? What was I thinking????

#larva def-xmac XMAC_KEYCODES 1 [ W,A,S,D ]

namespace ant {
	
#larva use-xmac XMAC_KEYCODES(code) bool code();
	
};


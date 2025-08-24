
#include "bug.hpp"
#include "ant.hpp"

#include <GLFW/glfw3.h>

namespace {
	
#larva use-xmac XMAC_KEYCODES(code) bool code;

};

#define TRACK_KEY(code)\
	     if (key == GLFW_KEY_##code && action == GLFW_RELEASE) code = 0;\
	else if (key == GLFW_KEY_##code && action == GLFW_PRESS  ) code = 1;

void bug::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	
#larva use-xmac XMAC_KEYCODES(code) TRACK_KEY(code)
	
}

#larva use-xmac XMAC_KEYCODES(code) bool ant::code() { return ::code; }


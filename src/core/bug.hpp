
#ifndef BUG_H
#define BUG_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

namespace bug {
	
	void Kill();
	void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void Update();
	
}

#endif // BUG_H


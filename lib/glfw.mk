include vars.mk

GLFW_CMAKE_OPTIONS = -DGLFW_BUILD_EXAMPLES=0 \
						-DGLFW_BUILD_TESTS=0

$(GLFW_TARGET): glfw
	cmake -Bbuild/glfw $(GLFW_CMAKE_OPTIONS) $<
	cmake --build build/glfw

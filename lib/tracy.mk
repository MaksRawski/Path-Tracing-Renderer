include vars.mk

# NOTE: this library is conditionally included in vars.mk
TRACY_CMAKE_OPTIONS = -DTRACY_STATIC=1 -DTRACY_ENABLE=1

# NOTE: also builds the server
$(TRACY_TARGET): $(TRACY_SERVER_PROFILER)
	cmake -B$(LIB_BUILD_DIR)/tracy $(TRACY_CMAKE_OPTIONS) tracy
	cmake --build $(LIB_BUILD_DIR)/tracy

# TODO: run this only if ncessary?
$(TRACY_SERVER_PROFILER): tracy/profiler
	cmake -B$(TRACY_SERVER_DIR) -DCMAKE_BUILD_TYPE=Release -S tracy/profiler
	cmake --build $(TRACY_SERVER_DIR) --parallel

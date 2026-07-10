include vars.mk

NFD_CMAKE_OPTIONS = -DNFD_BUILD_TESTS=OFF -DNFD_PORTAL=ON

$(NFD_TARGET): nativefiledialog-extended/
	cmake -B$(LIB_BUILD_DIR)/nfd $(NFD_CMAKE_OPTIONS) $<
	cmake --build $(LIB_BUILD_DIR)/nfd

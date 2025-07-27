#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1 and Mac OS X
#
# GLFW will be built from source in glfw/ directory
#

#CXX = g++
#CXX = clang++

EXE = build/caffeine
IMGUI_DIR = ./imgui
BUILD_DIR = build
GLFW_BUILD_DIR = build_glfw
GLFW_SOURCE_DIR = glfw
SOURCES = main.cpp
SOURCES += sleep_prevention/sleep_prevention.cpp
SOURCES += gl_context/gl_context.cpp
SOURCES += imgui-toggle/imgui_toggle.cpp
SOURCES += imgui-toggle/imgui_toggle_palette.cpp
SOURCES += imgui-toggle/imgui_toggle_presets.cpp
SOURCES += imgui-toggle/imgui_toggle_renderer.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
OBJS = $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
UNAME_S := $(shell uname -s)
LINUX_GL_LIBS = -lGL

CXXFLAGS = -std=c++11 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -I./sleep_prevention -I./gl_context -I./imgui-toggle
CXXFLAGS += -g -Wall -Wformat
LIBS =
LDFLAGS =

# GLFW build targets
GLFW_LIB = $(GLFW_BUILD_DIR)/src/libglfw3.a

##---------------------------------------------------------------------
## OPENGL ES
##---------------------------------------------------------------------

## This assumes a GL ES library available in the system, e.g. libGLESv2.so
# CXXFLAGS += -DIMGUI_IMPL_OPENGL_ES2
# LINUX_GL_LIBS = -lGLESv2

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS += $(LINUX_GL_LIBS)
	# Use our built GLFW via pkg-config for static linking
	LIBS += $(shell PKG_CONFIG_PATH=$(GLFW_BUILD_DIR)/src pkg-config --static --libs glfw3)
	CXXFLAGS += $(shell PKG_CONFIG_PATH=$(GLFW_BUILD_DIR)/src pkg-config --cflags glfw3)
	# Add flags for static linking
	LDFLAGS += -static -static-libgcc -static-libstdc++
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
	# Use our built GLFW via pkg-config for static linking
	LIBS += $(shell PKG_CONFIG_PATH=$(GLFW_BUILD_DIR)/src pkg-config --static --libs glfw3)
	CXXFLAGS += $(shell PKG_CONFIG_PATH=$(GLFW_BUILD_DIR)/src pkg-config --cflags glfw3)
	# Add flags for static linking (macOS doesn't support -static but we can link libstdc++ statically)
	LDFLAGS += -static-libgcc -static-libstdc++
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
	LIBS += -lgdi32 -lopengl32 -limm32
	# Use our built GLFW directly without pkg-config to avoid path issues
	# Force static linking and add all required Windows libraries for GLFW
	LIBS += -L$(GLFW_BUILD_DIR)/src -lglfw3 -lgdi32 -luser32 -lkernel32 -lshell32
	CXXFLAGS += -I$(GLFW_SOURCE_DIR)/include
	# Add flags for static linking and remove console window
	LDFLAGS += -static -static-libgcc -static-libstdc++ -mwindows
	CFLAGS = $(CXXFLAGS)
endif

##---------------------------------------------------------------------
## GLFW BUILD RULES
##---------------------------------------------------------------------

$(GLFW_LIB): | check_glfw_source
	@echo "Building GLFW from source..."
	@mkdir -p $(GLFW_BUILD_DIR)
	@cd $(GLFW_BUILD_DIR) && cmake ../$(GLFW_SOURCE_DIR) \
		-DCMAKE_BUILD_TYPE=Release \
		-DGLFW_BUILD_EXAMPLES=OFF \
		-DGLFW_BUILD_TESTS=OFF \
		-DGLFW_BUILD_DOCS=OFF \
		-DGLFW_INSTALL=OFF \
		-DBUILD_SHARED_LIBS=OFF \
		-DGLFW_BUILD_SHARED_LIBS=OFF
	@echo "GLFW build complete"

check_glfw_source:
	@if [ ! -d "$(GLFW_SOURCE_DIR)" ]; then \
		echo "Error: GLFW source directory '$(GLFW_SOURCE_DIR)' not found!"; \
		echo "Please ensure the GLFW source code is in the '$(GLFW_SOURCE_DIR)' directory"; \
		exit 1; \
	fi
	@if [ ! -f "$(GLFW_SOURCE_DIR)/CMakeLists.txt" ]; then \
		echo "Error: Invalid GLFW source directory. CMakeLists.txt not found in '$(GLFW_SOURCE_DIR)'"; \
		exit 1; \
	fi

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(IMGUI_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(IMGUI_DIR)/backends/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: sleep_prevention/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: gl_context/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: imgui-toggle/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)
	@echo "Executable: $(EXE)"

# Main executable depends on GLFW being built first
$(EXE): $(GLFW_LIB) $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) -o $@ $(OBJS) $(CXXFLAGS) $(LDFLAGS) $(LIBS)
	@echo "Cleaning up object files..."
	@rm -f $(OBJS)
	@echo "Build complete - single executable created without dependencies"

clean:
	rm -rf $(BUILD_DIR)

clean-all: clean
	rm -rf $(GLFW_BUILD_DIR)

rebuild-glfw:
	rm -rf $(GLFW_BUILD_DIR)
	$(MAKE) $(GLFW_LIB)

.PHONY: all clean clean-all rebuild-glfw check_glfw_source
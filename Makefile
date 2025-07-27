#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1 and Mac OS X
#
# You will need GLFW (http://www.glfw.org):
# Linux:
#   apt-get install libglfw-dev
# Mac OS X:
#   brew install glfw
# MSYS2:
#   pacman -S --noconfirm --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-glfw
#

#CXX = g++
#CXX = clang++

EXE = build/example_glfw_opengl3
IMGUI_DIR = ./imgui
BUILD_DIR = build
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
	LIBS += $(LINUX_GL_LIBS) `pkg-config --static --libs glfw3`

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
	LIBS += -L/usr/local/lib -L/opt/local/lib -L/opt/homebrew/lib
	#LIBS += -lglfw3
	LIBS += -lglfw

	CXXFLAGS += -I/usr/local/include -I/opt/local/include -I/opt/homebrew/include
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
	LIBS += -lglfw3 -lgdi32 -lopengl32 -limm32

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

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

$(EXE): $(OBJS)
	@mkdir -p $(BUILD_DIR)
ifeq ($(OS), Windows_NT)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS) -mwindows
	@cp glfw-3.4.bin.WIN64/lib-mingw-w64/glfw3.dll $(BUILD_DIR)/
	@echo "Copied glfw3.dll to build directory"
else
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)
endif


clean:
	rm -rf $(BUILD_DIR)

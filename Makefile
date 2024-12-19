# Compiler
CXX = g++

# Compiler and Linker flags
PKG_CONFIG = pkg-config
PKG_LIBS = opus alsa opencv4 libcamera libevent_pthreads libavcodec libavformat libswscale
CXXFLAGS = -I./inc \
           -I/usr/include/opencv4 \
           -I/usr/include/opencv4/opencv2 \
           -I/usr/include/libcamera \
		   -I/usr/include/aarch64-linux-gnu/ \
           `$(PKG_CONFIG) --cflags $(PKG_LIBS)` \
           -Wall -std=gnu++17
LDFLAGS = `$(PKG_CONFIG) --libs $(PKG_LIBS)` -lavcodec -lavformat -lavutil -lswscale

# Source and object files
SRC_DIR = src
OBJ_DIR = obj
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Executable name
TARGET = rtsp-server.out

# Default rule
all: $(TARGET)

# Link the object files to create the executable
$(TARGET): $(OBJS)
	@echo "Linking target: $@"
	$(CXX) -o $@ $^ $(LDFLAGS)

# Compile each .cpp file into an .o file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@echo "Compiling: $< -> $@"
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# Create the obj directory if it doesn't exist
$(OBJ_DIR):
	@echo "Creating directory: $@"
	mkdir -p $(OBJ_DIR)

# Clean up build files
clean:
	@echo "Cleaning up..."
	rm -rf $(OBJ_DIR) $(TARGET)

# Phony targets
.PHONY: all clean

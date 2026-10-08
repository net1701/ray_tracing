ARCH   ?= sm_20
CC     = g++ -std=c++0x
NVCC   = nvcc -arch=$(ARCH) -DSPHERES_SATURATE_COLORS #-DSPHERES_SIMPLE_COLORS -DIN_KERNEL_PRINTF
TARGET = bin/ray_tracing

CPP_SOURCES  := src/ConfigReader.cpp     \
                src/EasyBMP.cpp

CUDA_SOURCES := src/Config.cu       	 \
				src/main.cu              \
				src/Vec3d.cu             \
				src/Scene.cu
INCLUDES     := -I./include
LIBRARIES    := -lcurand
OBJS         := $(CPP_SOURCES:.cpp=.o) $(CUDA_SOURCES:.cu=.o)
OBJS         := $(subst src,obj,${OBJS})

default: $(TARGET)

all: default

obj/%.o: src/%.cu
	@mkdir -p obj
	$(NVCC) $(CPPFLAGS) -c $< -o $@ $(INCLUDES)

obj/%.o: src/%.cpp
	@mkdir -p obj
	$(CC) $(CPPFLAGS) -c $< -o $@ $(INCLUDES)

$(TARGET): $(OBJS)
	@mkdir -p bin
	$(NVCC) $(OBJS) -o $(TARGET) $(LIBRARIES)

clean:
	-rm -f obj/*.o
	-rm -f $(TARGET)

obj/main.o:	include/Log.h
obj/Config.o: include/ConfigReader.h include/Config.h
obj/Scene.o: include/Scene.h include/Sphere.h include/Light.h include/GpuScene.h


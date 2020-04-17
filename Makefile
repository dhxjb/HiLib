PROJECT_DIR = /home/xiangjb/project
TOOLCHAIN_DIR = $(PROJECT_DIR)/toolchains/buildroot_host
COMPILE_OPTS = -I./include -I$(PROJECT_DIR)/opensource/alsa-lib-1.1.7/include
LINK_OPTS = -lasound -pthread -L.

CC = $(TOOLCHAIN_DIR)/bin/arm-linux-gnueabihf-gcc
C_FLAGS = $(COMPILE_OPTS)
CXX = $(TOOLCHAIN_DIR)/bin/arm-linux-gnueabihf-g++
CXX_FLAGS = $(COMPILE_OPTS) -std=c++11 -Wall
AR = $(TOOLCHAIN_DIR)/bin/arm-linux-gnueabihf-ar

HC_LIB = libhicreation.a
ALSA_MODULE = alsa

all: $(ALSA_MODULE)-test $(HC_LIB)
.PHONY: all 

CXX-src := $(wildcard src/*.cpp)
# CXX-objs = alsa_device.o alsa-test.o
CXX-objs := $(CXX-src:%.cpp=%.o)
# CXX-objs = $(patsubst %.cpp, %.o, $(CXX-src))
hclib-objs := $(filter-out %-test.cpp, $(CXX-objs))

$(ALSA_MODULE)-test: src/$(ALSA_MODULE)-test.o $(HC_LIB)
	$(CXX) -o $@ $^ $(LINK_OPTS)

$(HC_LIB): $(hclib-objs)
	$(AR) rv $@  $^

%.o: %.cpp
	$(CXX) -c $(CXX_FLAGS) $< -o $@

.PHONY: clean
	rm -f *.o



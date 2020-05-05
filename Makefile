CURRENT_DIR = $(shell pwd)
TOOLCHAIN_DIR = /home/xiangjb/project/toolchains/buildroot_host
COMPILE_OPTS = -I$(CURRENT_DIR)/include \
	-I$(CURRENT_DIR)/third_party/alsa/alsa-lib-1.1.7/include \
	-I$(CURRENT_DIR)/third_party/sdl/SDL2-2.0.9/include \
	-I$(CURRENT_DIR)/third_party/ffmpeg/ffmpeg-3.4.6 \
	-I$(CURRENT_DIR)/third_party/i2c-tools/i2c-tools-4.1/include 
LINK_OPTS = -pthread -lasound -lSDL2 -lavformat -lavcodec -lswresample -lswscale -lavutil \
			-li2c -L.

CC = $(TOOLCHAIN_DIR)/bin/arm-linux-gnueabihf-gcc
C_FLAGS = $(COMPILE_OPTS)
CXX = $(TOOLCHAIN_DIR)/bin/arm-linux-gnueabihf-g++
CXX_FLAGS = $(COMPILE_OPTS) -std=c++11 -Wall
AR = $(TOOLCHAIN_DIR)/bin/arm-linux-gnueabihf-ar

HC_LIB = libhicreation.a
ALSA_MODULE = alsa
RADIO_MODULE = radio
SDL_MODULE = sdl
FF_MODULE = ff
I2C_MODULE = i2c
TEST_MODULES := $(ALSA_MODULE)-test $(RADIO_MODULE)-test $(SDL_MODULE)-test $(FF_MODULE)-test \
		$(I2C_MODULE)-test


all: $(HC_LIB) $(TEST_MODULES)
.PHONY: all

hclib-src := $(wildcard src/*.cpp)
test-src := $(wildcard test/*.cpp)
# hclib-objs := $(filter-out *-test.cpp, $(CXX-objs))
# CXX-objs = $(patsubst %.cpp, %.o, $(CXX-src))
hclib-objs := $(hclib-src:src/%.cpp=obj/%.o)
test-objs := $(test-src:test/%.cpp=obj/%.o)

# $(ALSA_MODULE)-test: test/$(ALSA_MODULE)-test.o $(HC_LIB)
#	$(CXX) -o $@ $^ $(LINK_OPTS)

# $(RADIO_MODULE)-test: test/$(RADIO_MODULE)-test.o $(HC_LIB)
#	$(CXX) -o $@ $^ $(LINK_OPTS)

#$(RADIO_MODULE)-test: test/$(RADIO_MODULE)-test.o $(HC_LIB)
#	$(CXX) -o $@ $^ $(LINK_OPTS)

$(TEST_MODULES): %: obj/%.o $(HC_LIB) 
	$(CXX) -o $@ $^ $(LINK_OPTS)

$(HC_LIB): $(hclib-objs)
	$(AR) rv $@  $^

obj/%.o: src/%.cpp
	$(CXX) -c $(CXX_FLAGS) $< -o $@

obj/%.o: test/%.cpp
	$(CXX) -c $(CXX_FLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f obj/*.o $(HC_LIB) $(TEST_MODULES)



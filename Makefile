CURRENT_DIR = $(shell pwd)
TOOLCHAIN_DIR = $(CURRENT_DIR)/../toolchains/buildroot_host
COMPILE_OPTS = -I$(CURRENT_DIR)/include \
	-I$(CURRENT_DIR)/third_party/alsa/alsa-lib-1.1.7/include
LINK_OPTS = -lasound -pthread -L.

CC = $(TOOLCHAIN_DIR)/bin/arm-linux-gnueabihf-gcc
C_FLAGS = $(COMPILE_OPTS)
CXX = $(TOOLCHAIN_DIR)/bin/arm-linux-gnueabihf-g++
CXX_FLAGS = $(COMPILE_OPTS) -std=c++11 -Wall
AR = $(TOOLCHAIN_DIR)/bin/arm-linux-gnueabihf-ar

HC_LIB = libhicreation.a
ALSA_MODULE = alsa
RADIO_MODULE = radio

all: $(HC_LIB)
.PHONY: all
test: $(ALSA_MODULE)-test $(RADIO_MODULE)-test
.PHONY: test

hclib-src := $(wildcard src/*.cpp)
test-src := $(wildcard test/*.cpp)
# hclib-objs := $(filter-out *-test.cpp, $(CXX-objs))
# CXX-objs = $(patsubst %.cpp, %.o, $(CXX-src))
hclib-objs := $(hclib-src:%.cpp=%.o)
test-objs := $(test-src:%.cpp=%.o)

$(ALSA_MODULE)-test: test/$(ALSA_MODULE)-test.o $(HC_LIB)
	$(CXX) -o $@ $^ $(LINK_OPTS)

$(RADIO_MODULE)-test: test/$(RADIO_MODULE)-test.o $(HC_LIB)
	$(CXX) -o $@ $^ $(LINK_OPTS)

$(HC_LIB): $(hclib-objs)
	$(AR) rv $@  $^

%.o: %.cpp
	$(CXX) -c $(CXX_FLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f src/*.o test/*.o $(HC_LIB)



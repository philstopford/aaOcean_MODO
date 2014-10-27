# Copyright (c) 2008-2013 Luxology LLC

LXSDK = ../LXSDK
LXSDK_INC = $(LXSDK)/include
LXSDK_BUILD = $(LXSDK)/samples/Makefiles/build
SRC_DIR=aaOcean
AAO_INC = dependencies/aaOcean/src
HELPERS_INC = dependencies/helpers/
OBJ_DIR=Linux/obj
TARGET_DIR=Linux/build
MYCXX = g++
LINK = g++
CXXFLAGS = -std=c++0x  -DMODO701 -DFFTWSINGLETHREADED -g -c -I$(LXSDK_INC) -I$(AAO_INC) -I$(HELPERS_INC) -fPIC -m64 -msse
LDFLAGS = -L$(LXSDK_BUILD) -L/usr/lib -lcommon -lfftw3f -lfftw3f_threads -lpthread -shared

OBJS = $(AAO_INC)/aaOceanClass.o $(OBJ_DIR)/aaOcean_init.o $(OBJ_DIR)/aaOcean_texture.o $(OBJ_DIR)/aaOcean_chanmod.o $(OBJ_DIR)/aaOcean_deformer.o $(OBJ_DIR)/aaOcean_command.o
TARGET = $(TARGET_DIR)/aaocean.lx

all: $(TARGET)

lxsdk:
	cd $(LXSDK)/samples/Makefiles/common; make

$(OBJ_DIR):
	mkdir $@

.PRECIOUS :$(OBJ_DIR)/%.o 
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(MYCXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJ_DIR) lxsdk $(OBJS)
	$(LINK) -o $@ $(OBJS) $(LDFLAGS)

clean:
	cd $(LXSDK)/samples/Makefiles/common; make clean
	rm -rf $(TARGET) $(OBJ_DIR)

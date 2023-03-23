# Makefile - Build configuration file.
# Copyright (C) 2023  Makonede
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.


ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif
include $(DEVKITPRO)/libnx/switch_rules
TARGET := linkcannon
BUILD_DIR := .
ROOT_DIR := ..
LIB_DIR := $(ROOT_DIR)/lib
SOURCE_DIR := $(ROOT_DIR)/src/
CONFIG_DIR := $(ROOT_DIR)/config
ALL_SOURCES_DIRS := $(shell find $(SOURCE_DIR) -type d)
LIBDIRS := $(PORTLIBS) $(LIBNX)
ALL_INCLUDE_DIRS := $(SOURCE_DIR) $(LIB_DIR)/nnheaders/include $(LIB_DIR)/sead/include $(LIB_DIR)/botw/src $(LIB_DIR)/LinkCannon $(LIB_DIR)/botw/lib/agl/include
VPATH := $(foreach dir,$(ALL_SOURCES_DIRS),$(CURDIR)/$(dir))
LDSCRIPT := $(CONFIG_DIR)/symbols.ld
LINKER_VERSION_SCRIPT := $(CONFIG_DIR)/exported
SWITCH_SPECS := $(CONFIG_DIR)/switch.specs
BOTW_DIR=$(LIB_DIR)/botw/src
SEAD_DIR=$(LIB_DIR)/sead/modules/src
CFILES := $(foreach dir,$(ALL_SOURCES_DIRS),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(ALL_SOURCES_DIRS),$(notdir $(wildcard $(dir)/*.cpp))) $(BOTW_DIR)/KingSystem/Event/evtMetadata.cpp $(SEAD_DIR)/prim/seadSafeString.cpp $(BOTW_DIR)/KingSystem/Event/evtOrderParam.cpp $(BOTW_DIR)/KingSystem/ActorSystem/actBaseProcLink.cpp $(SEAD_DIR)/basis/seadNew.cpp $(SEAD_DIR)/heap/seadHeapMgr.cpp $(SEAD_DIR)/heap/seadArena.cpp $(SEAD_DIR)/thread/nin/seadCriticalSectionNin.cpp $(SEAD_DIR)/container/seadPtrArray.cpp $(SEAD_DIR)/heap/seadDisposer.cpp $(SEAD_DIR)/heap/seadHeap.cpp $(SEAD_DIR)/container/seadListImpl.cpp
SFILES := $(foreach dir,$(ALL_SOURCES_DIRS),$(notdir $(wildcard $(dir)/*.s)))
OFILES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
DFILES := $(OFILES:.o=.d)
APP_JSON := $(CONFIG_DIR)/app.json
LD := $(CXX)
INCLUDE := $(foreach dir,$(ALL_INCLUDE_DIRS),-I$(CURDIR)/$(dir)) $(foreach dir,$(LIBDIRS),-I$(dir)/include)
DEFINES := -D__SWITCH__ -DSWITCH -DNNSDK $(EXTRA_DEFINES)
ARCH := -march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIC -ftls-model=local-exec
CFLAGS := -g -Wall -ffunction-sections -O3 $(ARCH) $(DEFINES) $(INCLUDE) $(BOTW_VERSION_DEFINES)
CXXFLAGS := $(CFLAGS) -fno-rtti -fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables -enable-libstdcxx-allocator=new -fpermissive -std=c++2a
ASFLAGS := -g $(ARCH)
LDFLAGS := -specs=$(SWITCH_SPECS) -g $(ARCH) -Wl,-Map,$(TARGET).map -Wl,--version-script=$(LINKER_VERSION_SCRIPT) -Wl,-init=linkcannon_init -Wl,-fini=linkcannon_fini -Wl,--export-dynamic -nodefaultlibs
LIBS := -lgcc -lstdc++ -u malloc
LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)
DEPSDIR ?= .
.PHONY: all
all: $(TARGET).nso app.npdm
$(TARGET).elf: $(OFILES) $(SWITCH_SPECS)
app.npdm: $(APP_JSON)
	npdmtool $(APP_JSON) $@ 2> /dev/null
-include $(DFILES)

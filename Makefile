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
ALL_INCLUDE_DIRS := $(SOURCE_DIR) $(LIB_DIR)/sead/include $(LIB_DIR)/LinkCannon $(LIB_DIR)/nnheaders/include
VPATH := $(foreach dir,$(ALL_SOURCES_DIRS),$(CURDIR)/$(dir))
LDSCRIPT := $(CONFIG_DIR)/symbols.ld
LINKER_VERSION_SCRIPT := $(CONFIG_DIR)/exported
SWITCH_SPECS := $(CONFIG_DIR)/switch.specs
CFILES := $(foreach dir,$(ALL_SOURCES_DIRS),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(ALL_SOURCES_DIRS),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES := $(foreach dir,$(ALL_SOURCES_DIRS),$(notdir $(wildcard $(dir)/*.s)))
OFILES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
DFILES := $(OFILES:.o=.d)
APP_JSON := $(CONFIG_DIR)/app.json
LD := $(CXX)
INCLUDE := $(foreach dir,$(ALL_INCLUDE_DIRS),-I$(CURDIR)/$(dir)) $(foreach dir,$(LIBDIRS),-I$(dir)/include)
DEFINES := -D__SWITCH__ -DSWITCH -DNNSDK $(EXTRA_DEFINES)
ARCH := -march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIC -ftls-model=local-exec
CFLAGS := -g -Wall -ffunction-sections -O3 $(ARCH) $(DEFINES) $(INCLUDE)
CXXFLAGS := $(CFLAGS) -fno-rtti -fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables -enable-libstdcxx-allocator=new -fpermissive -std=c++2a
ASFLAGS := -g $(ARCH)
LDFLAGS := -specs=$(SWITCH_SPECS) -g $(ARCH) -Wl,-Map,$(TARGET).map -Wl,--version-script=$(LINKER_VERSION_SCRIPT) -Wl,-init=init -Wl,-fini=fini -Wl,--export-dynamic -nodefaultlibs
LIBS := -lgcc -lstdc++ -u malloc
LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)
DEPSDIR ?= .
.PHONY: all
all: $(TARGET).nso app.npdm
$(TARGET).elf: $(OFILES) $(SWITCH_SPECS)
app.npdm: $(APP_JSON)
	npdmtool $(APP_JSON) $@ 2> /dev/null
-include $(DFILES)

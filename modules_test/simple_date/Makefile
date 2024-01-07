LIB_NAME := simple_date
VERSION_MAJOR := 0
VERSION_MINOR := 0
CXX := g++
COMPILE_FLAGS := -std=c++20 -Wall -Wextra -g
RELEASE_COMPILE_FLAGS := -O2 -DNDEBUG
DEBUG_COMPILE_FLAGS := -Og -DDEBUG
LINK_FLAGS :=
RELEASE_LINK_FLAGS :=
DEBUG_LINK_FLAGS :=
SOURCE_FILE_EXT := .cpp
HEADER_FILE_EXT := .hpp
SOURCE_DIRS := ./source
# TODO FIXME vvv
INCLUDE_DIRS := ./include ../../include ../../submodules/libconfigfile/include
SUBMODULE_DIR := ./submodules
LIBRARIES :=
INSTALL_PATH := /usr/local

##########

SHELL := /bin/bash

.SUFFIXES:

export BUILD_DIR := ./build

STATIC_LIB_NAME := $(LIB_NAME).a
SHARED_LIB_NAME_LINKER_NAME := $(LIB_NAME).so
SHARED_LIB_NAME_SONAME := $(SHARED_LIB_NAME_LINKER_NAME).$(VERSION_MAJOR)
SHARED_LIB_NAME := $(SHARED_LIB_NAME_SONAME).$(VERSION_MINOR)
LIB_INSTALL_PATH := $(INSTALL_PATH)/lib
HEADER_INSTALL_PATH := $(INSTALL_PATH)/include

COMPILE_FLAGS += -fPIC
LINK_FLAGS += -shared -Wl,-soname,$(SHARED_LIB_NAME_SONAME)

INCLUDE_DIRS += $(wildcard $(SUBMODULE_DIR)/*/include)
LINK_FLAGS += $(addprefix -l, $(LIBRARIES))
SUBMODULE_OBJECTS := $(wildcard $(SUBMODULE_DIR)/*/build/*.a)
INCLUDE_FLAGS := $(addprefix -I, $(shell find $(INCLUDE_DIRS) -type d))

export CPPFLAGS := $(INCLUDE_FLAGS) -MMD -MP

release: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) $(RELEASE_COMPILE_FLAGS)
release: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS) $(RELEASE_LINK_FLAGS)
debug: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) $(DEBUG_COMPILE_FLAGS)
debug: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS) $(DEBUG_LINK_FLAGS)

SOURCES := $(shell find $(SOURCE_DIRS) -type f -name '*$(SOURCE_FILE_EXT)')
OBJECTS := $(SOURCES:%=$(BUILD_DIR)/%.o)
DEPENDENCIES := $(OBJECTS:.o=.d)

.PHONY: release
release:
	@$(MAKE) all --no-print-directory

.PHONY: debug
debug:
	@$(MAKE) all --no-print-directory

.PHONY: all
all: $(BUILD_DIR)/$(SHARED_LIB_NAME) $(BUILD_DIR)/$(STATIC_LIB_NAME)


$(BUILD_DIR)/$(SHARED_LIB_NAME): $(OBJECTS)
	$(CXX) $(OBJECTS) $(SUBMODULE_OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/$(STATIC_LIB_NAME): $(OBJECTS)
	ar rcs $@ $(OBJECTS) $(SUBMODULE_OBJECTS)

$(BUILD_DIR)/%$(SOURCE_FILE_EXT).o: %$(SOURCE_FILE_EXT)
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

-include $(DEPENDENCIES)

.PHONY: install
install:
	@install -v -Dm755 $(BUILD_DIR)/$(SHARED_LIB_NAME) -t $(LIB_INSTALL_PATH)/
	@ln -v -s $(LIB_INSTALL_PATH)/$(SHARED_LIB_NAME) $(LIB_INSTALL_PATH)/$(SHARED_LIB_NAME_SONAME)
	@ln -v -s $(LIB_INSTALL_PATH)/$(SHARED_LIB_NAME_SONAME) $(LIB_INSTALL_PATH)/$(SHARED_LIB_NAME_LINKER_NAME)
	@install -v -Dm644 $(BUILD_DIR)/$(STATIC_LIB_NAME) -t $(LIB_INSTALL_PATH)/
	@install -v -Dm644 $(foreach INCLUDE_DIR,$(INCLUDE_DIRS),$(wildcard $(INCLUDE_DIR)/*)) -t $(HEADER_INSTALL_PATH)/$(LIB_NAME)
	@ldconfig -v

.PHONY: uninstall
uninstall:
	@rm -v $(LIB_INSTALL_PATH)/$(SHARED_LIB_NAME_LINKER_NAME)
	@rm -v $(LIB_INSTALL_PATH)/$(SHARED_LIB_NAME_SONAME)
	@rm -v $(LIB_INSTALL_PATH)/$(SHARED_LIB_NAME)
	@rm -v $(LIB_INSTALL_PATH)/$(STATIC_LIB_NAME)
	@rm -v -r $(HEADER_INSTALL_PATH)/$(LIB_NAME)

.PHONY: clean
clean:
	@rm -v -r $(BUILD_DIR)

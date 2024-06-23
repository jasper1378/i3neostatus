BIN_NAME := i3neostatus
VERSION_MAJOR := 0
VERSION_MINOR := 0
# CXX := g++
CXX := clang++
COMPILE_FLAGS := -std=c++20 -Wall -Wextra
RELEASE_COMPILE_FLAGS := -g -O2 -DNDEBUG
DEBUG_COMPILE_FLAGS := -g3 -O0 -DDEBUG
LINK_FLAGS := -rdynamic
RELEASE_LINK_FLAGS :=
DEBUG_LINK_FLAGS :=
LIBRARIES :=
SOURCE_FILE_EXT := .cpp
HEADER_FILE_EXT := .hpp
SOURCE_DIRS := ./source
INCLUDE_DIRS := ./include
INCLUDE_INSTALL_DIRS := ./include_install
SUBMODULE_DIR := ./submodules
INSTALL_PATH := /usr/local

##########

COMPILE_FLAGS += -DMAKE_INSTALL_PATH="\"$(INSTALL_PATH)\""

##########

SHELL := /bin/bash

.SUFFIXES:

export BUILD_DIR := ./build

BIN_INSTALL_PATH := $(INSTALL_PATH)/bin
HEADER_INSTALL_PATH := $(INSTALL_PATH)/include

LINK_FLAGS += $(addprefix -l, $(LIBRARIES))
SUBMODULE_OBJECTS := $(wildcard $(SUBMODULE_DIR)/*/build/*.a)
INCLUDE_FLAGS := $(addprefix -I, $(shell find $(INCLUDE_DIRS) -type d))

export CPPFLAGS := $(INCLUDE_FLAGS) -MMD -MP

RELEASE_CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) $(RELEASE_COMPILE_FLAGS)
RELEASE_LDFLAGS := $(LDFLAGS) $(LINK_FLAGS) $(RELEASE_LINK_FLAGS)
DEBUG_CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) $(DEBUG_COMPILE_FLAGS)
DEBUG_LDFLAGS := $(LDFLAGS) $(LINK_FLAGS) $(DEBUG_LINK_FLAGS)

all: CXXFLAGS := $(RELEASE_CXXFLAGS)
all: LDFLAGS := $(RELEASE_LDFLAGS)
release: CXXFLAGS := $(RELEASE_CXXFLAGS)
release: LDFLAGS := $(RELEASE_LDFLAGS)
debug: CXXFLAGS := $(DEBUG_CXXFLAGS)
debug: LDFLAGS := $(DEBUG_LDFLAGS)

SOURCES := $(shell find $(SOURCE_DIRS) -type f -name '*$(SOURCE_FILE_EXT)')
OBJECTS := $(SOURCES:%=$(BUILD_DIR)/%.o)
DEPENDENCIES := $(OBJECTS:.o=.d)

.PHONY: release
release: _all

.PHONY: debug
debug: _all

.PHONY: all
all: _all

.PHONY: _all
_all: _build_executable

.PHONY: _build_executable
_build_executable : $(BUILD_DIR)/$(BIN_NAME)

$(BUILD_DIR)/$(BIN_NAME): $(OBJECTS)
	$(CXX) $(OBJECTS) $(SUBMODULE_OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%$(SOURCE_FILE_EXT).o: %$(SOURCE_FILE_EXT)
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

-include $(DEPENDENCIES)

.PHONY: install
install: _install_executable _install_headers

.PHONY: _install_executable
_install_executable:
	@install -v -Dm755 $(BUILD_DIR)/$(BIN_NAME) -t $(BIN_INSTALL_PATH)/

.PHONY: _install_headers
_install_headers:
	# @install -v -Dm644 $(foreach INCLUDE_DIR,$(INCLUDE_INSTALL_DIRS),$(wildcard $(INCLUDE_DIR)/*)) -t $(HEADER_INSTALL_PATH)/$(BIN_NAME)
	$(foreach INCLUDE_DIR,$(INCLUDE_INSTALL_DIRS), cd $(INCLUDE_DIR); find . -type f,l -exec install -v -Dm644 {} -T $(HEADER_INSTALL_PATH)/$(BIN_NAME)/{} \;;)

.PHONY: uninstall
uninstall: _uninstall_executable _uninstall_headers

.PHONY: _uninstall_executable
_uninstall_executable:
	@rm -v $(BIN_INSTALL_PATH)/$(BIN_NAME)

.PHONY: _uninstall_headers
_uninstall_headers:
	@rm -v -r $(HEADER_INSTALL_PATH)/$(BIN_NAME)

.PHONY: clean
clean:
	@rm -v -r $(BUILD_DIR)

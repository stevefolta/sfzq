PLUGIN := sfzq.clap
SOURCES := sfzq.cpp
SOURCES += SFZQPlugin.cpp
SFZ_DIR := SFZ
SFZ_SOURCES += SFZRegion.cpp SFZSample.cpp
SFZ_SOURCES += WAVReader.cpp SampleBuffer.cpp RIFF.cpp
SOURCES += $(foreach source,$(SFZ_SOURCES),$(SFZ_DIR)/$(source))
FRAMEWORK_DIR := CLAPFramework
FRAMEWORK_SOURCES += CLAPPlugin.cpp
FRAMEWORK_SOURCES += CLAPPosixFDExtension.cpp CLAPCairoGUIExtension.cpp
FRAMEWORK_SOURCES += CLAPAudioPortsExtension.cpp CLAPNotePortsExtension.cpp
FRAMEWORK_SOURCES += CLAPParamsExtension.cpp CLAPStateExtension.cpp CLAPTimerSupportExtension.cpp
FRAMEWORK_SOURCES += CLAPStream.cpp
SOURCES += $(foreach source,$(FRAMEWORK_SOURCES),$(FRAMEWORK_DIR)/$(source))
WIDGETS_DIR := Widgets
WIDGET_SOURCES += Widget.cpp Button.cpp Scrollbar.cpp FileList.cpp FileChooser.cpp
WIDGET_SOURCES += Label.cpp ProgressBar.cpp
WIDGET_SOURCES += TimeSeconds.cpp
SOURCES += $(foreach source,$(WIDGET_SOURCES),$(WIDGETS_DIR)/$(source))
LIBRARIES += X11 cairo
SUBDIRS += $(SFZ_DIR) $(FRAMEWORK_DIR) $(WIDGETS_DIR)
CFLAGS += --no-exceptions --no-rtti

OBJECTS_DIR := objects
CFLAGS += -Wall
LINK_FLAGS += -shared

-include Makefile.local

OBJECTS = $(foreach source,$(SOURCES),$(OBJECTS_DIR)/$(source:.cpp=.o))
OBJECTS_SUBDIRS = $(foreach dir,$(SUBDIRS),$(OBJECTS_DIR)/$(dir))

ifndef VERBOSE_MAKE
	QUIET := @
endif

all: $(PLUGIN)

CPP := g++
CFLAGS += -MMD
CFLAGS += -g
CFLAGS += $(foreach dir,$(SUBDIRS),-I$(dir))
CFLAGS += $(foreach switch,$(DEFINES),-D$(switch))
CFLAGS += $(foreach switch,$(SWITCHES),-D$(switch))
LINK_FLAGS += -g
LINK_FLAGS += $(foreach lib,$(LIBRARIES),-l$(lib))

$(OBJECTS_DIR)/%.o: %.cpp
	@echo Compiling $<...
	$(QUIET) $(CPP) -c $< -g $(CFLAGS) -o $@

$(OBJECTS): | $(OBJECTS_DIR)

$(PLUGIN): $(OBJECTS)
	@echo "Linking $@..."
	$(QUIET) $(CPP) $(filter-out $(OBJECTS_DIR),$^) $(LINK_FLAGS) -o $@

$(OBJECTS_DIR):
	@echo "Making $@..."
	$(QUIET) mkdir -p $(OBJECTS_DIR) $(OBJECTS_SUBDIRS)

-include $(OBJECTS_DIR)/*.d
-include $(OBJECTS_DIR)/*/*.d


.PHONY: clean
clean:
	rm -rf $(OBJECTS_DIR)

.PHONY: tags
tags:
	ctags -R .

.PHONY: edit-all
edit-all:
	@ $(EDITOR) $(filter-out sfzq.h,$(foreach source,$(SOURCES),$(source:.cpp=.h) $(source)))

.PHONY: validate
validate:
	$(QUIET) clap-validator validate --only-failed --no-parallel $(PLUGIN)

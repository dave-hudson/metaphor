CC := g++
CFLAGS := -std=c++17 -O2 -Wall
LDFLAGS := -std=c++17
RM := rm

#
# Define our target app.
#
APP := build/m6rc

#
# Define the source files for our build.
#
METAPHORC_SRCS :=

#
# Pick up source files.
#
include src/Makefile.mk

#
# Create a list of object files from source files.
#
METAPHORC_OBJS := $(patsubst src/%.cpp,build/obj/%.o,$(METAPHORC_SRCS))

BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj

$(OBJ_DIR)/%.o : src/%.cpp
	$(CC) $(CFLAGS) -MD -c $< -o $@

.PHONY: all

all: $(APP)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR) $(OBJ_DIR)/m6rc

# Include dependency files
-include $(METAPHORC_OBJS:.o=.d)

$(APP): $(OBJ_DIR) $(METAPHORC_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(METAPHORC_OBJS)

.PHONY: test

test: all
	python3 src/testrun/testrun.py test/test.json

.PHONY: clean

clean:
	$(RM) -f $(APP) $(METAPHORC_OBJS) $(METAPHORC_OBJS:.o=.d)

.PHONY: realclean

realclean: clean
	$(RM) -f *~ src/*~
	$(RM) -fr $(BUILD_DIR)

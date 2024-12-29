SDK_DIR := .

OBJ_OUT_DIR := ../build
SDK_OUT_DIR := $(OBJ_OUT_DIR)/src

$(shell mkdir -p $(OBJ_OUT_DIR))
$(shell mkdir -p $(SDK_OUT_DIR))

SDK_SRC := $(wildcard $(SDK_DIR)/*.cpp)

SDK_OBJ := $(patsubst $(SDK_DIR)/%.cpp, $(SDK_OUT_DIR)/%.o, $(SDK_SRC))

PCH := $(SDK_OUT_DIR)/pch.h.gch

PCH_SRC := $(SDK_DIR)/pch.h

CC := g++

CXXFLAGS ?= -O2

.PHONY: all
all: $(PCH) $(SDK_OBJ)

$(PCH): $(PCH_SRC)
	$(CC) -c $(CXXFLAGS) $(LDFLAGS) $(PCH_SRC) -o $(PCH)

$(SDK_OUT_DIR)/%.o: $(SDK_DIR)/%.cpp $(PCH)
	$(CC) -c $(CXXFLAGS) $< -o $@ -include $(PCH_SRC)

.PHONY: clean
	$(RM) $(SDK_OUT_DIR)
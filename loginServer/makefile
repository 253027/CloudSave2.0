ROOT_DIR := ..

SRC := $(wildcard *.cpp)

OBJ_DIR := $(ROOT_DIR)/build/loginServer
$(shell mkdir -p $(OBJ_DIR))

OBJ := $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRC))

SDK_OBJ := $(wildcard $(ROOT_DIR)/build/src/*.o)

CC := g++
CXXFLAGS ?= -O2 -MMD -MP
LDFLAGS := -L$(ROOT_DIR)/lib
LDLIBS := -lcrypt -lmysqlclient -lspdlog

loginServer: $(OBJ) $(SDK_OBJ)
	$(CC) $^ $(CXXFLAGS) -o $@ $(LDFLAGS) $(LDLIBS)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) -c $(CXXFLAGS) $< -o $@

-include $(OBJ:.o=.d)

.PHONY: clean
clean:
	$(RM) $(OBJ_DIR)/*.o $(OBJ_DIR)/*.d loginServer
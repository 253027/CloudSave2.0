ROOT_DIR := ..

SRC := $(wildcard *.cpp)

OBJ_DIR := $(ROOT_DIR)/build/proxyServer
$(shell mkdir -p $(OBJ_DIR))

OBJ := $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRC))

SDK_OBJ := $(wildcard $(ROOT_DIR)/build/src/*.o)

CC := g++
CXXFLAGS ?= -O2 -MMD -MP
CPPFLAGS := -I$(ROOT_DIR)/protobuf/include
LDFLAGS := -L$(ROOT_DIR)/lib -L$(ROOT_DIR)/protobuf/lib
LDLIBS := -lcrypt -lmysqlclient -lspdlog -lprotobuf

proxyServer: $(OBJ) $(SDK_OBJ)
	$(CC) $^ $(CXXFLAGS) $(CPPFLAGS) -o $@ $(LDFLAGS) $(LDLIBS)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@

-include $(OBJ:.o=.d)

.PHONY: clean
clean:
	$(RM) $(OBJ_DIR)/*.o $(OBJ_DIR)/*.d proxyServer
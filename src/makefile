# 源文件目录
SDK_DIR := .
BASE_DIR := $(SDK_DIR)/base
PROTOCOL_DIR := $(SDK_DIR)/protocal

# 对象文件输出目录
OBJ_OUT_DIR := ../build
SDK_OUT_DIR := $(OBJ_OUT_DIR)/src

# 创建输出目录
$(shell mkdir -p $(OBJ_OUT_DIR))
$(shell mkdir -p $(SDK_OUT_DIR))

# 源文件
SDK_SRC := $(wildcard $(SDK_DIR)/*.cpp)
BASE_SRC := $(wildcard $(BASE_DIR)/*.cpp)
PROTOCOL_SRC := $(wildcard $(PROTOCOL_DIR)/*.cpp)

# 所有源文件
ALL_SRC := $(SDK_SRC) $(BASE_SRC) $(PROTOCOL_SRC)

# 对象文件
SDK_OBJ := $(patsubst $(SDK_DIR)/%.cpp, $(SDK_OUT_DIR)/%.o, $(SDK_SRC))
BASE_OBJ := $(patsubst $(BASE_DIR)/%.cpp, $(SDK_OUT_DIR)/%.o, $(BASE_SRC))
PROTOCOL_OBJ := $(patsubst $(PROTOCOL_DIR)/%.cpp, $(SDK_OUT_DIR)/%.o, $(PROTOCOL_SRC))

# 所有对象文件
ALL_OBJ := $(SDK_OBJ) $(BASE_OBJ) $(PROTOCOL_OBJ)

# 预编译头文件
PCH := $(SDK_OUT_DIR)/pch.h.gch
PCH_SRC := $(BASE_DIR)/pch.h

# 编译器和编译选项
CC := g++
CXXFLAGS ?= -O2 -MMD -MP  # 添加生成依赖文件的选项
CPPFLAGS := -I../protobuf/include

.PHONY: all
all: $(PCH) $(ALL_OBJ)

# 生成预编译头文件
$(PCH): $(PCH_SRC)
	$(CC) -c $(CXXFLAGS) $(LDFLAGS) $(PCH_SRC) -o $(PCH)

# 生成对象文件，包含预编译头文件
$(SDK_OUT_DIR)/%.o: $(SDK_DIR)/%.cpp $(PCH)
	$(CC) -c $(CXXFLAGS) $< -o $@ -include $(PCH_SRC)

$(SDK_OUT_DIR)/%.o: $(BASE_DIR)/%.cpp $(PCH)
	$(CC) -c $(CXXFLAGS) $< -o $@ -include $(PCH_SRC)

$(SDK_OUT_DIR)/%.o: $(PROTOCOL_DIR)/%.cpp $(PCH)
	$(CC) -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@ -include $(PCH_SRC)

# 包含依赖文件（.d）
-include $(ALL_OBJ:.o=.d)

.PHONY: clean
clean:
	$(RM) $(SDK_OUT_DIR)/*.o $(SDK_OUT_DIR)/*.d $(PCH)

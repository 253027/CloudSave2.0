# 源文件目录
SDK_DIR := .

# 对象文件输出目录
OBJ_OUT_DIR := ../build
SDK_OUT_DIR := $(OBJ_OUT_DIR)/src

# 创建输出目录
$(shell mkdir -p $(OBJ_OUT_DIR))
$(shell mkdir -p $(SDK_OUT_DIR))

# 源文件
SDK_SRC := $(wildcard $(SDK_DIR)/*.cpp)

# 对象文件
SDK_OBJ := $(patsubst $(SDK_DIR)/%.cpp, $(SDK_OUT_DIR)/%.o, $(SDK_SRC))

# 预编译头文件
PCH := $(SDK_OUT_DIR)/pch.h.gch
PCH_SRC := $(SDK_DIR)/pch.h

# 编译器和编译选项
CC := g++
CXXFLAGS ?= -O2 -MMD -MP  # 添加生成依赖文件的选项

.PHONY: all
all: $(PCH) $(SDK_OBJ)

# 生成预编译头文件
$(PCH): $(PCH_SRC)
	$(CC) -c $(CXXFLAGS) $(LDFLAGS) $(PCH_SRC) -o $(PCH)

# 生成对象文件，包含预编译头文件
$(SDK_OUT_DIR)/%.o: $(SDK_DIR)/%.cpp $(PCH)
	$(CC) -c $(CXXFLAGS) $< -o $@ -include $(PCH_SRC)

# 包含依赖文件（.d）
-include $(SDK_OBJ:.o=.d)

.PHONY: clean
clean:
	$(RM) $(SDK_OUT_DIR)/*.o $(SDK_OUT_DIR)/*.d $(PCH)

# 根目录，项目中所有代码的根路径
ROOT_DIR := .
SDK_DIR := $(ROOT_DIR)/ServerSDK

# 设置对象文件的输出目录
OBJ_OUT_DIR := build
SDK_OUT_DIR := $(OBJ_OUT_DIR)/ServerSDK

# 创建输出目录
$(shell mkdir -p $(OBJ_OUT_DIR))
$(shell mkdir -p $(SDK_OUT_DIR))

# 搜索 ServerSDK 源文件并为每个文件构造完整的路径
SDK_SRC := $(wildcard $(SDK_DIR)/*.cpp)

# 构造对象文件路径（保留子目录结构） ./ServerSDK/*.cpp --> build/ServerSDK/*.o
SDK_OBJ := $(patsubst $(SDK_DIR)/%.cpp, $(SDK_OUT_DIR)/%.o, $(SDK_SRC))

# 预编译头文件路径
PCH := $(SDK_OUT_DIR)/pch.h.gch

# 预编译头文件的源文件路径
PCH_SRC := $(SDK_DIR)/pch.h

CC := g++

DEBUG ?= 1

ifeq ($(DEBUG), 1)
    CFLAGS := -g -D_DEBUG -lpthread -fsanitize=address
else
    CFLAGS := -O2 -lpthread
endif

.PHONY: all
all: $(PCH) $(SDK_OBJ) SessionServer

# 生成预编译头文件
$(PCH): $(PCH_SRC)
	$(CC) -c $(CFLAGS) $(PCH_SRC) -o $(PCH)

# 模式规则，从 SDK_DIR/*.cpp 生成 SDK_OUT_DIR/*.o，使用预编译头文件
$(SDK_OUT_DIR)/%.o: $(SDK_DIR)/%.cpp $(PCH)
	$(CC) -c $(CFLAGS) $< -o $@ -include $(PCH_SRC)

.PHONY: SessionServer
SessionServer: $(SDK_OBJ)
	$(MAKE) -C SessionServer

.PHONY: clean
clean:
	$(RM) -r $(OBJ_OUT_DIR)
	$(MAKE) -C SessionServer clean
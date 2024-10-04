# 根目录，项目中所有代码的根路径
ROOT_DIR := .

# 包含源文件的子目录列表
SUBDIRS := ServerSDK

# 构造源文件的目录路径
SRC_DIRS := $(addprefix $(ROOT_DIR)/, $(SUBDIRS))

# 搜索源文件并为每个文件构造完整的路径
SRC := ./SessionServer/server.cpp \
       $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.cpp))

# 设置对象文件的输出目录
OBJ_DIR := build
$(shell mkdir -p $(OBJ_DIR))

# 构造对象文件路径（保留子目录结构）
OBJ := $(patsubst $(ROOT_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))

# 预编译头文件路径 (在ServerSDK目录下)
PCH := $(OBJ_DIR)/pch.h.gch

# 预编译头文件的源文件路径 (ServerSDK/pch.h)
PCH_SRC := $(ROOT_DIR)/ServerSDK/pch.h

# 编译器和编译选项
CC := g++

# 是否启用调试模式 (默认为 1，即调试模式)
DEBUG ?= 1

# 根据 DEBUG 变量设置编译选项
ifeq ($(DEBUG), 1)
    CFLAGS := -g -D_DEBUG -lpthread -fsanitize=address
else
    CFLAGS := -O2 -lpthread
endif

# 链接选项
LDFLAGS := -L./lib
LDLIBS := -lcrypt -lmysqlclient -lspdlog

# 生成可执行文件
./SessionServer/server: $(OBJ)
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS) $(LDLIBS)

# 生成预编译头文件 (从ServerSDK/pch.h)
$(PCH): $(PCH_SRC)
	$(CC) -c $(CFLAGS) $(PCH_SRC) -o $(PCH)

# 模式规则，从%.cpp生成%.o，使用预编译头文件
$(OBJ_DIR)/%.o: $(ROOT_DIR)/%.cpp $(PCH)
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@ -include $(PCH_SRC)

# 清理规则，删除生成的二进制文件和对象文件
.PHONY: clean
clean:
	$(RM) -r $(OBJ_DIR) server
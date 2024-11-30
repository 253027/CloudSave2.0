# 根目录的路径
ROOT_DIR := ..

# 子目录的源文件
SRC := $(wildcard *.cpp)

# 设置对象文件的输出目录
OBJ_DIR := $(ROOT_DIR)/build/SessionServer
$(shell mkdir -p $(OBJ_DIR))

# 构造对象文件路径（保留子目录结构）
OBJ := $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRC))

# 使用 src 的对象文件
SDK_OBJ := $(wildcard $(ROOT_DIR)/build/src/*.o)

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
LDFLAGS := -L$(ROOT_DIR)/lib
LDLIBS := -lcrypt -lmysqlclient -lspdlog

# 生成可执行文件
server: $(OBJ) $(SDK_OBJ)
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS) $(LDLIBS)

# 模式规则，从%.cpp生成%.o
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

# 清理规则，删除生成的二进制文件和对象文件
.PHONY: clean
clean:
	$(RM) $(OBJ_DIR)/*.o server
